#include "GLTextureObject.h"
#include <iostream>
#include <string>


const char* GLTextureObject::openGLErrorString( GLenum _errorCode )
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

void GLTextureObject::CheckGLError(std::string str)
{
    error = glGetError();

    if( error != GL_NO_ERROR )
    {

        printf( "Error! %s %s\n", str.c_str(), openGLErrorString( error ) );
    }
}	

GLTextureObject::~GLTextureObject()
{


    glDeleteTextures(1,&textureID);



}


GLTextureObject::GLTextureObject(std::string filePath, GLenum textureTypeIn)

{
    //textureID = 0;
    error = GL_NO_ERROR;
    //default texture type
    textureType = textureTypeIn;


    CheckGLError("unpack");

    glGenTextures(1, &this->textureID);
    CheckGLError("gen");
    glBindTexture(textureType, textureID);
    CheckGLError("binded");

    if(filePath.size() == 0)//for fbo drawing
    {
        if (textureType == GL_TEXTURE_RECTANGLE)
        {
            setRenderTargetMode(true);
            glTexImage2D(textureType, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, NULL);//default size
            CheckGLError("tex image");
        }

        if (textureType == GL_TEXTURE_3D)
        {

            glTexImage3D(textureType, 0, GL_R32F, 64,64, 64, 0, GL_RED, GL_FLOAT, NULL);
            CheckGLError("tex image");

        }
    }

    else
    {
        setRenderTargetMode(false);
        std::cout << "fbo mode false" << std::endl;
        //do custom loading here inclusing resetting size based on loaded file. reset function below is only for fbo stuff
    }



}

void GLTextureObject::bind()
{

    glBindTexture(textureType, textureID);
    CheckGLError("bind texture");

}

GLuint GLTextureObject::getTextureID()
{
    return textureID;
}

GLenum GLTextureObject::getTextureType()
{
    return textureType;
}



void GLTextureObject::setRenderTargetMode(bool mode)
{

    fboMode = mode;

}

void GLTextureObject::setType(GLenum type)
{

    textureType = type;


}




void GLTextureObject::resize(int w, int h)
{
    if (fboMode == false)
    {

        //cant do resizing in this function as its only for resizing for fbo drawing
        return;
    }

    else
    {
        width = w;
        height = h;

        bind();
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(textureType, 0, GL_RGBA32F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
        CheckGLError("resize");
    }

}

void GLTextureObject::resizeWith3dData(int w, int h, int d, void *data, GLint internalFormat, GLenum incomingFormat)
{

    setRenderTargetMode(false);
    textureType = GL_TEXTURE_3D;
    width = w;
    height = h;
    depth = d;
    this->bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//GL_NEAREST
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//GL_LINEAR
    glTexImage3D(textureType, 0, internalFormat, w, h, d, 0, incomingFormat, GL_FLOAT, data);
    CheckGLError("resize");
}


