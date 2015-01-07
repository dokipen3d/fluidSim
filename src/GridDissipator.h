#ifndef GRIDDISSIPATOR_H
#define GRIDDISSIPATOR_H

#include "GridOperator.h"
#include <iostream>

using namespace std;

class GridDissipator : public GridOperator {
public:
  GridDissipator(GridObject *inGridObject) : GridOperator(inGridObject) {
    cout << "in GridDissipator Constructor" << endl;
    setupDefaults();
  };

  // custom constructor
  virtual void setupDefaults();
  virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                         Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                         uint32_t channel, bool internalAccessible);

  float dissipationRate;
};

#endif // GRIDDISSIPATOR_H
