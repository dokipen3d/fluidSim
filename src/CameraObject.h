#ifndef CAMERAOBJECT_H
#define CAMERAOBJECT_H

#include "BaseObject.h"
#include <iostream>

#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp"

using namespace std;

class CameraObject : public BaseObject {

public:
  CameraObject(std::string newName) : BaseObject(newName) {
    camY = 60;
    camX = -0.0f;
    camZ = -100.0;
    setupDefaults();

    // std::cout << "in camera constructor" << std::endl;
  };
  void setupDefaults();
  void moveCamUp();
  virtual ~CameraObject();
  glm::mat4 GetRenderMatrix();
  glm::mat4 projectionMatrix;

  float camY;
  float camX;
  float camZ;
};

#endif
