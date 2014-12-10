#include "ImplicitSphere.h"
#include "glm/exponential.hpp"// glm::pow
#include <iostream>

using namespace std;


void ImplicitSphere::setepDefaults()
{
    //glm::vec3
    this->boundingBox.min = glm::vec3(-1*radius);
    this->boundingBox.max = glm::vec3(radius);
    cout << "bbox of sphere is " << this->boundingBox.min.x << " " << this->boundingBox.max.x<< endl;

}

glm::float32 ImplicitSphere::sampleVolume(glm::vec3 pos)
{
    float X = pos.x*pos.x;
    float Y = pos.y*pos.y;
    float Z = pos.z*pos.z;

    //cout << "sampling" << pos.x << " " << pos.y << " " <<pos.z << endl;
    return (glm::sqrt(X + Y + Z )) -radius ;
    //return (pos.x*pos.x + pos.y*pos.y + pos.z*pos.z) - radius;
}
