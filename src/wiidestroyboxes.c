#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "draw.h"
#include "box2d/box2d.h"

Display *display;
Window window;
int screen;
GC gc;


int scale_x(float in) {
    return (in * 60) + 300;
}


int scale_y(float in) {
    return (in * -40) + 200;
}


int main(void) {
    printf("runuing\n");

    // x11
    display = XOpenDisplay(NULL);
    screen = DefaultScreen(display);
    window = XCreateSimpleWindow( display,RootWindow(display, screen),0, 0,600, 400,1,BlackPixel(display, screen),WhitePixel(display, screen));
    XMapWindow(display, window);
    gc = XCreateGC(display, window, 0, NULL);

    // box2d make world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0, -10.0};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // make ground
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2){0.0, -10.0};
    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);
    b2Polygon groundBox = b2MakeBox(50.0, 10.0);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // make boxes
    b2BodyId bodyId [5];
    b2BodyDef bodyDef = b2DefaultBodyDef();
    b2Polygon box = b2MakeBox(1.0, 1.0);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    bodyDef.type = b2_dynamicBody;
    
    for (int i=0;i<5;i++) {
        bodyDef.position = (b2Vec2){0.0, 4.0+i};
        bodyId[i] = b2CreateBody(worldId, &bodyDef);
        shapeDef.density = 1.0;
        shapeDef.material.friction = 0.3;
        b2CreatePolygonShape(bodyId[i], &shapeDef, &box);
        b2Body_SetAngularVelocity(bodyId[i], 10.0);
    }

    // set sim 
    const float timeStep = 1.0 / 60.0;
    const int subStep = 4;

    for (int i=0;i<500;i++) {
        b2World_Step(worldId, timeStep, subStep);
        XClearWindow(display, window);

        for (int i=0;i<5;i++) {
            b2Vec2 pos = b2Body_GetPosition(bodyId[i]);
            b2Rot rot = b2Body_GetRotation(bodyId[i]);
            draw_rotated_rect(display, window, gc, scale_x(pos.x), scale_y(pos.y), 80, 80, b2Rot_GetAngle(rot));
        }

        XFlush(display);
        usleep((1/60.0)/ 0.000001);
    }

    // clean up
    b2DestroyWorld(worldId);
    XCloseDisplay(display);
}
