
#include "BaseObject.h"
#include <iostream>


using namespace std;



BaseObject::BaseObject(std::string newName)
{
    name = newName;

    //cout << "in BaseObject "<<  name << " constructor" << endl;
    modelMatrix = glm::mat4(1.0);


}





BaseObject::~BaseObject()
{
    //cout << "in BaseObject " << name << " destructor" << endl;



}



void BaseObject::setTranslate(glm::vec3 translate)
{
    //cout << "translating" << endl;
    modelMatrix = glm::translate(glm::mat4(1.0), translate);

}

void BaseObject::setRotate(glm::vec3 rotate)
{

    modelMatrix = glm::rotate(modelMatrix, rotate.x, glm::vec3(1.0f,0.0f,0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotate.y, glm::vec3(0.0f,1.0f,0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotate.z, glm::vec3(0.0f,0.0f,1.0f));



}

void BaseObject::setScale(glm::vec3 scale)
{



}

void BaseObject::Rotate(glm::vec3 rotate)
{



}

void BaseObject::Scale(glm::vec3 scale)
{



}
