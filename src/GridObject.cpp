#include "GridObject.h"
#include <iostream>

// channels are only created properly in reset grid. if we want to add another
// channel, then we need to add a channel info item and resetGrid will create it
// for us.

using namespace std;

//----------------------------------------------
GridObject::GridObject() {
  myString << "setting grid object" << endl;
  SetDefaultChannels();
  SetCommonOptions();
  resetGrid();
  simTime = 0.0;

  //    for (int i = 0; i < 1; i++)
  //    {

  //        channelObjs[(channelDetails["density"].memoryIndex)]->CreateChunk(i,0,0);

  //    }

  cout << myString.str();
  myString.clear();
  {
    // Testing more stuff
    //         {
    //            //loop for testing creation of chunks
    //            for (int i = 0; i < 2000; i++)
    //            {
    //                    channelObjs[(channelDetails["density"].memoryIndex)]->CreateChunk(i,0,0);
    //                    bounding_box_.fluidMax = glm::i32vec3(
    //                    glm::max(bounding_box_.fluidMax.x, i),
    //                                                        glm::max(bounding_box_.fluidMax.y,
    //                                                        1),
    //                                                        glm::max(bounding_box_.fluidMax.z,
    //                                                        1)
    //                                                    );
    //                    bounding_box_.fluidMin = glm::i32vec3(
    //                    glm::min(bounding_box_.fluidMin.x, i),
    //                                                        glm::min(bounding_box_.fluidMin.y,
    //                                                        1),
    //                                                        glm::min(bounding_box_.fluidMin.z,
    //                                                        1)
    //                                                    );

    //            }

    // 	//loop for destroying chunks using the bounds
  }
}

//----------------------------------------------
GridObject::GridObject(const std::vector<ChannelInfo> &customChannels) {
  SetCommonOptions();

  for (auto it = customChannels.begin(); it != customChannels.end(); it++) {
    AddChannel(*it);
  }
}

//----------------------------------------------
GridObject::~GridObject() {

  // cout << "grid Object destroyed" << endl;
}

//----------------------------------------------
void GridObject::AddChannel(ChannelInfo inInfo) {
  channelDetails[inInfo.channelName] =
      ChannelInfo(numChannels, inInfo.channelName, inInfo.channelType);
  numChannels++;
  cout << "channel added" << endl;
  resetGrid();
}

//----------------------------------------------
void GridObject::SetCommonOptions() {
  chunkSize = 8;
  bucketSize = 1;
  numChannels = 0;
  boundingBox.min = glm::vec3(0.0);
  boundingBox.max = glm::vec3(0.0);
  boundingBox.fluidMin = glm::i32vec3(0, 0, 0);
  boundingBox.fluidMax = glm::i32vec3(0, 0, 0);
}

//----------------------------------------------
uint32_t GridObject::GetMemoryIndexForChannelName(std::string nameQuery) {
  return channelDetails[nameQuery].memoryIndex;
}

//----------------------------------------------
void GridObject::SwapChannelPointers(
    std::string nameToSwap) // change direction of writing to
{

  int channelIndexToSwap = channelDetails[nameToSwap].memoryIndex;
  // cout << "changing " << nameToSwap << channelIndexToSwap << endl;
  channelObjs[channelIndexToSwap].swap(channelObjs.at(channelIndexToSwap));
  std::swap(channelObjs.at(channelIndexToSwap),
            channelObjs.at(channelIndexToSwap + 1));
  // cout << " swapping " << channelObjs.at(channelIndexToSwap).get() << " with
  // " << channelObjs.at(channelIndexToSwap+1).get() << endl;
}

void GridObject::incrementSimTime(double timeElapsed) {
  simTime += timeElapsed;
}

//----------------------------------------------
void GridObject::SetDefaultChannels() {
  channelIndex = 0;
  numChannels = 0;
  // prepare default channels
  // scalarChannelInit =
  // {string("density"),string("temperature"),string("fuel"),string("pressure")
  // };
  scalarChannelInit = {string("density"), string("pressure")};

  vectorChannelInit = {string("velocity")};

  // prepare the info structures for passing into the channel objects
  for (int i = 0; i < scalarChannelInit.size(); i++) {
    channelDetails[scalarChannelInit.at(i)] =
        ChannelInfo(channelIndex, scalarChannelInit.at(i), ChannelType::scalar);

    channelIndex++;
    channelIndex++;

    numChannels++;
  }

  for (int i = 0; i < vectorChannelInit.size(); i++) {
    channelDetails[vectorChannelInit.at(i)] =
        ChannelInfo(channelIndex, vectorChannelInit.at(i), ChannelType::vector);
    channelIndex++;
    channelIndex++;

    numChannels++;
  }

  for (int i = 0; i < sdfChannelInit.size(); i++) {
    channelDetails[sdfChannelInit.at(i)] =
        ChannelInfo(channelIndex, sdfChannelInit.at(i), ChannelType::sdf);
    channelIndex++;
    channelIndex++;

    numChannels++;
  }

  myString << "channel defaults set up as no custom channels were specified. "
              "numchannel is " << numChannels << std::endl;
}

//----------------------------------------------
void GridObject::resetGrid() {
  // reset all channel and create new ones if needed
  // clear existing ones
  channelObjs.clear();

  // create new channel objects and fill with channel info
  for (auto it = channelDetails.begin(); it != channelDetails.end(); it++) {
    // std::unique_ptr<ChannelObject> pChannel_objA(new
    // ChannelObject(it->second, this));
    auto pChannel_objA = make_unique<ChannelObject>(it->second, this);
    channelObjs.emplace_back(std::move(pChannel_objA));
    // std::unique_ptr<ChannelObject> pChannel_objB(new
    // ChannelObject(it->second, this));
    auto pChannel_objB = make_unique<ChannelObject>(it->second, this);
    channelObjs.emplace_back(std::move(pChannel_objB));
    myString << "\"" << channelObjs.back()->channelInfo.channelName << "\""
             << endl;
    ;
  }
  // channelObjs.front()->controlChannel = true;
  channelObjs.at(0)->controlChannel = true;
  channelObjs.at(1)->controlChannel = true;

  cout << " chanObjs size is " << channelObjs.size() << endl;
  cout << "grid reset" << endl;
}

void GridObject::unifyBounds() {

  boundingBox.min.x = boundingBox.fluidMin.x * chunkSize;
  boundingBox.min.y = boundingBox.fluidMin.y * chunkSize;
  boundingBox.min.z = boundingBox.fluidMin.z * chunkSize;
  boundingBox.max.x = (boundingBox.fluidMax.x + 1) * chunkSize;
  boundingBox.max.y = (boundingBox.fluidMax.y + 1) * chunkSize;
  ;
  boundingBox.max.z = (boundingBox.fluidMax.z + 1) * chunkSize;
  ;
}
