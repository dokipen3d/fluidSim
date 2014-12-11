#ifndef GRIDPADCULL_H
#define GRIDPADCULL_H

#include "GridOperator.h"
#include <iostream>

class Chunk;
class ChannelObject;
class GridObject;

class GridPadCull : public GridOperator {
public:
  GridPadCull(GridObject *inGridObject) : GridOperator(inGridObject) {
    std::cout << "in GridPadCull Constructor" << std::endl;
    setupDefaults();
  };

  virtual void setupDefaults() override;
  virtual void PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                           glm::i32vec3 chunkIdSecondary) override;
  virtual void GridOp() override;

  virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelLocalPosition,
                         Chunk *inChunk, Chunk *outChunk, u_int32_t dataIndex,
                         uint32_t channel); // pure virtual. out chunk might not
                                            // be needed but if we write a node
                                            // that copies into a new grid we
                                            // need to know where
  signed int chunksToPad = 1;
};

#endif // GRIDPADCULL_H
