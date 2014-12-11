#include "GLVertexArrayObject.h"
#include "GLVertexBufferObject.h"
#include "GLShaderObject.h"

const char *GLVertexArrayObject::openGLErrorString(GLenum _errorCode) {
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

void GLVertexArrayObject::CheckGLError(std::string str) {
  error = glGetError();

  if (error != GL_NO_ERROR) {

    printf("Error! %s %s\n", str.c_str(), openGLErrorString(error));
  }
}

GLVertexArrayObject::GLVertexArrayObject() {
  glGenVertexArrays(1, &arrayID);
  CheckGLError("gen va");
}

GLVertexArrayObject::~GLVertexArrayObject() {
  unBind();
  glDeleteVertexArrays(1, &arrayID);
}

void GLVertexArrayObject::bind() {
  glBindVertexArray(arrayID);
  CheckGLError("bind va");
}

void GLVertexArrayObject::unBind() {
  glBindVertexArray(0);
  CheckGLError("unbind va");
}

void GLVertexArrayObject::setVertexAttributeDataPointer(
    GLVertexBufferObject &bufferToBind, GLuint shaderAttributePosition,
    GLint sizeOfData) {

  bufferToBind.bind();
  glVertexAttribPointer(
      shaderAttributePosition, sizeOfData, GL_FLOAT, GL_FALSE, 0,
      NULL); // set vertex pointer offset info for CURRENTLY BOUND BUFFER
  CheckGLError("vert pointer");
  glEnableVertexAttribArray(shaderAttributePosition);
  CheckGLError("attrib erray enable");
}
