#include <X11/Xlib.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    float x, y;
} Point;

void rotate_point(Point *p, float cx, float cy, float angle) {
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin
    p->x -= cx;
    p->y -= cy;

    // rotate
    float xnew = p->x * c - p->y * s;
    float ynew = p->x * s + p->y * c;

    // translate back
    p->x = xnew + cx;
    p->y = ynew + cy;
}

void draw_rotated_rect(Display *d, Window w, GC gc,
                       int x, int y, int width, int height,

                       float angle)
{
    XDrawArc(d, w, gc, x-2, y-2, 4, 4, 0, 360 * 64);

    Point pts[4] = {
        {x - width/2, y - height/2},
        {x + width/2, y - height/2},
        {x + width/2, y + height/2},
        {x - width/2, y + height/2}
    };

    for (int i = 0; i < 4; i++) {
        rotate_point(&pts[i], x, y, angle);
    }

    XPoint xpts[5];
    for (int i = 0; i < 4; i++) {
        xpts[i].x = (short)pts[i].x;
        xpts[i].y = (short)pts[i].y;
    }
    xpts[4] = xpts[0]; // close polygon

    XDrawLines(d, w, gc, xpts, 5, CoordModeOrigin);

    XDrawLine(d, w, gc, (int)pts[0].x, (int)pts[0].y, x, y);
    // Draw angle text inside the rectangle (not rotated) 
    if(0){
        char buf[64]; 
        snprintf(buf, sizeof(buf), "%3.1f", angle* (180.0 / M_PI) ); 
        // Draw text near the center 
        XDrawString(d, w, gc, x - 30, y, buf, strlen(buf));        
    }
}
