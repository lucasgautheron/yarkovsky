#include "sdldglutils/sdlglutils.h"
//#include "fonts.h"

struct color
{
    short unsigned int r, g, b, a;
};

#define face_to_color(f, r, g, b) \
    r = (f+1)&0x0000FF; \
    g = ((f+1)&0x00FF00)>>8; \
    b = ((f+1)&0xFF0000)>>16;

#define color_to_face(r,g,b) ( (r | (g<<8)) | (b<<16) ) - 1 )

extern SDL_Surface *screen;
extern vec angle;
extern double zoom;
extern int w, h;

void init_sdl();
void render();

// 3D
void renderframe();
void rendermodel(Model &mdl);
void rendermodelfaces(Model &mdl);
