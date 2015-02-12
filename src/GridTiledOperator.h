#ifndef GRIDTILEDOPERATOR_H
#define GRIDTILEDOPERATOR_H

#include <vector>
#include <sstream>
#include <string>
#include "glm/fwd.hpp"
#include "GlobalTypes.hpp"


class GridObject;
class ChannelObject;
class Chunk;

struct chunkAddresses2 {
  Chunk *chunkSource;
  Chunk *chunkTarget;
  glm::i32vec3 chunkIndex;
};

class GridTiledOperator {

public:
  GridTiledOperator(GridObject *inGridObject, ETileImportType processType);
  virtual ~GridTiledOperator();

  virtual void setupDefaults() = 0;
  void SetGridObject(GridObject *inGridObject);
#pragma omp declare simd
  virtual void Algorithm(int worldX, int worldY, int worldZ, int indexX, int indexY, int indexZ) = 0; // pure virtual. out chunk might
                                                // not be needed but if we write
                                                // a node that copies into a new
                                                // grid we need to know where
  virtual void IterateGrid();
  virtual void PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                           glm::i32vec3 chunkIdSecondary);
  virtual void PreChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                          glm::i32vec3 chunkIdSecondary);

  virtual void PreGridOp();

  virtual void GridOp();
  void copyRawInput(glm::i32vec3 chunkCoords, uint32_t channel);

  void copyStaggeredInput();
  void copyFiniteDifferenceInput(glm::i32vec3 chunkCoords, Chunk* main, uint32_t channel);
  void copyTileBack(glm::i32vec3 chunkCoords);

  void SetChannelName(std::string nameToSet);
  std::string name;
  void setNodeName(std::string nameIn);
  float dx;
  bool forceInputBoundsIteration = false;
  bool callPreChunkOp = true;
  bool callGridOp = false;
  bool callPostChunkOp = false;
  void refreshSourceAndTargetChannelDetails();
  u_int32_t numberOfIterations = 1;
  int startVoxel = 0;
  int skipAmount = 1;
    bool debug = false;
  std::vector<chunkAddresses2> chunks;
  std::vector<float> inTile;
  std::vector<float> outTile;

protected:
  double currentTime;
  GridObject *gridObjectPtr;
  ChannelObject *currentSourceChannelObject;
  ChannelObject *currentTargetChannelObject; // might make this a vector of
                                             // pointers to dynamically add
                                             // channels and pick from multiple
  EChannelType typeToOperateOn;
  ETileImportType tileImportType;
  std::string channelName;
  std::stringstream myString;
  bool createChunks;
  BoundingBox boundingBox;
  int chnkSize;
  uint32_t chunkOpCounter = 0;
  uint32_t totalChunksToOperateOn;
  int32_t internalChannels = 1;
  int iteration;



};

#endif
