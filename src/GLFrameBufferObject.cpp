#include "GLFrameBufferObject.h"
#include "GLTextureObject.h"
#include <iostream>

GLFrameBufferObject::GLFrameBufferObject() {
  if (glIsFramebuffer(frameBufferID) == false) {
    glGenFramebuffers(1, &frameBufferID);
    CheckGLError("fbo init");
    setType(GL_DRAW_FRAMEBUFFER);
  }
}

GLFrameBufferObject::~GLFrameBufferObject() {
  glDeleteFramebuffers(1, &frameBufferID);
}

const char *GLFrameBufferObject::openGLErrorString(GLenum _errorCode) {
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

void GLFrameBufferObject::CheckGLError(std::string str) {
  error = glGetError();

  if (error != GL_NO_ERROR) {

    printf("Error! %s %s\n", str.c_str(), openGLErrorString(error));
  }
}

bool GLFrameBufferObject::bind() {
  glBindFramebuffer(frameBufferType, frameBufferID);
  CheckGLError("fbo bind");

  return true;
}

bool GLFrameBufferObject::readBind() {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferID);
  CheckGLError("fbo readBind");
  return true;
}

void GLFrameBufferObject::unBind() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
                    0); // set back to standard opengl buffer
  CheckGLError("fbo unbind");
}

void GLFrameBufferObject::setType(GLenum type) { frameBufferType = type; }

void GLFrameBufferObject::bindTexture(GLTextureObject &textureToBindTo) {
  if (glIsTexture(textureToBindTo.getTextureID())) {
    textureToBindTo.bind();
    bind();
    glFramebufferTexture2D(frameBufferType, GL_COLOR_ATTACHMENT0,
                           textureToBindTo.getTextureType(),
                           textureToBindTo.getTextureID(), 0);
    CheckGLError("fbo bind texture");
    unBind();

  } else {
    std::cout << "not a texture" << std::endl;
  }
}

bool GLFrameBufferObject::checkFrameBufferStatus() {
  if (glCheckFramebufferStatus(frameBufferType) == GL_FRAMEBUFFER_COMPLETE) {

    return true;
  } else {
    std::cout << "fbo not complete" << std::endl;
    return false;
  }
}

bool GLFrameBufferObject::checkFrameBufferReadStatus() {
  if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) ==
      GL_FRAMEBUFFER_COMPLETE) {

    return true;
  } else {
    std::cout << "fbo read not complete" << std::endl;
    return false;
  }
}
