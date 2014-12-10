#ifndef GLVERTEXARRAYOBJECT_H
#define GLVERTEXARRAYOBJECT_H

#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <OpenGL/gl.h>

class GLVertexBufferObject;

class GLVertexArrayObject
{
public:
    GLVertexArrayObject();
    virtual ~GLVertexArrayObject();
    void bind();
    void unBind();
    void setVertexAttributeDataPointer(GLVertexBufferObject &bufferToBind,
                                       GLuint shaderAttributePosition,
                                       GLint sizeOfData);
    const char* openGLErrorString( GLenum _errorCode );
    void CheckGLError(std::string str);


private:
    GLenum error;

    GLuint arrayID;
};

#endif // GLVERTEXARRAYOBJECT_H
