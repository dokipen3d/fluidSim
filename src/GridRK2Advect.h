#ifndef GRIDRK2ADVECT_H
#define GRIDRK2ADVECT_H

#include <iostream>
#include <string>
using namespace std;

#include "GridOperator.h"
#include "VolumeSource.h"

class Chunk;
class ChannelObject;
class GridObject;

class GridRK2Advect : public GridOperator {

public:
  GridRK2Advect(GridObject *inGridObject) : GridOperator(inGridObject) {
    cout << "in GridEmitter Constructor" << endl;
    setupDefaults();
  };
  virtual ~GridRK2Advect();

  virtual void setupDefaults();
  virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                         Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                         uint32_t channnel);
  // float addPositiveDifference(float inputReference, float amountToAdd);
  // virtual void PreChunkOp(Chunk* inChunk, glm::i32vec3 chunkIdSecondary)
  // override;
  ChannelObject *velocitySourceChannelObject;
  virtual void PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                          glm::i32vec3 chunkIdSecondary) override;
  virtual void PreGridOp();
  virtual void GridOp() override;

  shared_ptr<VolumeSource> sourceVolume;
};

#endif
