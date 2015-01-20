#ifndef IMPLICITSPHERE_H
#define IMPLICITSPHERE_H

#include "ImplicitVolume.h"

class ImplicitSphere : public ImplicitVolume {
public:
  ImplicitSphere(std::string newName) : ImplicitVolume(newName) {
    setepDefaults();
  };
  void setepDefaults();
  virtual float sampleVolume(glm::vec3 pos);
  float radius = 53;


};

#endif // IMPLITSPHERE_H
