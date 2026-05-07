#include <stdio.h>
#include <stdlib.h>

#include "../box2d/box2d/box2d.h"
#include "game.h"

int boxes = 0;
int box_hp[MAX_BOXES];
int box_score[MAX_BOXES];
float box_size[MAX_BOXES];
int box_gravity[MAX_BOXES];
float box_density[MAX_BOXES];
BoxType_T box_img[MAX_BOXES];
int box_hiting[MAX_BOXES];

b2WorldDef worldDef;
b2WorldId worldId;
b2BodyDef groundBodyDef;
b2BodyId groundId;
b2Polygon groundBox;
b2ShapeDef groundShapeDef;
b2BodyDef boxDef;
b2BodyId boxID[MAX_BOXES];
b2Polygon box;
b2ShapeDef shapeDef;

// time setup
const float timeStep = 1.0 / 60.0;
const int subStep = 4;
int frame = 0;

// from main.c
extern int difficulty;
extern int score;
extern bool tnt_boxes;
extern bool tele_boxes;


void setup_box2d(void) {
    // make the world
    worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0, 0.0};
    worldId = b2CreateWorld(&worldDef);

    // make the ground
    groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.gravityScale = 0;
    groundId = b2CreateBody(worldId, &groundBodyDef);
    groundBox = b2MakeBox(10.0, 10.0);
    groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // setup boxes
    boxDef = b2DefaultBodyDef();
    box = b2MakeBox(0.5, 0.5);
    shapeDef = b2DefaultShapeDef();
    boxDef.type = b2_dynamicBody;
}


static void make_box(float friction, float density, float gravity, float size, BoxType_T img, int hp, int score) {
    box_hp[boxes] = hp;
    box_score[boxes] = score;

    shapeDef.material.friction = friction;
    box_gravity[boxes] = gravity;

    box_size[boxes] = size * 0.4;
    box = b2MakeBox(size * 0.4, size * 0.4);

    shapeDef.density = density;
    box_density[boxes] = density * size * 0.4;

    box_img[boxes] = img;
    box_hiting[boxes] = 0;

    boxID[boxes] = b2CreateBody(worldId, &boxDef);
    b2CreatePolygonShape(boxID[boxes], &shapeDef, &box);
    boxes++;
}


void box2d_next_frame(void) {
    b2World_Step(worldId, timeStep, subStep);

    if (frame % (60 / difficulty) == 0 && frame >= 300 && difficulty >= 3) {
        score -= 10;
    }
    
    // make a box
    if (frame % 5 == 0) {
        boxDef.position = (b2Vec2){rand() % 10 - 5, 12};
        int rang_box_size = rand() % 3 + 1;

        // the boss boxes
        if (frame % 1200 == 0 && frame >= 1200) {
            boxDef.position = (b2Vec2){0, 12};
            if (difficulty <= 1) {
                make_box(1.0, 4.0, -10.0, 7.5, BOX, 50, 100);
            }
            else if (difficulty == 2) {
                make_box(1.0, 8.0, -10.0, 5.0, GOLD_BOX, 80, 50);
            }
            else if (difficulty == 3) {
                make_box(1.0, 6.0, -5.0, 5.0, TELE_BOX, 30, 150);
            }
            else if (difficulty >= 4) {
                make_box(1.0, 6.0, -10.0, 7.5, TNT_BOX, 1, -5000);
            }
        }
        // the normal boxes
        else if (rand() % 10 == 0 && tnt_boxes) {
            make_box(1.0, pow(1.0 * rang_box_size, 2), -80.0, 1.0 * rang_box_size, TNT_BOX, 1, -100 * difficulty);
        }
        else if (rand() % 14 == 0 && tele_boxes) {
            make_box(1.0, pow(0.75 * rang_box_size, 2), -5.0, 1.0 * rang_box_size, TELE_BOX, 1 * rang_box_size, 50);
        }
        else if (rand() % 12 == 0) {
            make_box(1.0, pow(1.5 * rang_box_size, 2), -80.0, 1.0 * rang_box_size, GOLD_BOX, 2 * rang_box_size, 30);
        }
        else {
            make_box(1.0, pow(1.0 * rang_box_size, 2), -80, 1.0 * rang_box_size, BOX, 1.5 * rang_box_size, 10);
        }
    }

    // gravity and hiting effect for the boxes
    for (int i=0; i<boxes; i++) {
        b2Vec2 apply_force = {0, box_density[i] * box_gravity[i]};
        b2Body_ApplyForceToCenter(boxID[i], apply_force, true);
        box_hiting[i] -= 1;
    }
    
    // move the ground
    b2Body_SetTransform(groundId, (b2Vec2){0.0, - 16.0}, b2MakeRot(sin(frame / 90.0 * difficulty) * 0.35));
    frame++;
}


void respawn_box(int boxID_to_move) {
    b2Body_SetTransform(boxID[boxID_to_move], (b2Vec2){rand() % 10 - 5, 8}, b2MakeRot(0));
}


void clean_up_box2d(void) {
    b2DestroyWorld(worldId);
}