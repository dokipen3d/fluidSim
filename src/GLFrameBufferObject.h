#ifndef GLFRAMEBUFFEROBJECT_H
#define GLFRAMEBUFFEROBJECT_H

#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <string>

class GLTextureObject;

class GLFrameBufferObject {
public:
  GLFrameBufferObject();
  virtual ~GLFrameBufferObject();

  const char *openGLErrorString(GLenum _errorCode);
  void CheckGLError(std::string str);
  bool bind();
  bool readBind();
  void unBind();
  void setType(GLenum type);
  void bindTexture(GLTextureObject &textureToBindTo);
  GLenum frameBufferType;
  GLuint frameBufferID;
  GLenum error;
  bool checkFrameBufferStatus();
  bool checkFrameBufferReadStatus();
};

#endif // GLFRAMEBUFFEROBJECT_H
