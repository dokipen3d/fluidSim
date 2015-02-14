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



struct BoundingBox
{

    glm::vec3 min;				//Bounding box in world space for things like geo and world bounds
	glm::vec3 max;
    glm::i32vec3 fluidMin = glm::i32vec3(0,0,0);		//bounding box in chunkSpace;
	glm::i32vec3 fluidMax = glm::i32vec3(0,0,0);		//
    BoundingBox(): min(glm::vec3(0.0)), max(glm::vec3(0.0)){};
	BoundingBox(glm::vec3 inMin, glm::vec3 inMax) : min(inMin), max(inMax){};



};

struct BoundingBoxCustom
{

    //Bounding box in world space for things like geo and world bounds

    std::atomic<float> minX;
    std::atomic<float> minY;
    std::atomic<float> minZ;
    std::atomic<float> maxX;
    std::atomic<float> maxY;
    std::atomic<float> maxZ;

    std::atomic<int> fluidMinX;
    std::atomic<int> fluidMinY;
    std::atomic<int> fluidMinZ;
    std::atomic<int> fluidMaxX;
    std::atomic<int> fluidMaxY;
    std::atomic<int> fluidMaxZ;



    BoundingBoxCustom(): minX(0), minY(0), minZ(0), maxX(0), maxY(0), maxZ(0){};
        BoundingBoxCustom(float inMinX, float inMinY, float inMinZ,
                          float inMaxX, float inMaxY, float inMaxZ ) :
                            minX(inMinX), minY(inMinY), minZ(inMinZ),
                            maxX(inMaxX), maxY(inMaxY), maxZ(inMaxZ){};

    void setminX(int inMinX)
    {
        fluidMinX = std::min(inMinX, fluidMinX.load());
    }
    void setminY(int inMinY)
    {
        fluidMinY = std::min(inMinY, fluidMinY.load());
    }
    void setminZ(int inMinZ)
    {
        fluidMinZ = std::min(inMinZ, fluidMinZ.load());
    }
    void setmaxX(int inMaxX)
    {
        fluidMaxX = std::max(inMaxX, fluidMaxX.load());
    }
    void setmaxY(int inMaxY)
    {
        fluidMaxY = std::max(inMaxY, fluidMaxY.load());
    }
    void setmaxZ(int inMaxZ)
    {
        fluidMaxZ = std::max(inMaxZ, fluidMaxZ.load());
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
