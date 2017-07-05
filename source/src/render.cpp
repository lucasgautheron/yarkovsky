#include "project.h"

#ifdef GUI

SDL_Surface *screen = NULL;
//std::vector<GPFont *> fonts;

vec angle(0,0,0);
double zoom = 0.01;
int w = 1920, h = 1080;

void init_sdl()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        fprintf(stderr, "Erreur lors de l'initialisation de la SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    screen = SDL_SetVideoMode(w, h, 32, SDL_OPENGL | SDL_SWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);

    SDL_WM_SetCaption("Asteroid", NULL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0);				// fond noir
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);

}

void load_fonts()
{
    //fonts.push_back(new GPFont("consola.ttf", 12));
    //fonts.push_back(new GPFont("arial.ttf", 12));
    //fonts.push_back(new GPFont("consola.ttf", 12));
}

void auto_ortho(double scale, double z_scale)
{
    double x_scale = scale, y_scale = scale;
    if(w>h) y_scale *= double(h)/double(w);
    else x_scale *= double(h)/double(w);
    glViewport (0, 0, w, h) ;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-x_scale, x_scale, -y_scale, y_scale, -z_scale, z_scale);
    
    //glOrtho(asteroid1.mdl->bbmin.x, asteroid1.mdl->bbmax.x, asteroid1.mdl->bbmin.y, asteroid1.mdl->bbmax.y, asteroid1.mdl->bbmin.z, asteroid1.mdl->bbmax.z);
    glOrtho(asteroid1.mdl->o.x-asteroid1.mdl->size, asteroid1.mdl->o.x+asteroid1.mdl->size, asteroid1.mdl->o.y-asteroid1.mdl->size, asteroid1.mdl->o.y+asteroid1.mdl->size, asteroid1.mdl->o.z-asteroid1.mdl->size, asteroid1.mdl->o.z+asteroid1.mdl->size);

    glMatrixMode(GL_MODELVIEW);

    matrix m(4, 4);
    for(int i = 0; i < 4; ++i) for(int j = 0; j < 4; ++j)
    {
        if(i < 3 && j < 3) m[i][j] = asteroid1.rotmatrix[i][j];
        else if (i != j) m[i][j] = 0;
        else m[i][j] = 1;
    }
    double *arr = m.array();
    glLoadMatrixd(arr);
    glTranslated(-asteroid1.centerofmass.x, -asteroid1.centerofmass.y, -asteroid1.centerofmass.z);
    delete[] arr;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    auto_ortho(1, 1);
    //renderframe();
    rendermodelfaces(*asteroid1.mdl);
    SDL_GL_SwapBuffers();
}

void draw_text(const char *text, double x, double y, double z, int r, int g, int b, int font)
{
    /*glPushMatrix();

    glTranslated(x, y, z);
    glColor3ub(r, g, b);

    GPFont *ttf_font = (font >= 0 && font < fonts.size()) ? fonts[font] : fonts.back();
    if(!ttf_font) return;
    ttf_font->Print2D(text);
    glPopMatrix();*/
}

void draw_label(string text, int x, int y)
{
    int width = text.length() * 9, height = 25;

    draw_text(text.c_str(), (w-width)/2, 10, 0, 255, 255, 255);
}

void renderframe()
{
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(1.5*asteroid1.size, 0.0, 0.0);

    glColor3d(0.0, 1.0, 0.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 1.5*asteroid1.size, 0.0);

    glColor3d(0.0, 0.0, 1.0);
    glVertex3d(0.0, 0.0, 0.0);
    glVertex3d(0.0, 0.0, 1.5*asteroid1.size);

    glColor3d(1.0, 0.0, 1.0);
    glVertex3d(0.0, 0.0, 0.0);
    vec rv = asteroid1.rotvel;
    rv.normalize(1.5*asteroid1.size);
    glVertex3d(rv.x, rv.y, rv.z);

    glEnd();
    glEnable(GL_LIGHTING);
}

void rendermodel(Model &mdl)
{
    static double min_temp = 300, max_temp = 0;
    //glColor3d(1, 1, 1);
    glDisable(GL_LIGHTING);

    glBegin(GL_TRIANGLES);
    for(int i = 0; i < mdl.faces.size(); ++i)
    {
        //double val = 200+10*mdl.faces[i]->viewfactor;
        double val = mdl.faces[i]->temp[0];
        if(val > max_temp) max_temp = val;
        if(val < min_temp) min_temp = val;
    }

    for(int i = 0; i < mdl.faces.size(); ++i)
    {
        double val = mdl.faces[i]->temp[0];
        //double val = 200+10*mdl.faces[i]->viewfactor;
        glColor3d((val-min_temp)/(max_temp-min_temp), 1-fabs(val-((min_temp+max_temp)/2.0))/(max_temp-min_temp), 1-(val-min_temp)/(max_temp-min_temp));
        glNormal3dv(mdl.faces[i]->n.v);
        glVertex3dv(mdl.faces[i]->vertices[0]->v);
        glVertex3dv(mdl.faces[i]->vertices[1]->v);
        glVertex3dv(mdl.faces[i]->vertices[2]->v);
    }
    glEnd();

    glColor3d(0, 0, 0);
    for(int i = 0; i < mdl.faces.size(); ++i)
    {
        glBegin(GL_LINE_STRIP);
        glVertex3dv(mdl.faces[i]->vertices[0]->v);
        glVertex3dv(mdl.faces[i]->vertices[1]->v);
        glVertex3dv(mdl.faces[i]->vertices[2]->v);
        glVertex3dv(mdl.faces[i]->vertices[0]->v);
        glEnd();
    }
    
    return;

    glColor3d(1.0, 0.0, 0.0);
    vec delta = (mdl.bbmax-mdl.bbmin);
    vec delta_x = vec(delta.x, 0, 0);
    vec delta_y = vec(0, delta.y, 0);
    vec delta_z = vec(0, 0, delta.z);

    glColor3d(1.0, 0.0, 0.0);
    //glDisable(GL_LIGHTING);
    glLineWidth(1.0);
    
    glBegin(GL_LINES);
		glVertex3dv((mdl.bbmin).v); glVertex3dv((mdl.bbmin+delta_x).v);
		glVertex3dv((mdl.bbmin+delta_x).v); glVertex3dv((mdl.bbmin+delta_x+delta_y).v);
		glVertex3dv((mdl.bbmin+delta_x+delta_y).v); glVertex3dv((mdl.bbmin+delta_y).v);
		glVertex3dv((mdl.bbmin+delta_y).v); glVertex3dv((mdl.bbmin).v);
        
		glVertex3dv((mdl.bbmin+delta_z).v); glVertex3dv((mdl.bbmin+delta_x+delta_z).v);
		glVertex3dv((mdl.bbmin+delta_x+delta_z).v); glVertex3dv((mdl.bbmin+delta_x+delta_y+delta_z).v);
		glVertex3dv((mdl.bbmin+delta_x+delta_y+delta_z).v); glVertex3dv((mdl.bbmin+delta_y+delta_z).v);
		glVertex3dv((mdl.bbmin+delta_y+delta_z).v); glVertex3dv((mdl.bbmin+delta_z).v);

		glVertex3dv((mdl.bbmin).v); glVertex3dv((mdl.bbmin+delta_z).v);
		glVertex3dv((mdl.bbmin+delta_x).v); glVertex3dv((mdl.bbmin+delta_z+delta_x).v);
		glVertex3dv((mdl.bbmin+delta_y).v); glVertex3dv((mdl.bbmin+delta_z+delta_y).v);
		glVertex3dv((mdl.bbmin+delta_x+delta_y).v); glVertex3dv((mdl.bbmin+delta_z+delta_x+delta_y).v);
    glEnd();
    glEnable(GL_LIGHTING);
}

void rendermodelfaces(Model &mdl)
{
    glDisable(GL_LIGHTING);
    glBegin(GL_TRIANGLES);
    for(int i = 0; i < mdl.faces.size(); ++i)
    {
        int r, g, b;
        face_to_color(i, r, g, b);
        glColor3ub(r, g, b);
        glNormal3dv(mdl.faces[i]->n.v);
        glVertex3dv(mdl.faces[i]->vertices[0]->v);
        glVertex3dv(mdl.faces[i]->vertices[1]->v);
        glVertex3dv(mdl.faces[i]->vertices[2]->v);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}
#endif
