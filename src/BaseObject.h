#ifndef BASEOBJECT_H
#define BASEOBJECT_H

#define GLM_FORCE_RADIANS



#include "glm/fwd.hpp"

#include <memory>
#include <string>
#include "GlobalTypes.hpp"

#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "glm/mat4x4.hpp" // glm::mat4


class BaseObject
{


public:
    BaseObject(std::string newName);
    virtual ~BaseObject();


    void setTranslate(glm::vec3 translate);
    void setRotate(glm::vec3 rotate);
    void setScale(glm::vec3 scale);

    void Translate(glm::vec3 translate);
    void Rotate(glm::vec3 rotate);
    void Scale(glm::vec3 scale);

    std::string name;

    glm::mat4 modelMatrix;
    BoundingBox boundingBox;


private:








};








#endif
