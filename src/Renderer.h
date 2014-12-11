#ifndef RENDERER_H
#define RENDERER_H

#include "CameraObject.h"
#include "RenderableObject.h"

#include <memory>
#include <vector>

class Renderer {

public:
  Renderer();
  virtual ~Renderer();

  void Render();
  void addRenderObject(std::shared_ptr<RenderableObject> &inRenderObjectPtr);

private:
  std::unique_ptr<CameraObject> camera;
  std::vector<std::shared_ptr<RenderableObject>> renderObjects;
};

#endif
