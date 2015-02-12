#ifndef IMPLICITTORUS_H
#define IMPLICITTORUS_H

#include "ImplicitVolume.h"

class ImplicitTorus : public ImplicitVolume {
public:
  ImplicitTorus(std::string newName) : ImplicitVolume(newName) {
    setepDefaults();
  };
  void setepDefaults();
  virtual float sampleVolume(glm::vec3 pos);
  float radiusA = 64;
  float radiusB = 24;
};

#endif // IMPLICITSPHERE_H
