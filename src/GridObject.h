#ifndef GRIDOBJECT_H
#define GRIDOBJECT_H

#include <vector>
#include <unordered_map>
#include <map>
#include "ChannelObject.h"
#include "GlobalTypes.hpp"
#include <memory>
#include <string>
#include <sstream>

using namespace std;

class ChannelObject;

class GridObject {
public:
  GridObject();
  GridObject(const std::vector<EChannelInfo> &customChannels);

  virtual ~GridObject();
  void resetGrid();
  int chunkSize; // voxel dimensions in chunks
  float bucketSize; // voxelSize;
  void unifyBounds(); // reset the geo bounds from the bucket bounds
  void AddChannel(EChannelInfo inInfo);
  BoundingBox boundingBox;
  std::map<std::string, EChannelInfo> channelDetails; // we can specify "density"
                                                     // or "velocity" and get to
                                                     // right channel
  std::vector<std::unique_ptr<ChannelObject>> channelObjs;
  uint32_t GetMemoryIndexForChannelName(std::string nameQuery);
  void SwapChannelPointers(std::string nameToSwap);

  double simTime; // will put into timeManager
  void incrementSimTime(double timeElapsed);
  float dx;

private:
  void SetDefaultChannels();
  void SetCommonOptions();

  int numChannels; // how many channels there are
  int channelIndex; // for going up two ints
  stringstream myString;
  std::vector<std::string> scalarChannelInit;
  std::vector<std::string> vectorChannelInit;
  std::vector<std::string> sdfChannelInit;
};

#endif
