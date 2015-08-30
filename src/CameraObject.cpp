#include "CameraObject.h"
#include <iostream>

void CameraObject::setupDefaults() {

  projectionMatrix = glm::perspective(20.0f, 16.0f / 9.0f, 0.05f, 1000.f);
  modelMatrix = glm::lookAt(
      glm::vec3(camX, camY, camZ), // Camera is at (4,3,3), in World Space
      glm::vec3(0, 40, 0),         // and looks at the origin
      glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)
      );
}

void CameraObject::moveCamUp() {
  camY += 0.0;
  //camZ -= 0.6;
  //camX -= 0.2;

  setupDefaults();
}

CameraObject::~CameraObject() {
  // std::cout << "in camera destructor" << std::endl;
}

glm::mat4 CameraObject::GetRenderMatrix() {

  // return projectionMatrix * glm::inverse(modelMatrix);
  return projectionMatrix * modelMatrix;
}
