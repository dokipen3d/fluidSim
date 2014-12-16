#ifndef RENDERABLEOBJECT_H
#define RENDERABLEOBJECT_H

#include "BaseObject.h"
#include "GL/glew.h"
#include <memory>

#include <iostream>
#include <string>

#include "GLShaderObject.h"
#include "GLTextureObject.h"
#include "GLVertexArrayObject.h"
#include "GLVertexBufferObject.h"
#include "GLFrameBufferObject.h"
#include "GridObject.h"
#include "ChannelObject.h"
#include "VolumeSource.h"

class RenderableObject : public BaseObject {

public:
  RenderableObject(std::string newName, GridObject *inGridObject)
      : BaseObject(newName) {
    // std::cout << "in render object " << newName << " over loaded constructor"
    // << std::endl;
    gridObjectPtr = inGridObject;
    setupObjects();
    res = 256;
    threeD = new float[res * res * res];
    // fillTexture();
  };
  virtual ~RenderableObject();
  void Render(glm::mat4 ProjectionViewMatrix);

  void updateQuad(float minX, float minY, float minZ, float maxX, float maxY,
                  float maxZ);
  const char *openGLErrorString(GLenum _errorCode);
  void CheckGLError(std::string str);
  void setupObjects();
  void SetGridObject(GridObject *inGridObject);
  void fillTexture();
  BoundingBox bbox;
  ChannelObject *sampleObject;
  float *threeD;
  int res;

private:
  std::unique_ptr<GLShaderObject> shader, volumeShader;
  std::unique_ptr<GLTextureObject> textureA, textureB, ThreeDee;
  std::unique_ptr<GLVertexBufferObject> pos, col;
  std::unique_ptr<GLVertexArrayObject> vao;
  std::unique_ptr<GLFrameBufferObject> fboFront, fboBack;

  GridObject *gridObjectPtr;

  GLenum error;
  // GLuint vbo;
  // GLuint vboC;

  // GLuint vao;
  GLint projectionMatrixLocation;
  GLint projectionMatrixLocation2;
  GLint texLocFront;
  GLint texLocBack;
  GLint texLocVolume;

  shared_ptr<VolumeSource> sourceVolume;

  GLfloat positions[108];
  static GLfloat colours[108];
  static GLfloat quad[12];
  static GLfloat quadTex[8];
};

#endif
