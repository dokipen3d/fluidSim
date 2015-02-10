#ifndef GLOBALTYPES_H
#define GLOBALTYPES_H

#define GLM_FORCE_RADIANS

//#include "glm/fwd.hpp"

#include "glm/vec3.hpp"
#include "glm/gtc/type_precision.hpp"
#include <string>


enum class SamplerMode {trilinear, cubic, catmull_rom};
enum class ChannelType {scalar, sdf, vector};

//raw is straight memcpy in and out no padding
//fd is copy centre in rows and pad 1 voxel around edges
//staggered is copy 1 edge at top middle and side.
enum class TileImportType { raw, finiteDifference, staggered};

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

//----------------------------------------------
struct ChannelInfo
{
	uint32_t memoryIndex;
	std::string channelName;
	ChannelType channelType;
	ChannelInfo(){};
	ChannelInfo(int gridObjectMemIndex, std::string inName, ChannelType inType) : memoryIndex(gridObjectMemIndex), channelName(inName), channelType(inType){};

};


//namespace std {


//        template<typename T, typename ...Args>
//        std::unique_ptr<T> make_unique( Args&& ...args )
//        {
//            return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
//        }



//}


#endif
