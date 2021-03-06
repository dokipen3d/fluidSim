#pragma once

#include "GridOperator.h"
#include <iostream>

using namespace std;

class GridBouyancy : public GridOperator {
public:
  GridBouyancy(GridObject *inGridObject) : GridOperator(inGridObject) {
    cout << "in GriBouyany Constructor" << endl;
    setupDefaults();
  };

  // custom constructor
  virtual void setupDefaults();
  virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                         Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                         uint32_t channel, bool internalAccessible);
  virtual void PreGridOp();
  virtual void GridOp();

  ChannelObject* densitySourceChannel;
};

//#endif // GRIDDISSIPATOR_H
