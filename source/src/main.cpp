// main.cpp
// program start / quit
#include "project.h"

void init();
void close();
void callback_quit();

bool keyreleased = true;
bool mousedown = false;

int mouse_x = 0, mouse_y = 0;

int lastmillis = 0, curmillis = 0, diffmillis;

bool paused = false;
int nloops = 0;

int main(int argc, char *argv[])
{
    init();
    if(argc <= 1)
    {
        printf("modele non specifie\n");
        callback_quit();
        return EXIT_FAILURE;
    }

    if (argc > 2)
    {
        nloops = atoi(argv[2]);
        printf("Set to perform %d loops before stopping.\n", nloops);
    }

    

    asteroid1.loadmdl(argv[1], argc > 3 ? argv[3] : "default.csv");
    asteroid1.time = 0.0;
    asteroid1.angle = vec(0, 0, 0);

    double minstep = (asteroid1.mdl->faces[0]->depth * asteroid1.mdl->faces[0]->depth) / (TEMPDIV * TEMPDIV * asteroid1.diffusivity);
    if(asteroid1.timestep > minstep)
    {
        printf("error: timestep is too high for the simulation to properly converge (max dt: %e s)\n", minstep);
        return 1;
    }

    printf("%e %e\n", asteroid1.mdl->faces[0]->depth / TEMPDIV, sqrt(asteroid1.diffusivity * asteroid1.period / 3.14) / 50);
 
    if( asteroid1.mdl->faces[0]->depth / TEMPDIV >= sqrt(asteroid1.diffusivity * asteroid1.period / 3.14) / 50) 
    {
        printf("error: spatial step is too high (max dz: %e m)\n", sqrt(asteroid1.diffusivity * asteroid1.period / 3.14) / 50);
        return 1;
    }

    // main loop
#ifdef GUI
    SDL_Event event;
    for(;;)
    {
        lastmillis = curmillis; 
        curmillis = SDL_GetTicks();
        diffmillis = curmillis - lastmillis;

        SDL_PollEvent(&event);

        bool pressed = false;
        switch(event.type)
        {
            case SDL_KEYDOWN:
            {
                pressed = true; // a key has been pressed
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE: callback_quit(); break;
                    case SDLK_p: if(keyreleased) paused = !paused; break;
                    case SDLK_RIGHT:
                    {
                        angle.y -= 20.0*double(diffmillis)/1000.0;
                        angle.y = angle.y > 360 ? int(diffmillis) % 360 : angle.y;
                    }
                    break;
                    case SDLK_LEFT:
                    {
                        angle.y += 20.0*double(diffmillis)/1000.0;
                        angle.y = angle.y > 360 ? int(angle.y) % 360 : angle.y;
                    }
                    case SDLK_UP:
                    {
                        angle.z += 20.0*double(diffmillis)/1000.0;
                        angle.z = angle.z > 360 ? int(angle.z) % 360 : angle.z;
                    }
                    break;
                    case SDLK_DOWN:
                    {
                        angle.z -= 20.0*double(diffmillis)/1000.0;
                        angle.z = angle.z > 360 ? int(angle.z) % 360 : angle.z;
                    }
                    break;
                    case SDLK_KP_PLUS:
                    {
                        zoom *= (1+double(diffmillis)/1000.0);
                    }
                    break;
                    case SDLK_KP_MINUS:
                    {
                        zoom *= (1-double(diffmillis)/1000.0);
                    }
                    break;
                    default:
                    {
                        
                    }
                }
            }
            break;
            case SDL_QUIT: callback_quit(); break;
            case SDL_VIDEORESIZE:
            {
                w = event.resize.w;
                h = event.resize.h;
                glViewport(0, 0, w, h);

            }
        }
        
        if(pressed && keyreleased) keyreleased = false;
        else if(!pressed && !keyreleased) keyreleased = true;
        
        render();
        if(!paused) asteroid1.calcloop();

        if(diffmillis < 5) SDL_Delay(5-diffmillis);
    }
#else
    for(int i = 1; i != nloops; ++i)
    {
        asteroid1.calcloop();
        if (i % 20 == 0) printf("20 loops executed");
    }
#endif
    callback_quit();
    return EXIT_SUCCESS;
}

void init()
{
    srand ( (unsigned int)time(NULL) );
    dbgoutf("Initialisation en cours...");
#ifdef GUI
    init_sdl();
#endif
}

void callback_quit()
{
#ifdef GUI
    //TTF_Quit();
    SDL_Quit();
#endif
    exit(EXIT_SUCCESS);
}
