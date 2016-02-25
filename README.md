#fluidSim
========

##Fluid Simulation for fire &amp; smoke

###main.cpp
Here we create the GridObject and a few GridOperators which operate on the data that the GridObject manages. A renderable object also is created which is able to take care of basic opengl viewport rendering.

###GridObject
GridObject is the 'instance' of our simulation. This is in charge of higher level management of the grid, such as time tracking, channels available and their types. The main resolution of the grid is set here.


###ChannelObjects
Inside the gridobject there are some 'channelObjects'. These manage the actual data through a lockFreeHashTable. Each channel can be a vector or scalar. The hash table stored pointers to 'chunks' of voxels. Creation and Deletion of chunks for each channel are called here. You can also directly sample the data by passing in a location to the ChannelObject.

###GridTiledOperator
Base Class for operating on the data. It scans the channelObjects for active chunks and then goes though each chunk, copies the data in, performs a kernel operation on the data and then copies out. The copying is for cache usage (see older tiledOperator for how it accessed each value individually before).
