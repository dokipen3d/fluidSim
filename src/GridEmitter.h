#ifndef GRIDEMITTER_H
#define GRIDEMITTER_H

#include <iostream>
#include <string>
#include <algorithm>
#include <atomic>
using namespace std;

#include "GridTiledOperator.h"
#include "VolumeSource.h"
#include "ImplicitSphere.h"
#include "ImplicitTorus.h"

class Chunk;
class ChannelObject;

//helper function to return the sign of a value.stackoverflow question 1903954
template <typename T>
int sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

template <typename T>
int when_less_than(T a, T b)
{
    return std::min(1.0 - sign(a - b), 1.0 );
}


class GridEmitter : public GridTiledOperator {

public:
  GridEmitter(GridObject *inGridObject) : GridTiledOperator(inGridObject) {
    cout << "in GridEmitter Constructor" << endl;
    setupDefaults();
  };
  virtual ~GridEmitter();

  virtual void setupDefaults();
  virtual void PreGridOp();
  virtual void GridOp();


  virtual void Algorithm(int worldX, int worldY, int worldZ,
                         uint32_t indexX, uint32_t indexY, uint32_t indexZ,
                         const std::vector<float>& inTile,
                         std::vector<float>& outTile,
                         const std::vector<float>& extraTile);

  virtual void ProcessTile(const std::vector<float>& inTile,
                           std::vector<float>& outTile,
                           const std::vector<float>& extraTile,
                           glm::i32vec3 chunkId,
                           Chunk *&pointerRefToSource,
                           Chunk *&pointerRefToTarget);


  float addPositiveDifference(float inputReference, float amountToAdd);
  float bandwidth = 4.0f;
  // virtual void PreChunkOp(Chunk* inChunk, glm::i32vec3 chunkIdSecondary)
  // override;

  virtual void PreChunkOp(Chunk *&inChunk, Chunk *&outChunk, glm::i32vec3 chunkIdSecondary);

  std::atomic<int> bShouldCreateChunk;
  std::atomic<int> countCreated;

  shared_ptr<VolumeSource> sourceVolume;
};

#endif
