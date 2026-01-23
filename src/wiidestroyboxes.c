#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <stdlib.h>

#include "draw.h"
#include "box2d/box2d.h"

#define MAX_BOXES 135

Display *display;
Window window;
int screen;
GC gc;

b2BodyId groundId;
int boxes = 0;
float box_size[MAX_BOXES];
b2BodyId bodyId[MAX_BOXES];



// for x11
int scale_x(float in) {
    return (in * 50) +540;
}


// for x11
int scale_y(float in) {
    return (in * -50) +270;
}

void draw_x11(void) {
    XClearWindow(display, window);
    for (int i=0;i<boxes;i++) {
        b2Vec2 pos = b2Body_GetPosition(bodyId[i]);
        b2Rot rot = b2Body_GetRotation(bodyId[i]);
        draw_rotated_rect(display, window, gc, scale_x(pos.x), scale_y(pos.y), box_size[i] *100, box_size[i] *100, -b2Rot_GetAngle(rot));
    }
    b2Vec2 pos = b2Body_GetPosition(groundId);
    b2Rot rot = b2Body_GetRotation(groundId);
    draw_rotated_rect(display, window, gc, scale_x(pos.x), scale_y(pos.y), 1000, 100, -b2Rot_GetAngle(rot));
    XFlush(display);
}


int main(void) {
    // x11 setup
    display = XOpenDisplay(NULL);
    screen = DefaultScreen(display);
    window = XCreateSimpleWindow( display,RootWindow(display, screen),0, 0,1080, 540,1,BlackPixel(display, screen),WhitePixel(display, screen));
    XMapWindow(display, window);
    gc = XCreateGC(display, window, 0, NULL);

    // box2d make world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0, 0.0};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // make ground
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.gravityScale = 0;
    groundId = b2CreateBody(worldId, &groundBodyDef);
    b2Polygon groundBox = b2MakeBox(10.0, 1.0);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // setup boxes
    int box_gravity[MAX_BOXES];
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2Polygon box = b2MakeBox(0.5, 0.5);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    bodyDef.type = b2_dynamicBody;

    // start simulatin
    const float timeStep = 1.0 / 60.0;
    const int subStep = 4;
    int frame = 0;
    while (true) {
        b2World_Step(worldId, timeStep, subStep);

        // make box
        if (boxes <MAX_BOXES && frame %9 == 0) {
            bodyDef.position = (b2Vec2){rand()%16 -8, 16};
            if (rand() %9 == 0) {
                // gold box
                shapeDef.density = 4.0;
                shapeDef.material.friction = 0.35;
                shapeDef.material.restitution = 0.0;
                box_gravity[boxes] = -20.0;
                box_size[boxes] = 1.0;
            }            
            else if (rand() %9 == 0) {
                // teleporting box
                shapeDef.density = 1.0;
                shapeDef.material.friction = 0.35;
                shapeDef.material.restitution = 0.5;
                box_gravity[boxes] = -5.0;
                box_size[boxes] = 0.75;
            }
            else {
                // box
                shapeDef.density = 1.0;
                shapeDef.material.friction = 0.35;
                shapeDef.material.restitution = 0.0;
                box_gravity[boxes] = -20.0;
                box_size[boxes] = 0.5;
            }

            box = b2MakeBox(box_size[boxes], box_size[boxes]);
            bodyId[boxes] = b2CreateBody(worldId, &bodyDef);
            b2CreatePolygonShape(bodyId[boxes], &shapeDef, &box);
            boxes++;
        }
        for (int i=0;i<boxes;i++) {
            b2Vec2 use_the_force = {0,box_gravity[i]};
            b2Body_ApplyForceToCenter(bodyId[i], use_the_force, true);
        }
        b2Vec2 setpos = {0.0,-2.5};
        b2Body_SetTransform(groundId, setpos, b2MakeRot(sin(frame /60.0) /4));

        draw_x11();
        frame++;
        usleep((1/60.0)/ 0.000001);
    }
    // clean up x11 and box2d
    b2DestroyWorld(worldId);
    XCloseDisplay(display);
}