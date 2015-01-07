#ifndef GRIDPRESSURERBGS_H
#define GRIDPRESSURERBGS_H

#include "GridOperator.h"
#include "ChannelObject.h"
#include <iostream>

using namespace std;

class GridPressureRBGS : public GridOperator
{
public:
    GridPressureRBGS(GridObject *inGridObject) : GridOperator(inGridObject) {
      cout << "in GridDivergence Constructor" << endl;
      setupDefaults();
    };
    virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                           Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                           uint32_t channel, bool internalAccessible);
    virtual void setupDefaults();

    virtual ~GridPressureRBGS();
    virtual void PreGridOp();
    float scale;
    float scaleSquared;

    float omega = 1.5;

    virtual void GridOp();
protected:
    ChannelObject* divergenceSource;
    virtual void PreChunkOp(Chunk *&inChunk, Chunk *&outChunk, glm::i32vec3 chunkIdSecondary);
};

#endif // GRIDDIVERGENCE_H
