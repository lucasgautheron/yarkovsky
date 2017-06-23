#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <vector>
#include <sstream>
#include <stdarg.h>
#include <math.h>
#include <omp.h>
#include <ctime>

#if defined(WIN32)
    #include <ppl.h>
    #include <windows.h>
    #include <Mmsystem.h>
    #include <time.h>
#else
    #include <sys/time.h>
#endif

#ifdef GUI
#if defined(WIN32)
  #include <gl/gl.h>
  #include <gl/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

#include <SDL/SDL.h>
#include "SDL/SDL_image.h"
#include "SDL/SDL_thread.h"
#endif

#if defined(WIN32)
  #include "zlib/zlib.h"
#else
  #include "zlib.h"
#endif

using namespace std;

#include "tools.h"
#include "stream.h"
#include "maths.h"
#include "model.h"
#include "physics.h"
#ifdef GUI
#include "render.h"
#endif
