#include "RenderableObject.h"
#include <iostream>
#include "glm/gtc/type_ptr.hpp"
#include <array>
#include "ImplicitSphere.h"

using namespace std;

const char *RenderableObject::openGLErrorString(GLenum _errorCode) {
  // Only 3.2+ Core and ES 2.0+ errors, no deprecated strings like stack
  // underflow etc.
  if (_errorCode == GL_INVALID_ENUM) {
    return "GL_INVALID_ENUM";
  } else if (_errorCode == GL_INVALID_VALUE) {
    return "GL_INVALID_VALUE";
  } else if (_errorCode == GL_INVALID_OPERATION) {
    return "GL_INVALID_OPERATION";
  } else if (_errorCode == GL_INVALID_FRAMEBUFFER_OPERATION) {
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  } else if (_errorCode == GL_OUT_OF_MEMORY) {
    return "GL_OUT_OF_MEMORY";
  } else if (_errorCode == GL_NO_ERROR) {
    return "GL_NO_ERROR";
  } else {
    return "unknown error";
  }
}

void RenderableObject::CheckGLError(string str) {
  error = glGetError();

  if (error != GL_NO_ERROR) {

    printf("Error! %s %s\n", str.c_str(), openGLErrorString(error));
  }
}

void RenderableObject::setupObjects() {

  // glEnable(GL_TEXTURE_3D);
  // CheckGLError("enable3d");

  // cout << "in actual Renderable constructor" << endl;
  shader = std::make_unique<GLShaderObject>();
  volumeShader = std::make_unique<GLShaderObject>();

  textureA = std::make_unique<GLTextureObject>(
      "", GL_TEXTURE_RECTANGLE); // path to texture or blank if going to be
                                 // rendering into it
  textureB = std::make_unique<GLTextureObject>("", GL_TEXTURE_RECTANGLE);
  ThreeDee = std::make_unique<GLTextureObject>("", GL_TEXTURE_3D);
  CheckGLError("make3d");

  pos = std::make_unique<GLVertexBufferObject>();
  col = std::make_unique<GLVertexBufferObject>();
  vao = std::make_unique<GLVertexArrayObject>();
  fboFront = std::make_unique<GLFrameBufferObject>();
  fboBack = std::make_unique<GLFrameBufferObject>();

  textureA->resize(1280, 720);
  textureB->resize(1280, 720);
  // textureC->resize(1280, 720);

  glEnable(GL_CULL_FACE);

  // cout << "loading shaders" << endl;
  if (!shader->loadProgram(
          "/Users/delliott/Documents/Projects/fluidSim/src/vertexShader.vert",
          "/Users/delliott/Documents/Projects/fluidSim/src/"
          "fragmentShader.frag")) {

    printf("Unable to load basic shader!\n");
  }

  // glEnable (GL_TEXTURE_RECTANGLE_ARB);
  // cout << "loading shaders" << endl;
  if (!volumeShader->loadProgram(
          "/Users/delliott/Documents/Projects/fluidSim/src/vertexShader.vert",
          "/Users/delliott/Documents/Projects/fluidSim/src/fluidNew.fsh")) {

    printf("Unable to load volume shader!\n");
  }
  bbox = gridObjectPtr->boundingBox;
  sampleObject = gridObjectPtr->channelObjs[0].get();
  //    cout << "bbox is " << bbox.min.x << " to " << bbox.max.x << endl;
  //    cout << "bbox is " << bbox.min.y << " to " << bbox.max.y << endl;
  //    cout << "bbox is " << bbox.min.z << " to " << bbox.max.z << endl;

  updateQuad(bbox.minX, bbox.minY, bbox.minZ, bbox.maxX, bbox.maxY,
             bbox.maxZ);

  auto emitterSphere = make_shared<ImplicitSphere>(name);
  sourceVolume = emitterSphere;
  // updateQuad(0, 0, 0, 1, 1,1);

  vao->bind(); // first bind vao to start recording state
  // std::cout << "first bind" << std::endl;
  pos->uploadData(108 * sizeof(float), positions); // this binds the buffer for
                                                   // us under the hood. this
                                                   // binds the buffer and
                                                   // stores it in the vao
  vao->setVertexAttributeDataPointer(
      *pos, shader->getAttributeLocation("position"),
      3); // enables vertex array attributes and sets up pointer data

  col->uploadData(108 * sizeof(float), colours);
  vao->setVertexAttributeDataPointer(*col,
                                     shader->getAttributeLocation("colour"), 3);
  vao->unBind();

  // projectionMatrixLocation2 = glGetUniformLocation(shader->getProgramID(),
  // "projectionView2"); // Get the location of our projection matrix in the
  // shader
  // cout << "projection location2 = " << projectionMatrixLocation2 << endl;
  projectionMatrixLocation =
      glGetUniformLocation(shader->getProgramID(),
                           "projectionView"); // Get the location of our
                                              // projection matrix in the shader
  // cout << "projection location = " << projectionMatrixLocation << endl;
  projectionMatrixLocation2 =
      glGetUniformLocation(volumeShader->getProgramID(),
                           "projectionView"); // Get the location of our
                                              // projection matrix in the shader
  // cout << "projection location2 = " << projectionMatrixLocation2 << endl;

  texLocBack = glGetUniformLocation(volumeShader->getProgramID(), "texBack");
  CheckGLError("get uni back");

  texLocFront = glGetUniformLocation(volumeShader->getProgramID(), "texFront");
  CheckGLError("get uni front");

  texLocVolume =
      glGetUniformLocation(volumeShader->getProgramID(), "volumeTexture");
  cout << "volume 3d loc is " << texLocVolume << endl;
  CheckGLError("get uni vol");

  /*
  vbo = 0; //initialize to default 0 state
  vboC = 0;
  vao = 0;

  cout << "generating buffers" << endl;

  glGenBuffers(1, &vbo); // Generate 1 buffer
  cout << "generating buffers 2" << endl;


  glGenBuffers(1, &vboC); // Generate 1 buffer

  cout << "generating array" << endl;

  glGenVertexArrays(1, &vao); //Generate 1 vertex array

  cout << "updating quad" << endl;


  glBindVertexArray (vao);//bind array to start recording state of what buffers
  are used and whats in them
  glBindBuffer (GL_ARRAY_BUFFER, vbo);//bind this buffer. its bind state gets
  stored in the above vao
  cout << "buffer data 1" << endl;

  glBufferData (GL_ARRAY_BUFFER, 108 * sizeof (float), positions,
  GL_STATIC_DRAW);//upload data to buffer for position
  GLint location1 = shader->getAttributeLocation("position");//get location of
  input in shader
  glVertexAttribPointer (location1, 3, GL_FLOAT, GL_FALSE, 0, NULL);//set vertex
  pointer offset info for CURRENTLY BOUND BUFFER


  glBindBuffer (GL_ARRAY_BUFFER, vboC);
  cout << "buffer data 2" << endl;

  glBufferData (GL_ARRAY_BUFFER, 108 * sizeof (float), colors, GL_STATIC_DRAW);
  GLint location2 = shader->getAttributeLocation("colour");

  glVertexAttribPointer (location2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  glEnableVertexAttribArray (0);
  glEnableVertexAttribArray (1);


  glBindVertexArray (0);
  */

  glActiveTexture(GL_TEXTURE0);
  fboBack->bindTexture(*textureA);

  CheckGLError("bind text A");
  glActiveTexture(GL_TEXTURE1);
  fboFront->bindTexture(*textureB);

  // glFrontFace(GL_CW);
  CheckGLError("front face");
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // GLenum attatch[] = {GL_COLOR_ATTACHMENT0_EXT};

  // glDrawBuffers(1, attatch);
  // CheckGLError("draw buffers");

  // volumeShader->bind();
  // fillTexture();
}

RenderableObject::~RenderableObject() {
  delete threeD;
  // std::cout << "in render object " << name << " destructor" << std::endl;
}

void RenderableObject::SetGridObject(GridObject *inGridObject) {
  gridObjectPtr = inGridObject;
}

void RenderableObject::fillTexture() {
  // cout << "uploading 3d data" << endl;
  bbox = gridObjectPtr->boundingBox;
  updateQuad(bbox.minX, bbox.minY, bbox.minZ, bbox.maxX, bbox.maxY,
             bbox.maxZ);
  vao->bind(); // first bind vao to start recording state
  // std::cout << "first bind" << std::endl;
  pos->uploadData(108 * sizeof(float), positions);
  vao->unBind();

  // std::array<float, 64*64*64> threeDdata;
  // float threeDdata[res*res*res];
  // threeD = new float[res*res*res];
  //std::fill_n(threeD, (res * res * res), 0);
  uint32_t divergenceTarget =
  gridObjectPtr->GetMemoryIndexForChannelName(std::string("density"));
 cout << "channel index for rendering is " << divergenceTarget << endl;
  sampleObject = gridObjectPtr->channelObjs[divergenceTarget].get();
  // cout << "in renerable " << sampleObject << endl << endl;

  int implicit = 0;

  const float div = 1.0 / (res);

    cout << "render bbox is " << bbox.minX << " " << bbox.maxX << endl;
//    cout << "bbox is " << bbox.min.y << " " << bbox.max.y << endl;
//    cout << "bbox is " << bbox.min.z << " " << bbox.max.z << endl;

#pragma omp parallel for collapse(3)
  for (int k = 0; k < res; k++) {
    for (int j = 0; j < res; j++) {
      for (int i = 0; i < res; i++) {
        float posX = glm::mix(bbox.minX.load(memory_order_relaxed), bbox.maxX.load(memory_order_relaxed), div * i);
        float posY = glm::mix(bbox.minY.load(memory_order_relaxed), bbox.maxY.load(memory_order_relaxed), div * j);
        float posZ = glm::mix(bbox.minZ.load(memory_order_relaxed), bbox.maxZ.load(memory_order_relaxed), div * k);
        // cout << "pos " << posX << endl;
        //                if (implicit == 0)
        //                {
        // float sample =
        threeD[i + (j * res) + (k * res * res)] =
           // sampleObject->SampleTrilinear(posX, posY, posZ, 0)*1.0f;

                //sampleObject->SampleTrilinear(posX-0.5, posY-0.5, posZ-0.5, 0)*20.0f;

                glm::abs(sampleObject->SampleTrilinear(posX , posY, posZ, 0))*2.0f;
        // cout << sample << " ";
        //}

        //                else
        //                {
        //                    float sample =
        //                    sourceVolume->sampleVolume(glm::vec3(posX,
        //                    posY,posZ));

        //                    if (sample < 0)
        //                        threeD[i+(j*res)+(k*(res*res))] = 1;
        //                    else

        //                        threeD[i+(j*res)+(k*res*res)] = 0;
        //                        //cout << sample << " ";
        //                }
      }
    }

    // cout << "k = " << k <<endl;
  }

  // ThreeDee->resizeWith3dData(64, 64, 64, void *data);
  // cout << threeDdata[24] <<endl;
  glActiveTexture(GL_TEXTURE2);
  // CheckGLError("actice texture2");

  ThreeDee->resizeWith3dData(res, res, res, threeD, GL_R32F, GL_RED);

  // CheckGLError("upload texture");
  // delete threeD;
}

void RenderableObject::Render(glm::mat4 ProjectionViewMatrix) {
  glEnable(GL_CULL_FACE);

  // glDisable(GL_TEXTURE_RECTANGLE);
  // CheckGLError("disable tex");
  glClear(GL_COLOR_BUFFER_BIT);

  // glBindVertexArray (vao);
  vao->bind();
  // CheckGLError("bind vao in render");

  shader->bind();
  glm::mat4 renderMat = ProjectionViewMatrix * modelMatrix;
  glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE,
                     glm::value_ptr(renderMat));

  glCullFace(GL_BACK);

  // textureA->bind();

  fboBack->bind();

  // CheckGLError("bind fbo");

  glClearColor(0.0, 0.0, 0.0, 1.0);

  if (fboBack->checkFrameBufferStatus()) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    CheckGLError("draw");
  }
  fboBack->unBind();
  // CheckGLError("unbind fbo");

  glCullFace(GL_FRONT);

  // textureB->bind();
  fboFront->bind();

  if (fboFront->checkFrameBufferStatus()) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  fboFront->unBind();

  shader->unbind();

  // CheckGLError("unbind shader");

  glClearColor(0.18, 0.18, 0.2, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glCullFace(GL_BACK);

  ThreeDee->bind();

  glDisable(GL_CULL_FACE);

  volumeShader->bind();
  glUniformMatrix4fv(projectionMatrixLocation2, 1, GL_FALSE,
                     glm::value_ptr(renderMat));
  glUniform1i(texLocBack, 0);
  // CheckGLError("uni1");
  glUniform1i(texLocFront, 1);
  // CheckGLError("uni2");
  glUniform1i(texLocVolume, 2);
  // CheckGLError("uni3");

  glDrawArrays(GL_TRIANGLES, 0, 36);

  volumeShader->unbind();

  vao->unBind();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);

  // glClear(GL_COLOR_BUFFER_BIT);

  //    fboBack->readBind();
  //    if(fboBack->checkFrameBufferReadStatus())
  //    {
  //        //std::cout << "blit 1" << std::endl;
  //         glBlitFramebuffer(0, 0, 1280,

  //         720, 0, 0,

  //         1280/4.0, 720/4.0,

  //         GL_COLOR_BUFFER_BIT, GL_NEAREST);
  //    }

  //    fboFront->readBind();
  //    if(fboFront->checkFrameBufferReadStatus())
  //    {
  //        //std::cout << "blit 2" << std::endl;

  //         glBlitFramebuffer(0, 0, 1280,

  //         720, 1280/4.0, 0,

  //         (1280/4.0)*2, (720/4.0),

  //         GL_COLOR_BUFFER_BIT, GL_NEAREST);
  //     }

  //    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
  //    CheckGLError("glback fbo");

  glBindVertexArray(0);
}


GLfloat RenderableObject::colours[108] =

    {

     0.0f, 0.0f,
     0.0f,

     1.0f, 0.0f,
     0.0f,

     1.0f, 1.0f,
     0.0f,

     0.0f, 0.0f,
     0.0f,

     1.0f, 1.0f,
     0.0f,

     0.0f, 1.0f,
     0.0f,

     0.0f, 0.0f,
     0.0f,

     1.0f, 0.0f,
     1.0f,

     1.0f, 0.0f,
     0.0f,

     0.0f, 0.0f,
     0.0f,

     0.0f, 0.0f,
     1.0f,

     1.0f, 0.0f,
     1.0f,

     0.0f, 0.0f,
     0.0f,

     0.0f, 1.0f,
     0.0f,

     0.0f, 1.0f,
     1.0f,

     0.0f, 0.0f,
     0.0f,

     0.0f, 1.0f,
     1.0f,

     0.0f, 0.0f,
     1.0f,

     1.0f, 1.0f,
     1.0f,

     1.0f, 1.0f,
     0.0f,

     1.0f, 0.0f,
     0.0f,

     1.0f, 1.0f,
     1.0f,

     1.0f, 0.0f,
     0.0f,

     1.0f, 0.0f,
     1.0f,

     1.0f, 1.0f,
     1.0f,

     0.0f, 1.0f,
     0.0f,

     1.0f, 1.0f,
     0.0f,

     1.0f, 1.0f,
     1.0f,

     0.0f, 1.0f,
     1.0f,

     0.0f, 1.0f,
     0.0f,

     1.0f, 1.0f,
     1.0f,

     1.0f, 0.0f,
     1.0f,

     0.0f, 0.0f,
     1.0f,

     1.0f, 1.0f,
     1.0f,

     0.0f, 0.0f,
     1.0f,

     0.0f, 1.0f,
     1.0f

};

GLfloat RenderableObject::quad[12] =

    {

     0.0f, 0.0f,
     0.0f,

     1.0f, 0.0f,
     0.0f,

     1.0f, 0.0f,
     1.0f,

     0.0f, 0.0f,
     1.0f

};

GLfloat RenderableObject::quadTex[8] =

    {

     0.0f, 0.0f,

     1.0f, 0.0f,

     1.0f, 1.0f,

     0.0f, 1.0f

};

void RenderableObject::updateQuad(float minX, float minY, float minZ,
                                  float maxX, float maxY, float maxZ) {

  positions[0] = minX;
  positions[1] = minY;
  positions[2] = minZ;

  positions[3] = maxX;
  positions[4] = minY;
  positions[5] = minZ;

  positions[6] = maxX;
  positions[7] = maxY;
  positions[8] = minZ;

  positions[9] = minX;
  positions[10] = minY;
  positions[11] = minZ;

  positions[12] = maxX;
  positions[13] = maxY;
  positions[14] = minZ;

  positions[15] = minX;
  positions[16] = maxY;
  positions[17] = minZ;

  positions[18] = minX;
  positions[19] = minY;
  positions[20] = minZ;

  positions[21] = maxX;
  positions[22] = minY;
  positions[23] = maxZ;

  positions[24] = maxX;
  positions[25] = minY;
  positions[26] = minZ;

  positions[27] = minX;
  positions[28] = minY;
  positions[29] = minZ;

  positions[30] = minX;
  positions[31] = minY;
  positions[32] = maxZ;

  positions[33] = maxX;
  positions[34] = minY;
  positions[35] = maxZ;

  positions[36] = minX;
  positions[37] = minY;
  positions[38] = minZ;

  positions[39] = minX;
  positions[40] = maxY;
  positions[41] = minZ;

  positions[42] = minX;
  positions[43] = maxY;
  positions[44] = maxZ;

  positions[45] = minX;
  positions[46] = minY;
  positions[47] = minZ;

  positions[48] = minX;
  positions[49] = maxY;
  positions[50] = maxZ;

  positions[51] = minX;
  positions[52] = minY;
  positions[53] = maxZ;

  positions[54] = maxX;
  positions[55] = maxY;
  positions[56] = maxZ;

  positions[57] = maxX;
  positions[58] = maxY;
  positions[59] = minZ;

  positions[60] = maxX;
  positions[61] = minY;
  positions[62] = minZ;

  positions[63] = maxX;
  positions[64] = maxY;
  positions[65] = maxZ;

  positions[66] = maxX;
  positions[67] = minY;
  positions[68] = minZ;

  positions[69] = maxX;
  positions[70] = minY;
  positions[71] = maxZ;

  positions[72] = maxX;
  positions[73] = maxY;
  positions[74] = maxZ;

  positions[75] = minX;
  positions[76] = maxY;
  positions[77] = minZ;

  positions[78] = maxX;
  positions[79] = maxY;
  positions[80] = minZ;

  positions[81] = maxX;
  positions[82] = maxY;
  positions[83] = maxZ;

  positions[84] = minX;
  positions[85] = maxY;
  positions[86] = maxZ;

  positions[87] = minX;
  positions[88] = maxY;
  positions[89] = minZ;

  positions[90] = maxX;
  positions[91] = maxY;
  positions[92] = maxZ;

  positions[93] = maxX;
  positions[94] = minY;
  positions[95] = maxZ;

  positions[96] = minX;
  positions[97] = minY;
  positions[98] = maxZ;

  positions[99] = maxX;
  positions[100] = maxY;
  positions[101] = maxZ;

  positions[102] = minX;
  positions[103] = minY;
  positions[104] = maxZ;

  positions[105] = minX;
  positions[106] = maxY;
  positions[107] = maxZ;
}
