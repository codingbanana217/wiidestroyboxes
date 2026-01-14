#include <X11/Xlib.h>

void draw_rotated_rect(Display *d, Window w, GC gc,
                       int x, int y, int width, int height,
                       float angle_deg);


