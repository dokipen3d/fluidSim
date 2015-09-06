

#include "GridOpsCollection.h"


#include "GL/glew.h"
//#include <OpenGL/gl.h>
#include "/usr/local/include/SDL2/SDL.h"
#include "/usr/local/include/SDL2/SDL_opengl.h"

#include <memory>
#include <time.h>
#include <omp.h>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "Renderer.h"
#include "RenderableObject.h"
#include "glm/vec3.hpp"

using namespace std;

const char *openGLErrorString(GLenum _errorCode) {
  // Only 3.2+ Core and ES 2.0+ errors, no deprecated strings like stack
  // underflow etc.
  if (_errorCode == GL_INVALID_ENUM) {
    return "GL_INVALID_ENUM";
  } else if (_errorCode == GL_INVALID_VALUE) {
    return "GL_INVALID_VALUE";
  } else if (_errorCode == GL_INVALID_OPERATION) {
    return "GL_INVALID_OPERATION";
  } else if (_errorCode == GL_INVALID_FRAMEBUFFER_OPERATION) {
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  } else if (_errorCode == GL_OUT_OF_MEMORY) {
    return "GL_OUT_OF_MEMORY";
  } else if (_errorCode == GL_NO_ERROR) {
    return "GL_NO_ERROR";
  } else {
    return "unknown error";
  }
}

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

//----------------------------------------------
int main(int argc, char *argv[]) {

  cout << "number of args is" << argc << endl;
  cout << *argv[argc - 1] << endl;
bool operate = false;

 //omp_set_num_threads(4);
  int input, input2;
  SDL_Event keyevent;
  bool eventLoop = true;
  double elapsedTime = 0.0;
  double averageTime = 0.0;
  int framesElapsed = 1;

  //for only rendering frames every once in a while.
  int frameCount = 0;

  auto grid_obj = make_unique<GridObject>();

  auto gridEmit = make_unique<GridEmitter>(grid_obj.get());
  gridEmit->setNodeName(std::string("emitter"));

  auto gridDiss = make_unique<GridDissipator>(grid_obj.get());
  gridDiss->setNodeName(std::string("diss"));

  auto gridPad = make_unique<GridPadCull>(grid_obj.get());
  gridPad->setNodeName(std::string("padcull"));

  auto vecEmit = make_unique<GridVectorEmitter>(grid_obj.get());
  vecEmit->setNodeName(std::string("vectorEmit"));
  vecEmit->SetChannelName(std::string("velocity"));

  //auto basicAdvect = make_unique<GridMacCormackAdvect>(grid_obj.get());

  auto basicAdvect = make_unique<GridRK2Advect>(grid_obj.get());
  //auto basicAdvect = make_unique<GridBasicAdvect>(grid_obj.get());

  basicAdvect->setNodeName(std::string("basicAdvect"));

  //auto basicVelAdvect = make_unique<GridMacCormackVelAdvect>(grid_obj.get());

  auto basicVelAdvect = make_unique<GridRK2VelAdvect>(grid_obj.get());
  //auto basicVelAdvect = make_unique<GridBasicVelAdvect>(grid_obj.get());

  basicVelAdvect->setNodeName(std::string("basicVelAdvect"));

  auto bouyancy = make_unique<GridBouyancy>(grid_obj.get());
  bouyancy->setNodeName(std::string("bouyancy"));

  auto divergence = make_unique<GridDivergence>(grid_obj.get());
  divergence->setNodeName(std::string("divergence oper"));

  //auto pressure = make_unique<GridPressure>(grid_obj.get());

  auto pressure = make_unique<GridPressure>(grid_obj.get());
  //auto pressure = make_unique<GridPressureRBGS>(grid_obj.get());

  pressure->setNodeName(std::string("pressure oper"));

  auto pressuretiled = make_unique<GridTiledPressure>(grid_obj.get());
pressuretiled->setNodeName(std::string("tiledpress"));

  auto projection = make_unique<GridProjection>(grid_obj.get());
  projection->setNodeName(std::string("projection oper"));


  gridEmit->IterateGrid();

  SDL_Window *window = NULL; // The surface contained by the window
  SDL_GLContext maincontext; /* Our opengl context handle */

  /* Request opengl 3.2 context.
   * SDL doesn't have the ability to choose which profile at this time of
   * writing,
   * but it should default to the core profile */

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  }

  else { // Create window
    window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if (window == NULL) {
      printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }
  }

  /* Create our opengl context and attach it to our window */
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

  maincontext = SDL_GL_CreateContext(window);
  if (maincontext == NULL) {
    printf("context could not be created! SDL_Error: %s\n", SDL_GetError());
  }
  // SDL_GL_SetSwapInterval(1);
  glewExperimental = GL_TRUE;
  glewInit();
  GLenum error = glGetError();
  //    if( error != GL_NO_ERROR )
  //    {
  //        //printf( "Ignore glew error! %s\n", openGLErrorString( error ) );
  //        //printf( "Error binding shader! \n" );
  //    }

  auto renderer = std::make_unique<Renderer>();
  string newObject = "RenderObjectname";

  auto renderObj = make_shared<RenderableObject>(newObject, grid_obj.get());
  // renderObj->setTranslate(glm::vec3(0.0, 0.0, -2.0));
  renderer->addRenderObject(renderObj);
  // glEnable( GL_MULTISAMPLE );
  glClearColor(0.0, 0.0, 0.0, 1.0);

  glClear(GL_COLOR_BUFFER_BIT);

  while (eventLoop)

  {
    // glClearColor ( 0.18, 0.18, 0.18, 1.0 );
    double timeA = omp_get_wtime();
    while (
        //SDL_PollEvent(&keyevent) && (!operate) )  // Poll our SDL key event for any keystrokes.
        SDL_PollEvent(&keyevent) ) // Poll our SDL key event for any keystrokes.

    {
      switch (keyevent.type) {
      case SDL_KEYDOWN:
        switch (keyevent.key.keysym.sym) {
        case SDLK_g:
          operate = true;
          break;
        case SDLK_ESCAPE:
          eventLoop = false;
          break;
        }
      }
    }


    //if (operate){

    gridEmit->IterateGrid();
    gridPad->IterateGrid();

    //vecEmit->IterateGrid();

    //gridDiss->IterateGrid();
    bouyancy->IterateGrid();
    divergence->IterateGrid();
    pressuretiled->IterateGrid();
    //pressure->IterateGrid();

  projection->IterateGrid();
    basicAdvect->IterateGrid();
    basicVelAdvect->IterateGrid();






    //only render every x frames
    if (frameCount == 1){
        renderer->Render();
        frameCount = 0;
    }
    else{
        renderer->RenderSame();
    }
    frameCount++;
    grid_obj->incrementSimTime(0.041666666f);

    /* Swap our back buffer to the front */
    SDL_GL_SwapWindow(window);
    double timeB = omp_get_wtime();
    double frameTime = timeB - timeA;
    elapsedTime += frameTime;
    averageTime = elapsedTime/framesElapsed;
    framesElapsed++;

    cout << "total time was " << frameTime << " seconds." << "average time is " << averageTime << " seconds." << endl << endl;
    operate = false;
//    if (grid_obj->simTime > 0.05)
//        eventLoop = false;
    //break;
  }
  //}

  /* Clear our buffer with a red background */

  SDL_GL_DeleteContext(maincontext);

  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
