#ifndef GLOBALTYPES_H
#define GLOBALTYPES_H

#define GLM_FORCE_RADIANS

//#include "glm/fwd.hpp"

#include "glm/vec3.hpp"
#include "glm/gtc/type_precision.hpp"
#include <string>
#include <atomic>

enum class ESamplerMode {trilinear, cubic, catmull_rom};
enum class EChannelType {scalar, sdf, vector};

//raw is straight memcpy in and out no padding
//fd is copy centre in rows and pad 1 voxel around edges
//staggered is copy 1 edge at top middle and side.
enum class ETileImportType { raw, finiteDifference, finiteDifferenceSingleChannel, staggered};

//----------------------------------------------



struct BoundingBoxCustom
{

    glm::vec3 min;				//Bounding box in world space for things like geo and world bounds
	glm::vec3 max;
    glm::i32vec3 fluidMin = glm::i32vec3(0,0,0);		//bounding box in chunkSpace;
	glm::i32vec3 fluidMax = glm::i32vec3(0,0,0);		//
    BoundingBoxCustom(): min(glm::vec3(0.0)), max(glm::vec3(0.0)){};
        BoundingBoxCustom(glm::vec3 inMin, glm::vec3 inMax) : min(inMin), max(inMax){};



};

struct BoundingBox
{

    //Bounding box in world space for things like geo and world bounds

    std::atomic<float> minX;
    std::atomic<float> minY;
    std::atomic<float> minZ;
    std::atomic<float> maxX;
    std::atomic<float> maxY;
    std::atomic<float> maxZ;

    //integer coords for chunks

    std::atomic<int> fluidMinX;
    std::atomic<int> fluidMinY;
    std::atomic<int> fluidMinZ;
    std::atomic<int> fluidMaxX;
    std::atomic<int> fluidMaxY;
    std::atomic<int> fluidMaxZ;



    BoundingBox(): minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0){};
        BoundingBox(float inMinX, float inMinY, float inMinZ,
                          float inMaxX, float inMaxY, float inMaxZ ) :
                            minX(inMinX), minY(inMinY), minZ(inMinZ),
                            maxX(inMaxX), maxY(inMaxY), maxZ(inMaxZ){};

    BoundingBox(const BoundingBox& BBox){

        minX = BBox.minX.load(std::memory_order_relaxed);
        minY = BBox.minY.load(std::memory_order_relaxed);
        minZ = BBox.minZ.load(std::memory_order_relaxed);
        maxX = BBox.maxX.load(std::memory_order_relaxed);
        maxY = BBox.maxY.load(std::memory_order_relaxed);
        maxZ = BBox.maxZ.load(std::memory_order_relaxed);

        fluidMinX = BBox.fluidMinX.load(std::memory_order_relaxed);
        fluidMinY = BBox.fluidMinY.load(std::memory_order_relaxed);
        fluidMinZ = BBox.fluidMinZ.load(std::memory_order_relaxed);
        fluidMaxX = BBox.fluidMaxX.load(std::memory_order_relaxed);
        fluidMaxY = BBox.fluidMaxY.load(std::memory_order_relaxed);
        fluidMaxZ = BBox.fluidMaxZ.load(std::memory_order_relaxed);


    }

    BoundingBox& operator=( const BoundingBox& BBox ) {
        minX = BBox.minX.load(std::memory_order_relaxed);
        minY = BBox.minY.load(std::memory_order_relaxed);
        minZ = BBox.minZ.load(std::memory_order_relaxed);
        maxX = BBox.maxX.load(std::memory_order_relaxed);
        maxY = BBox.maxY.load(std::memory_order_relaxed);
        maxZ = BBox.maxZ.load(std::memory_order_relaxed);

        return *this;
      }

    void setmin(int inMinX, int inMinY, int inMinZ)
    {
        fluidMinX = std::min(inMinX, fluidMinX.load());
        fluidMinY = std::min(inMinY, fluidMinY.load());
        fluidMinZ = std::min(inMinZ, fluidMinZ.load());

    }

    void setmax(int inMaxX, int inMaxY, int inMaxZ)
    {
        fluidMaxX = std::max(inMaxX, fluidMaxX.load());
        fluidMaxY = std::max(inMaxY, fluidMaxY.load());
        fluidMaxZ = std::max(inMaxZ, fluidMaxZ.load());

    }

    void setfmin(float inMinX, float inMinY, float inMinZ)
    {
        minX = std::min(inMinX, minX.load());
        minY = std::min(inMinY, minY.load());
        minZ = std::min(inMinZ, minZ.load());
    }

    void setfmax(float inMaxX, float inMaxY, float inMaxZ)
    {
        maxX = std::max(inMaxX, maxX.load());
        maxY = std::max(inMaxY, maxY.load());
        maxZ = std::max(inMaxZ, maxZ.load());
    }


};

//----------------------------------------------
struct EChannelInfo
{
	uint32_t memoryIndex;
	std::string channelName;
        EChannelType channelType;
        EChannelInfo(){};
        EChannelInfo(int gridObjectMemIndex, std::string inName, EChannelType inType) : memoryIndex(gridObjectMemIndex), channelName(inName), channelType(inType){};

};


//namespace std {


//        template<typename T, typename ...Args>
//        std::unique_ptr<T> make_unique( Args&& ...args )
//        {
//            return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
//        }



//}


#endif
