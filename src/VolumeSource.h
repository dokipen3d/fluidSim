#ifndef VOLUMESOURCE_H
#define VOLUMESOURCE_H

#include "glm/fwd.hpp"
#include "BaseObject.h"

class VolumeSource : public BaseObject {
public:
  VolumeSource(std::string newName) : BaseObject(newName){};
  virtual float sampleVolume(glm::vec3 pos) = 0;
};

#endif // VOLUMESOURCE_H
