#include "ImplicitSphere.h"
#include "glm/exponential.hpp" // glm::pow
#include <iostream>

using namespace std;

void ImplicitSphere::setepDefaults() {
  // glm::vec3
  this->boundingBox.setfmin(-1*radius, -1*radius, -1*radius);
  this->boundingBox.setfmax(radius, radius, radius);
  cout << "bbox of sphere is " << this->boundingBox.minX << " "
       << this->boundingBox.maxX << endl;
}

glm::float32 ImplicitSphere::sampleVolume(glm::vec3 pos) {
  float X = pos.x * pos.x;
  float Y = pos.y * pos.y;
  float Z = pos.z * pos.z;

  float ret = (glm::sqrt(X + Y + Z)) - radius;

  // cout << "sampling" << pos.x << " " << pos.y << " " <<pos.z << endl;
  return ret;
  // return (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) - radius;
}
