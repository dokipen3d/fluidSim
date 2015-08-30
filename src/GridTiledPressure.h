#ifndef GRIDTILEDPRESSURE_H
#define GRIDTILEDPRESSURE_H

#include "GridTiledOperator.h"
#include "ChannelObject.h"
#include <iostream>

using namespace std;

class GridTiledPressure : public GridTiledOperator
{
public:
    GridTiledPressure(GridObject *inGridObject, std::vector<std::string> inChannelNames) : GridTiledOperator(inGridObject,inChannelNames) {
      cout << "in GridDivergence Constructor" << endl;
      setupDefaults();
    };
    #pragma omp declare simd
    virtual void Algorithm(int worldX, int worldY, int worldZ,
                           uint32_t indexX, uint32_t indexY, uint32_t indexZ,
                           const std::vector<float>& inTile,
                           std::vector<float>& outTile,
                           const std::vector<float>& extraTile);
    virtual void setupDefaults();
    float calcPressure(float a, float b, float c, float d, float e, float f, float g, float h);

    virtual ~GridTiledPressure();
    virtual void PreGridOp();
    float scale;
    float scaleSquared;

    float omega = 1.5;

    virtual void GridOp();
protected:
    ChannelObject* divergenceSource;
    virtual void PreChunkOp(Chunk *&inChunk, Chunk *&outChunk, glm::i32vec3 chunkIdSecondary);
    virtual void PostChunkOp(Chunk *&inChunk, Chunk *&outChunk,
                             glm::i32vec3 chunkIdSecondary);
};


#endif // GRIDDIVERGENCE_H
