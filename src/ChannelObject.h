#ifndef CHANNELOBJECT_H
#define CHANNELOBJECT_H

#include "LockFreeHashTable.h"
//#include "glm/fwd.hpp"

#include "GlobalTypes.hpp"
#include "Chunk.h"
#include <memory>
#include <string>

class GridObject;

class ChannelObject {
public:
  ChannelObject(EChannelInfo inInfo, GridObject *parentGrid);
  ~ChannelObject();
  void ClearChunks();
  Chunk *CreateChunk(int32_t x, int32_t y, int32_t z);
  void DeleteChunk(int32_t x, int32_t y, int32_t z);
  Chunk *GetChunk(int32_t x, int32_t y, int32_t z);
  bool ChunkExists(int32_t x, int32_t y, int32_t z);
  float SampleChannelAtPosition(float x, float y, float z);
  glm::vec3 SampleVectorAtPosition(float x, float y, float z);
  glm::vec3 SampleVectorAtPositionExplicit(float x, float y, float z);
  glm::vec3 SampleVectorAtCellFaceFast(float x, float y, float z, u_int32_t channel);
  glm::vec3 SampleVectorAtCellCentreFast(float x, float y, float z);


  //void separateFracInt(const float &in, float &floatPart,
  //                                   u_int32_t &intPart, const float &chunkSize);
  EChannelInfo channelInfo;
  float defaultValue;
  bool boundsSet;
  uint32_t parentChunkSize;
  float parentDx;
  float fParentChunkSize;
  const Chunk *dummyChunk;
  uint32_t numChunks = 0;
  void printChunks();
  int counter = 0;
  bool controlChannel = false;
  int channelAmount;
  float voxelSize;
  u_int32_t parentChunkSizeMinus1;// = parentChunkSize - 1;

  float SampleTrilinear(float x, float y, float z, uint32_t channel);
  float SampleExplicit(float x, float y, float z, uint32_t channel);
  float SampleExplicitAlt(float x, float y, float z);
  bool SampleIsOutsideBounds(float x, float y, float z, uint32_t channel);

  glm::vec3 SampleVectorAtPositionVel(float x, float y, float z);
  float SampleTrilinear2(float x, float y, float z, uint32_t channel);

protected:
  float getRidOfDivCalc;

  GridObject *parentGridObject =
      nullptr; // non owning pointer to parent for finding out chunksize.
  /* data */
  std::unique_ptr<LockFreeHashTable> chunks;

  float (ChannelObject::*SamplerPointerFunc[3])(float x, float y, float z,
                                                uint32_t channel);
  uint32_t currentChannelToSample; // for when we overload interpolate, we need
                                   // to set this so we can iterate throught the
                                   // chunks and return a vec 3
};

#endif
