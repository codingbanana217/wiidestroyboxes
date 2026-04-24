#include <grrlib.h>
#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/system.h>
#include <stdlib.h>

#include "../box2d/box2d/box2d.h"
#include "../source/game.h"

// Font
#include "BMfont5_png.h"
GRRLIB_texImg *tex_BMfont5;

//Image
#include "box_png.h"
GRRLIB_texImg *tex_box;
#include "gold_box_png.h"
GRRLIB_texImg *tex_gold_box;
#include "tele_box_png.h"
GRRLIB_texImg *tex_tele_box;
#include "tnt_box_png.h"
GRRLIB_texImg *tex_tnt_box;
#include "thing_1_png.h"
GRRLIB_texImg *tex_thing_1;
#include "shot_png.h"
GRRLIB_texImg *tex_shot;
#include "white_png.h"
GRRLIB_texImg *tex_white;

// Colors
#define GRRLIB_BLACK  0x000000FF
#define GRRLIB_WHITE  0xFFFFFFFF
#define GRRLIB_GRAY   0x808080FF
#define GRRLIB_RED    0xFF0000FF
#define GRRLIB_YELLOW 0xFFFF00FF
#define GRRLIB_GREEN  0x408000FF
#define GRRLIB_BLUE   0x0000FFFF
#define GRRLIB_PURPLE 0x360F44FF

void setup_box2d(void);
void box2d_next_frame(void);
void respawn_box(int boxID_to_move);
void clean_up_box2d(void);
int clamp(int value, int min, int max);
int disToPoint(float pointX, float pointY, float boxX, float boxY);
void draw(float x, float y, GRRLIB_texImg *img, b2Rot rot, float size_x, float size_y, int color);
void draw_boxes();
void draw_wiimotes();

extern int frame;
extern int boxes;
extern int box_hp[MAX_BOXES];
extern int box_score[MAX_BOXES];
extern float box_size[MAX_BOXES];
extern BoxType_T box_img[MAX_BOXES];
extern int box_hiting[MAX_BOXES];
extern b2BodyId boxID[MAX_BOXES];
extern b2BodyId groundId;

float time_limit = 30.0;
float time_left;
int difficulty = 2;
int score = 0;
int ui_number = 0;
bool debug = false;
bool telemode = false;
bool tnt_boxes = true;


int main(int argc, char **argv) {
    GRRLIB_Init();
    
    // font
    tex_BMfont5 = GRRLIB_LoadTexture(BMfont5_png);
    GRRLIB_InitTileSet(tex_BMfont5, 8, 16, 0); 

    // image
    tex_box = GRRLIB_LoadTexture(box_png);
    GRRLIB_SetMidHandle(tex_box, true);

    tex_gold_box = GRRLIB_LoadTexture(gold_box_png);
    GRRLIB_SetMidHandle(tex_gold_box, true);
    
    tex_tele_box = GRRLIB_LoadTexture(tele_box_png);
    GRRLIB_SetMidHandle(tex_tele_box, true);
    
    tex_tnt_box = GRRLIB_LoadTexture(tnt_box_png);
    GRRLIB_SetMidHandle(tex_tnt_box, true);

    tex_thing_1 = GRRLIB_LoadTexture(thing_1_png);
    GRRLIB_SetMidHandle(tex_thing_1, true);

    tex_shot = GRRLIB_LoadTexture(shot_png);
    GRRLIB_SetMidHandle(tex_shot, true);

    tex_white = GRRLIB_LoadTexture(white_png);
    GRRLIB_SetMidHandle(tex_white, true);

    // wiimote
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	
	setup_box2d();

    while (true) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0) | WPAD_ButtonsDown(1) | WPAD_ButtonsDown(2) | WPAD_ButtonsDown(3);

        if (pressed & WPAD_BUTTON_DOWN) {
            ui_number++;
            if (ui_number > 4) {
                ui_number = 0;
            }
        }

        if (pressed & WPAD_BUTTON_UP) {
            ui_number--;
            if (ui_number < 0) {
                ui_number = 4;
            }
        }

        if (pressed & (WPAD_BUTTON_RIGHT | WPAD_BUTTON_A)) {
            if (ui_number == 0) {
                break;
            }
            else if (ui_number == 1) {
                difficulty++;
                if (difficulty > 3 && !debug) {
                    difficulty = 1;
                }
            }
            else if (ui_number == 2) {
                telemode = !telemode;
            }
            else if (ui_number == 3) {
                tnt_boxes = !tnt_boxes;
            }
            else if (ui_number == 4) {
                debug = !debug;
            }
        }

        if (pressed & (WPAD_BUTTON_LEFT | WPAD_BUTTON_B)) {
            if (ui_number == 0) {
                break;
            }
            else if (ui_number == 1) {
                difficulty--;
                if (difficulty < 1 && !debug) {
                    difficulty = 3;
                }
            }
            else if (ui_number == 2) {
                telemode = !telemode;
            }
            else if (ui_number == 3) {
                tnt_boxes = !tnt_boxes;
            }
            else if (ui_number == 4) {
                debug = !debug;
            }
        }

        GRRLIB_FillScreen(GRRLIB_PURPLE);
        draw_wiimotes();
        
        
        if (ui_number == 0) {
            GRRLIB_Printf(250, 200, tex_BMfont5, GRRLIB_WHITE, 1, "> Start");
        }
        else {
            GRRLIB_Printf(250, 200, tex_BMfont5, GRRLIB_WHITE, 1, "Start");
        }

        if (ui_number == 1) {
            GRRLIB_Printf(250, 225, tex_BMfont5, GRRLIB_WHITE, 1, "> Difficulty %d", difficulty);
        }
        else {
            GRRLIB_Printf(250, 225, tex_BMfont5, GRRLIB_WHITE, 1, "Difficulty %d", difficulty);
        }

        if (ui_number == 2) {
            GRRLIB_Printf(250, 250, tex_BMfont5, GRRLIB_WHITE, 1, "> Teleport mode %d", telemode);
        }
        else {
            GRRLIB_Printf(250, 250, tex_BMfont5, GRRLIB_WHITE, 1, "Teleport mode %d", telemode);
        }

        if (ui_number == 3) {
            GRRLIB_Printf(250, 275, tex_BMfont5, GRRLIB_WHITE, 1, "> Tnt boxes %d", tnt_boxes);
        }
        else {
            GRRLIB_Printf(250, 275, tex_BMfont5, GRRLIB_WHITE, 1, "Tnt boxes %d", tnt_boxes);
        }

        if (ui_number == 4 && debug) {
            GRRLIB_Printf(250, 300, tex_BMfont5, GRRLIB_WHITE, 1, "> Debug %d", debug);
        }

        GRRLIB_Render();
    }

	while(true) {
        box2d_next_frame();
        time_left = time_limit - (frame / 60.0);
        
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0) | WPAD_ButtonsDown(1) | WPAD_ButtonsDown(2) | WPAD_ButtonsDown(3);

        if ((time_left <= 0.0) || (difficulty > 2 && score < 0) || (pressed & WPAD_BUTTON_HOME)) {
            break;
        }

        for (int wiimote = 0; wiimote <= 3; wiimote++) {
            WPADData* data = WPAD_Data(wiimote);

            if(!data->data_present) {
                continue;
            }

            if ((pressed && WPAD_BUTTON_A) || (pressed && WPAD_BUTTON_B)) {
                for (int i=0; i<boxes; i++) {
                    if (disToPoint(data->ir.x, data->ir.y, b2Body_GetPosition(boxID[i]).x, b2Body_GetPosition(boxID[i]).y) < box_size[i] * 25) {
                        box_hp[i] -= 1;
                        score += box_score[i];
                        box_hiting[i] = 6;

                        if (box_img[i] == TELE_BOX || telemode) {
                            respawn_box(i);
                        }

                        if (box_hp[i] <= 0) {
                            b2Body_SetTransform(boxID[i], (b2Vec2){-1000, -1000}, b2MakeRot(0));
                            b2Body_Disable(boxID[i]);

                            if (box_img[i] != TNT_BOX) {
                                score += box_score[i] * 2;
                                time_limit += 0.1 / difficulty;
                            }
                        }
                    }
                }
            }
        }
        // draw
        GRRLIB_FillScreen(GRRLIB_PURPLE);

        b2Vec2 pos = b2Body_GetPosition(groundId);
        b2Rot rot = b2Body_GetRotation(groundId);
        draw(pos.x, pos.y, tex_thing_1, rot, 10, 10, 0xffffffff);

        draw_boxes();
        draw_wiimotes();

        GRRLIB_Printf(5, 5, tex_BMfont5, GRRLIB_WHITE, 1, "Time remaining %0.1f", time_left);
        GRRLIB_Printf(5, 20, tex_BMfont5, GRRLIB_WHITE, 1, "Score %d", score);
        GRRLIB_Printf(5, 35, tex_BMfont5, GRRLIB_WHITE, 1, "Difficulty %d", difficulty);
        GRRLIB_Render();
    }
	// clean up box2d and GRRLIB
	clean_up_box2d();
    GRRLIB_FreeTexture(tex_box);
    GRRLIB_FreeTexture(tex_gold_box);
    GRRLIB_FreeTexture(tex_tele_box);
    GRRLIB_FreeTexture(tex_thing_1);
    GRRLIB_FreeTexture(tex_shot);
    GRRLIB_FreeTexture(tex_BMfont5);
    GRRLIB_FreeTexture(tex_white);
    GRRLIB_Exit();
    return 0;
}


int clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


int disToPoint(float pointX, float pointY, float boxX, float boxY) {
    float offsetX = abs(boxX * 25 + 320 - pointX);
    float offsetY = abs(boxY * - 25 + 264  - pointY);
    return (offsetX + offsetY) / 2;
}


void draw(float x, float y, GRRLIB_texImg *img, b2Rot rot, float size_x, float size_y, int color) {
    GRRLIB_DrawImg((x * 25) + 320, (y * - 25) + 264, img, b2Rot_GetAngle(rot) / B2_PI * - 180, size_x * 0.25, size_y * 0.25, color);
}


void draw_boxes() {
    WPAD_ScanPads();
    for (int wiimote = 0; wiimote <= 3; wiimote++) {
        WPADData* data = WPAD_Data(wiimote);
        
        if(!data->data_present) {
            continue;
        }

        for (int i=0; i<boxes; i++) {
            b2Vec2 pos = b2Body_GetPosition(boxID[i]);
            b2Rot rot = b2Body_GetRotation(boxID[i]);
            // 
            int light_effect = clamp(disToPoint(data->ir.x, data->ir.y, pos.x, pos.y) * time_limit / time_left, 0, 255);
            
            if (box_hiting[i] > 0) {
                draw(pos.x, pos.y, tex_white, rot, box_size[i], box_size[i], 0xFFFFFFFF - light_effect);
            }
            else if (box_img[i] == BOX) {
                draw(pos.x, pos.y, tex_box, rot, box_size[i], box_size[i], 0xFFFFFFFF - light_effect);
            }
            else if (box_img[i] == GOLD_BOX) {
                draw(pos.x, pos.y, tex_gold_box, rot, box_size[i], box_size[i], 0xFFFFFFFF - light_effect);
            }
            else if (box_img[i] == TELE_BOX) {
                draw(pos.x, pos.y, tex_tele_box, rot, box_size[i], box_size[i], 0xFFFFFFFF - light_effect);
            }
            else if (box_img[i] == TNT_BOX) {
                draw(pos.x, pos.y, tex_tnt_box, rot, box_size[i], box_size[i], 0xFFFFFFFF - light_effect);
            }

            if (debug) {
                GRRLIB_Printf((data->ir.x * 25) + 320, (data->ir.y * - 25) + 264, tex_BMfont5, GRRLIB_WHITE, 1, "%d", light_effect);
            }
        }
    }
}


void draw_wiimotes() {
    WPAD_ScanPads();

    for (int wiimote = 0; wiimote <= 3; wiimote++) {
        WPADData* data = WPAD_Data(wiimote);

        if(!data->data_present) {
            continue;
        }

        if (wiimote == 0) {
            GRRLIB_DrawImg(data->ir.x, data->ir.y, tex_shot, 1, 0.15, 0.15, GRRLIB_BLUE);
        }
        else if (wiimote == 1) {
            GRRLIB_DrawImg(data->ir.x, data->ir.y, tex_shot, 1, 0.15, 0.15, GRRLIB_RED);
        }
        else if (wiimote == 2) {
            GRRLIB_DrawImg(data->ir.x, data->ir.y, tex_shot, 1, 0.15, 0.15, GRRLIB_GREEN);
        }
        else {
            GRRLIB_DrawImg(data->ir.x, data->ir.y, tex_shot, 1, 0.15, 0.15, GRRLIB_YELLOW);
        }
    }
}