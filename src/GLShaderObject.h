//
//  GLShaderObject
//  
//
//  Created by Daniel Elliott on 10/09/13.
//
//



#ifndef GLSHADEROBJECT_H
#define GLSHADEROBJECT_H

#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

class GLShaderObject
{

    
    
    
public:
    
    GLShaderObject();
    virtual ~GLShaderObject();
    bool loadProgram(const char *vertex_path, const char *fragment_path);
    virtual void freeProgram();
    bool bind();
    void unbind();
    void setTextureUnits();
    GLint getAttributeLocation(std::string name);
    
    GLuint getProgramID();
    std::string readFile(const char *filePath);
    const char* openGLErrorString( GLenum _errorCode );
    void CheckGLError(std::string str);



    
protected:
    void printProgramLog( GLuint program);
    void printShaderLog(GLuint shader);
    
    GLuint mProgramID;
    GLuint tex1UniformLoc, tex2UniformLoc;
    GLuint projectionMatrixLocation;
    GLenum error;
    
    
private:
    
    
};


#endif













