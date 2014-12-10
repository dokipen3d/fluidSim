#ifndef GLVERTEXBUFFEROBJECT_H
#define GLVERTEXBUFFEROBJECT_H

#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>

class GLVertexBufferObject
{
public:
    GLVertexBufferObject();
    virtual ~GLVertexBufferObject();
    void bind();
    void unbind();
    GLuint getProgramID();
    void setType(GLenum type);
    void uploadData(GLsizeiptr  size,  const GLvoid *data);
    const char* openGLErrorString( GLenum _errorCode );
    void CheckGLError(std::string str);


private:

    GLenum bufferType;
    GLenum bufferDrawType;
    GLuint bufferID;
    GLenum error;

};

#endif // GLVERTEXBUFFEROBJECT_H
