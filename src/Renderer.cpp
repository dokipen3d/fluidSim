#include "Renderer.h"
#include <iostream>
#include <string>

using namespace std;

Renderer::Renderer() {
  // cout << "in renderer constructor" << endl;
  camera = make_unique<CameraObject>(string("cam"));
}

Renderer::~Renderer() {}

void Renderer::Render() {
  for (auto x : renderObjects) {
    //camera->moveCamUp();
    x->fillTexture();
    x->Render(camera->GetRenderMatrix());
    // x->Render(glm::mat4(1));
  }
}

void Renderer::RenderSame()
{
    for (auto x : renderObjects) {
      //camera->moveCamUp();
      x->Render(camera->GetRenderMatrix());
      // x->Render(glm::mat4(1));
    }
}

void Renderer::addRenderObject(
    std::shared_ptr<RenderableObject> &inRenderObjectPtr) {
  auto newRenderObject = inRenderObjectPtr;
  // cout << "pushing back shared ptr" << endl;
  renderObjects.push_back(newRenderObject);
}
