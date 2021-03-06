#ifndef GRIDMACCORMACKVELADVECT_H
#define GRIDMACCORMACKVELADVECT_H

#include <iostream>
#include <string>
using namespace std;

#include "GridOperator.h"
#include "VolumeSource.h"

class Chunk;
class ChannelObject;
class GridObject;

class GridMacCormackVelAdvect : public GridOperator {

public:
  GridMacCormackVelAdvect(GridObject *inGridObject) : GridOperator(inGridObject) {
    cout << "in GridEmitter Constructor" << endl;
    setupDefaults();
  };
  virtual ~GridMacCormackVelAdvect();

  virtual void setupDefaults();
  virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                         Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                         uint32_t channnel, bool internalAccessible);
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
