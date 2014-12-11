#ifndef GLTEXTUREOBJECT_H
#define GLTEXTUREOBJECT_H
#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <string>

class GLTextureObject {
public:
  GLTextureObject(std::string filePath, GLenum textureTypeIn);
  virtual ~GLTextureObject();
  void setType(GLenum type);
  void resize(int w, int h);
  void resizeWith3dData(int w, int h, int d, void *data, GLint internalFormat,
                        GLenum incomingFormat);

  void setRenderTargetMode(bool mode);
  void bind();
  GLuint getTextureID();
  GLenum getTextureType();

private:
  GLenum textureType;
  GLuint textureID;
  void setupTexture();
  uint32_t width;
  uint32_t height;
  uint32_t depth;
  bool fboMode;
  const char *openGLErrorString(GLenum _errorCode);
  void CheckGLError(std::string str);

  GLenum error;
};

#endif
