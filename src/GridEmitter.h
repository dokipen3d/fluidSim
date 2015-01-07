#ifndef GRIDEMITTER_H
#define GRIDEMITTER_H

#include <iostream>
#include <string>
using namespace std;

#include "GridOperator.h"
#include "VolumeSource.h"

class Chunk;
class ChannelObject;

class GridEmitter : public GridOperator {

public:
  GridEmitter(GridObject *inGridObject) : GridOperator(inGridObject) {
    cout << "in GridEmitter Constructor" << endl;
    setupDefaults();
  };
  virtual ~GridEmitter();

  virtual void setupDefaults();
  virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                         Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                         uint32_t channel, bool internalAccessible);
  float addPositiveDifference(float inputReference, float amountToAdd);
  float bandwidth = 4.0f;
  // virtual void PreChunkOp(Chunk* inChunk, glm::i32vec3 chunkIdSecondary)
  // override;

  shared_ptr<VolumeSource> sourceVolume;
};

#endif
