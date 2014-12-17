#ifndef GRIDPRESSURE_H
#define GRIDPRESSURE_H

#include "GridOperator.h"
#include "ChannelObject.h"
#include <iostream>

using namespace std;

class GridPressure : public GridOperator
{
public:
    GridPressure(GridObject *inGridObject) : GridOperator(inGridObject) {
      cout << "in GridDivergence Constructor" << endl;
      setupDefaults();
    };
    virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                           Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                           uint32_t channel);
    virtual void setupDefaults();

    virtual ~GridPressure();
    virtual void PreGridOp();
    float scale;
    float scaleSquared;

    virtual void GridOp();
protected:
    ChannelObject* divergenceSource;
    virtual void PreChunkOp(Chunk *&inChunk, Chunk *&outChunk, glm::i32vec3 chunkIdSecondary);
};

#endif // GRIDDIVERGENCE_H
