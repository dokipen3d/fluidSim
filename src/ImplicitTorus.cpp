#include "ImplicitTorus.h"
#include "glm/exponential.hpp" // glm::pow
#include "glm/gtx/rotate_vector.hpp"
#include <iostream>

using namespace std;

void ImplicitTorus::setepDefaults() {
  // glm::vec3
  this->boundingBox.min = glm::vec3(-1 * (radiusA + (radiusB)));
  this->boundingBox.max = glm::vec3(radiusA + (radiusB));
  cout << "bbox of torus is " << this->boundingBox.min.x << " "
       << this->boundingBox.max.x << endl;
}

glm::float32 ImplicitTorus::sampleVolume(glm::vec3 pos) {
  pos =  glm::rotateZ(pos, 1.57079633f);
  float R = radiusA;
  float r = radiusB;
  float R2 = glm::pow(R, 2.0f);
  float r2 = r * r;
  // P is given by the expression engine ala prman sl
  float x2 = pos.z * pos.z;
  float y2 = pos.y * pos.y;
  float z2 = pos.x * pos.x;

  return glm::pow(R - glm::sqrt(x2 + y2), 2.0f) + z2 - r2;
}
