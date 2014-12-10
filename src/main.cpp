#define GLM_COMPILER 0x40000060


#include "GridObject.h"
#include "GridEmitter.h"
#include "GridDissipator.h"
#include "GridPadCull.h"
#include "GridVectorEmitter.h"
#include "GridBasicAdvect.h"
#include "GridBouyancy.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>
//#include <OpenGL/gl.h>

#include <SDL2/SDL_opengl.h>

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

const char* openGLErrorString( GLenum _errorCode )
{
    // Only 3.2+ Core and ES 2.0+ errors, no deprecated strings like stack underflow etc.
    if      (_errorCode == GL_INVALID_ENUM)                  { return "GL_INVALID_ENUM"; }
    else if (_errorCode == GL_INVALID_VALUE)                 { return "GL_INVALID_VALUE"; }
    else if (_errorCode == GL_INVALID_OPERATION)             { return "GL_INVALID_OPERATION"; }
    else if (_errorCode == GL_INVALID_FRAMEBUFFER_OPERATION) { return "GL_INVALID_FRAMEBUFFER_OPERATION"; }
    else if (_errorCode == GL_OUT_OF_MEMORY)                 { return "GL_OUT_OF_MEMORY"; }
    else if (_errorCode == GL_NO_ERROR)                      { return "GL_NO_ERROR"; }
    else {
        return "unknown error";
    }
}

const int SCREEN_WIDTH = 1280; const int SCREEN_HEIGHT = 720;

//----------------------------------------------
int main(int argc, char *argv[])
{



    //omp_set_num_threads(1);
    int input, input2;
    SDL_Event keyevent;
    bool eventLoop = true;


    unique_ptr<GridObject> grid_obj(new GridObject());
    unique_ptr<GridEmitter> gridEmit(new GridEmitter(grid_obj.get() ) );
    gridEmit->setNodeName(std::string("emitter"));
    unique_ptr<GridDissipator> gridDiss(new GridDissipator(grid_obj.get() ) );
    gridDiss->setNodeName(std::string("diss"));
    unique_ptr<GridPadCull> gridPad(new GridPadCull(grid_obj.get() ) );
    gridPad->setNodeName(std::string("padcull"));
    auto vecEmit = make_unique<GridVectorEmitter>(grid_obj.get() );
    vecEmit->setNodeName(std::string("vectorEmit"));
    vecEmit->SetChannelName(std::string("velocity"));
    auto basicAdvect = make_unique<GridBasicAdvect>(grid_obj.get() );
    basicAdvect->setNodeName(std::string("basicAdvect"));

    auto bouyancy = make_unique<GridBouyancy>(grid_obj.get() );
    bouyancy->setNodeName(std::string("bouyancy"));

    //gridEmit->IterateGrid();


    SDL_Window* window = NULL; //The surface contained by the window
    SDL_GLContext maincontext; /* Our opengl context handle */

    /* Request opengl 3.2 context.
     * SDL doesn't have the ability to choose which profile at this time of writing,
     * but it should default to the core profile */
    


   if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }

    else { //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL );
        if( window == NULL ) {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
    }

    /* Create our opengl context and attach it to our window */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    maincontext = SDL_GL_CreateContext(window);
    if (maincontext == NULL)
    {
        printf( "context could not be created! SDL_Error: %s\n", SDL_GetError() );
    }
    //SDL_GL_SetSwapInterval(1);
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
    //renderObj->setTranslate(glm::vec3(0.0, 0.0, -2.0));
    renderer->addRenderObject(renderObj);
    //glEnable( GL_MULTISAMPLE );
    glClearColor ( 0.0, 0.0, 0.0, 1.0 );

    glClear ( GL_COLOR_BUFFER_BIT );

    while(eventLoop)

    {
        //glClearColor ( 0.18, 0.18, 0.18, 1.0 );
        double timeA = omp_get_wtime();
        while (SDL_PollEvent(&keyevent))   //Poll our SDL key event for any keystrokes.
        {
            switch(keyevent.type)
            {
            case SDL_KEYDOWN:
                switch(keyevent.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    eventLoop = false;
                    break;

                }

            }
        }


        gridEmit->IterateGrid();
        gridPad->IterateGrid();

        vecEmit->IterateGrid();


        //gridDiss->IterateGrid();
        //bouyancy->IterateGrid();

        basicAdvect->IterateGrid();
        renderer->Render();

        grid_obj->incrementSimTime(0.08);


        /* Swap our back buffer to the front */
        SDL_GL_SwapWindow(window);
        double timeB = omp_get_wtime();
        cout << "total time was " << timeB-timeA << " seconds" << endl << endl;



    }

    /* Clear our buffer with a red background */
    




    SDL_GL_DeleteContext(maincontext);

    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
