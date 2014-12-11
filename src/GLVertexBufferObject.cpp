#include "GLVertexBufferObject.h"

const char *GLVertexBufferObject::openGLErrorString(GLenum _errorCode) {
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

void GLVertexBufferObject::CheckGLError(std::string str) {
  error = glGetError();

  if (error != GL_NO_ERROR) {

    printf("Error! %s %s\n", str.c_str(), openGLErrorString(error));
  }
}

GLVertexBufferObject::GLVertexBufferObject() {
  glGenBuffers(1, &bufferID);
  bufferType = GL_ARRAY_BUFFER;
  bufferDrawType = GL_STATIC_DRAW;
}

GLVertexBufferObject::~GLVertexBufferObject() { glDeleteBuffers(1, &bufferID); }

void GLVertexBufferObject::bind() {
  glBindBuffer(bufferType, bufferID);
  CheckGLError("bind vbo");
}

void GLVertexBufferObject::unbind() {
  glBindBuffer(bufferType, 0);
  CheckGLError("unbind vbo");
}

GLuint GLVertexBufferObject::getProgramID() { return bufferID; }

void GLVertexBufferObject::setType(GLenum type) { bufferType = type; }

void GLVertexBufferObject::uploadData(GLsizeiptr size, const GLvoid *data) {
  bind();
  glBufferData(bufferType, size, data, bufferDrawType);
  CheckGLError("buffer data");
}
