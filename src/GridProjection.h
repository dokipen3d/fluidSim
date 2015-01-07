#ifndef GRIDPROJECTION_H
#define GRIDPROJECTION_H

#include "GridOperator.h"
#include "ChannelObject.h"
#include <iostream>

using namespace std;

class GridProjection : public GridOperator
{
public:
    GridProjection(GridObject *inGridObject) : GridOperator(inGridObject) {
      cout << "in GridProjection Constructor" << endl;
      setupDefaults();
    };
    virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelWorldPosition,
                           Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                           uint32_t channel, bool internalAccessible);
    virtual void setupDefaults();

    virtual ~GridProjection();
    float scale;
    float scaleSquared;
    virtual void PreGridOp();


protected:
    ChannelObject* pressureSource;
};

#endif //
