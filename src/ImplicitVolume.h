#ifndef IMPLICITVOLUME_H
#define IMPLICITVOLUME_H

#include "VolumeSource.h"

class ImplicitVolume : public VolumeSource {
public:
  ImplicitVolume(std::string newName) : VolumeSource(newName){};
};

#endif // IMPLICITVOLUME_H
