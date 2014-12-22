#ifndef GRIDDIVERGENCE_H
#define GRIDDIVERGENCE_H

#include "GridOperator.h"
#include <iostream>

using namespace std;

class GridDivergence : public GridOperator
{
public:
    GridDivergence(GridObject *inGridObject) : GridOperator(inGridObject) {
      cout << "in GridDivergence Constructor" << endl;
      setupDefaults();
    };
    virtual void Algorithm(glm::i32vec3 chunkId, glm::i32vec3 voxelPosition,
                           Chunk *inChunk, Chunk *outChunk, uint32_t dataIndex,
                           uint32_t channel);
    virtual void setupDefaults();

    virtual ~GridDivergence();
    virtual void PreGridOp();
    float scale;
    ChannelObject* velocitySourceChannelObject;
};

#endif // GRIDDIVERGENCE_H
