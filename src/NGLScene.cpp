#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/SimpleVAO.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/ShaderLib.h>
#include <ngl/Transformation.h>
#include <iostream>
NGLScene::NGLScene()
{
  setTitle("Morph Mesh Demo");
  m_weight1 = 0.0;
  m_weight2 = 0.0;
  m_animation = true;
  m_punchLeft = false;
  m_punchRight = false;
  m_timerLeft = new QTimer();
  m_timerRight = new QTimer();
  connect(m_timerLeft, SIGNAL(timeout()), this, SLOT(updateLeft()));
  connect(m_timerRight, SIGNAL(timeout()), this, SLOT(updateRight()));
}
void NGLScene::punchLeft()
{
  if (m_punchLeft != true)
  {
    m_weight1 = 0.0;
    m_timerLeft->start(4);
    m_punchLeft = true;
  }
}

void NGLScene::punchRight()
{
  if (m_punchRight != true)
  {
    m_weight2 = 0.0;
    m_timerRight->start(4);
    m_punchRight = true;
  }
}

// a simple structure to hold our vertex data
struct vertData
{
  ngl::Vec3 p1;
  ngl::Vec3 n1;
  ngl::Vec3 p2;
  ngl::Vec3 n2;
  ngl::Vec3 p3;
  ngl::Vec3 n3;
};
void NGLScene::createMorphMesh()
{

  // base pose is mesh 1 stored in m_meshes[0]
  // get the obj data so we can process it locally
  std::vector<ngl::Vec3> verts1 = m_meshes[0]->getVertexList();
  // should really check to see if the poses match if we were doing this properly
  std::vector<ngl::Vec3> verts2 = m_meshes[1]->getVertexList();
  std::vector<ngl::Vec3> verts3 = m_meshes[2]->getVertexList();
  // faces will be the same for each mesh so only need one
  std::vector<ngl::Face> faces = m_meshes[0]->getFaceList();
  // now get the normals
  std::vector<ngl::Vec3> normals1 = m_meshes[0]->getNormalList();
  std::vector<ngl::Vec3> normals2 = m_meshes[1]->getNormalList();
  std::vector<ngl::Vec3> normals3 = m_meshes[2]->getNormalList();

  // now we are going to process and pack the mesh into an ngl::VertexArrayObject
  std::vector<vertData> vboMesh;
  vertData d;
  auto nFaces = faces.size();
  // loop for each of the faces
  for (unsigned int i = 0; i < nFaces; ++i)
  {
    // now for each triangle in the face (remember we ensured tri above)
    for (unsigned int j = 0; j < 3; ++j)
    {
      // pack in the vertex data first

      d.p1 = verts1[faces[i].m_vert[j]];
      // the blend meshes are just the differences so we subtract the base mesh
      // from the current one (could do this on GPU but this saves processing time)
      d.p2 = verts2[faces[i].m_vert[j]] - d.p1;
      d.p3 = verts3[faces[i].m_vert[j]] - d.p1;

      // now do the normals
      d.n1 = normals1[faces[i].m_norm[j]];
      // again we only need the differences so subtract base mesh value from pose values
      d.n2 = normals2[faces[i].m_norm[j]] - d.n1;
      d.n3 = normals3[faces[i].m_norm[j]] - d.n1;

      // finally add it to our mesh VAO structure
      vboMesh.push_back(d);
    }
  }
  // first we grab an instance of our VOA class as a TRIANGLE_STRIP
  m_vaoMesh = ngl::VAOFactory::createVAO("simpleVAO", GL_TRIANGLES);
  // next we bind it so it's active for setting data
  m_vaoMesh->bind();
  auto meshSize = vboMesh.size();
  // now we have our data add it to the VAO, we need to tell the VAO the following
  // how much (in bytes) data we are copying
  // a pointer to the first element of data (in this case the address of the first element of the
  // std::vector
  m_vaoMesh->setData(ngl::AbstractVAO::VertexData(meshSize * sizeof(vertData), vboMesh[0].p1.m_x));

  // so data is Vert / Normal for each mesh
  m_vaoMesh->setVertexAttributePointer(0, 3, GL_FLOAT, sizeof(vertData), 0);
  m_vaoMesh->setVertexAttributePointer(1, 3, GL_FLOAT, sizeof(vertData), 3);

  m_vaoMesh->setVertexAttributePointer(2, 3, GL_FLOAT, sizeof(vertData), 6);
  m_vaoMesh->setVertexAttributePointer(3, 3, GL_FLOAT, sizeof(vertData), 9);

  m_vaoMesh->setVertexAttributePointer(4, 3, GL_FLOAT, sizeof(vertData), 12);
  m_vaoMesh->setVertexAttributePointer(5, 3, GL_FLOAT, sizeof(vertData), 15);

  m_vaoMesh->setNumIndices(meshSize);
  // finally we have finished for now so time to unbind the VAO
  m_vaoMesh->unbind();
}

void NGLScene::changeWeight(Weights _w, Direction _d)
{

  switch (_w)
  {
  case Weights::POSE1:
    if (_d == Direction::UP)
      m_weight1 += 0.1;
    else
      m_weight1 -= 0.1;
    break;

  case Weights::POSE2:
    if (_d == Direction::UP)
      m_weight2 += 0.1;
    else
      m_weight2 -= 0.1;
    break;
  }
  // clamp to 0.0 -> 1.0 range
  m_weight1 = std::min(1.0f, std::max(0.0f, m_weight1));
  m_weight2 = std::min(1.0f, std::max(0.0f, m_weight2));
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0, 10, 40);
  ngl::Vec3 to(0, 10, 0);
  ngl::Vec3 up(0, 1, 0);

  // first we create a mesh from an obj passing in the obj file and texture
  std::unique_ptr<ngl::Obj> mesh1(new ngl::Obj("models/BrucePose1.obj"));
  m_meshes.push_back(std::move(mesh1));

  std::unique_ptr<ngl::Obj> mesh2(new ngl::Obj("models/BrucePose2.obj"));
  m_meshes.push_back(std::move(mesh2));

  std::unique_ptr<ngl::Obj> mesh3(new ngl::Obj("models/BrucePose3.obj"));
  m_meshes.push_back(std::move(mesh3));
  createMorphMesh();

  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45, 720.0f / 576.0f, 0.05f, 350);
  // we are creating a shader called PerFragADS
  ngl::ShaderLib::createShaderProgram("PerFragADS");
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader("PerFragADSVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("PerFragADSFragment", ngl::ShaderType::FRAGMENT);
  // attach the source
  ngl::ShaderLib::loadShaderSource("PerFragADSVertex", "shaders/PerFragASDVert.glsl");
  ngl::ShaderLib::loadShaderSource("PerFragADSFragment", "shaders/PerFragASDFrag.glsl");
  // compile the shaders
  ngl::ShaderLib::compileShader("PerFragADSVertex");
  ngl::ShaderLib::compileShader("PerFragADSFragment");
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram("PerFragADS", "PerFragADSVertex");
  ngl::ShaderLib::attachShaderToProgram("PerFragADS", "PerFragADSFragment");

  // now we have associated this data we can link the shader
  ngl::ShaderLib::linkProgramObject("PerFragADS");
  // and make it active ready to load values
  ngl::ShaderLib::use("PerFragADS");
  // now we need to set the material and light values
  /*
   *struct MaterialInfo
   {
        // Ambient reflectivity
        vec3 Ka;
        // Diffuse reflectivity
        vec3 Kd;
        // Specular reflectivity
        vec3 Ks;
        // Specular shininess factor
        float shininess;
  };*/
  ngl::ShaderLib::setUniform("material.Ka", 0.1f, 0.1f, 0.1f);
  // red diffuse
  ngl::ShaderLib::setUniform("material.Kd", 0.8f, 0.8f, 0.8f);
  // white spec
  ngl::ShaderLib::setUniform("material.Ks", 1.0f, 1.0f, 1.0f);
  ngl::ShaderLib::setUniform("material.shininess", 1000.0f);
  // now for  the lights values (all set to white)
  /*struct LightInfo
  {
  // Light position in eye coords.
  vec4 position;
  // Ambient light intensity
  vec3 La;
  // Diffuse light intensity
  vec3 Ld;
  // Specular light intensity
  vec3 Ls;
  };*/
  ngl::ShaderLib::setUniform("light.position", ngl::Vec3(2, 20, 2));
  ngl::ShaderLib::setUniform("light.La", 0.1f, 0.1f, 0.1f);
  ngl::ShaderLib::setUniform("light.Ld", 1.0f, 1.0f, 1.0f);
  ngl::ShaderLib::setUniform("light.Ls", 0.9f, 0.9f, 0.9f);

  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces

  // as re-size is not explicitly called we need to do this.
  glViewport(0, 0, width(), height());
  m_text = std::make_unique<ngl::Text>("fonts/Arial.ttf", 16);
  m_text->setScreenSize(width(), height());
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("PerFragADS");
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  MV = m_view * m_mouseGlobalTX;
  MVP = m_project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("MV", MV);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
  ngl::ShaderLib::setUniform("weight1", m_weight1);
  ngl::ShaderLib::setUniform("weight2", m_weight2);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Rotation based on the mouse position for our global transform
  ngl::Transformation trans;
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  loadMatricesToShader();
  // draw the mesh
  m_vaoMesh->bind();
  m_vaoMesh->draw();
  m_vaoMesh->unbind();
  m_text->setColour(1.0f, 1.0f, 1.0f);

  m_text->renderText(10, 700, fmt::format("Q-W change Pose one weight {:0.2f}", m_weight1));
  m_text->renderText(10, 680, fmt::format("A-S change Pose one weight {:0.2f}", m_weight2));
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
  case Qt::Key_Q:
    changeWeight(Weights::POSE1, Direction::DOWN);
    break;
  case Qt::Key_W:
    changeWeight(Weights::POSE1, Direction::UP);
    break;

  case Qt::Key_A:
    changeWeight(Weights::POSE2, Direction::DOWN);
    break;
  case Qt::Key_S:
    changeWeight(Weights::POSE2, Direction::UP);
    break;
  case Qt::Key_Space:
    toggleAnimation();
    break;
  case Qt::Key_Z:
    punchLeft();
    break;
  case Qt::Key_X:
    punchRight();
    break;

  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}

void NGLScene::updateLeft()
{
  static Direction left = Direction::UP;
  if (left == Direction::UP)
  {
    m_weight1 += 0.2f;
    if (m_weight1 > 1.1f)
      left = Direction::DOWN;
  }
  else if (left == Direction::DOWN)
  {
    m_weight1 -= 0.2f;
    if (m_weight1 <= 0.0f)
    {
      m_weight1 = 0.0f;

      m_timerLeft->stop();
      left = Direction::UP;
      m_punchLeft = false;
    }
  }
  update();
}

void NGLScene::updateRight()
{
  static Direction right = Direction::UP;
  if (right == Direction::UP)
  {
    m_weight2 += 0.2f;
    if (m_weight2 > 1.1f)
      right = Direction::DOWN;
  }
  else if (right == Direction::DOWN)
  {
    m_weight2 -= 0.2f;
    if (m_weight2 <= 0.0f)
    {
      m_weight2 = 0.0f;
      m_timerRight->stop();
      right = Direction::UP;
      m_punchRight = false;
    }
  }
  update();
}
