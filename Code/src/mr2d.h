#ifndef _MR2D_H_
#define _MR2D_H_

#include <signal.h>

#include <utility>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <X11/Xlib.h>


namespace mr2d
{
	
	#include "geometry.h"
	#include "structures.h"
	#include "data.h"
	#include "mesh.h"
	#include "display.h"

}


#endif
