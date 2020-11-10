#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>


NGLScene::NGLScene()
{
  setTitle("VAOPrimitives Demo");
  m_animate=true;
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL( int _w, int _h )
{
  m_project=ngl::perspective(45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}
constexpr auto shaderProgram="PBR";
void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::initialize();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  m_lightAngle=0.0;
  m_lightPos.set(sinf(m_lightAngle),2,cosf(m_lightAngle));
  // now to load the shader and set the values
  // we are creating a shader called PBR to save typos
  // in the code create some constexpr
  constexpr auto vertexShader  = "PBRVertex";
  constexpr auto fragShader    = "PBRFragment";
  // create the shader program
  ngl::ShaderLib::createShaderProgram( shaderProgram );
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader( vertexShader, ngl::ShaderType::VERTEX );
  ngl::ShaderLib::attachShader( fragShader, ngl::ShaderType::FRAGMENT );
  // attach the source
  ngl::ShaderLib::loadShaderSource( vertexShader, "shaders/PBRVertex.glsl" );
  ngl::ShaderLib::loadShaderSource( fragShader, "shaders/PBRFragment.glsl" );
  // compile the shaders
  ngl::ShaderLib::compileShader( vertexShader );
  ngl::ShaderLib::compileShader( fragShader );
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram( shaderProgram, vertexShader );
  ngl::ShaderLib::attachShaderToProgram( shaderProgram, fragShader );
  // now we have associated that data we can link the shader
  ngl::ShaderLib::linkProgramObject( shaderProgram );
  // and make it active ready to load values
  ngl::ShaderLib::use(shaderProgram );
 // We now create our view matrix for a static camera
  ngl::Vec3 from( 0.0f, 2.0f, 10.0f );
  ngl::Vec3 to( 0.0f, 0.0f, 0.0f );
  ngl::Vec3 up( 0.0f, 1.0f, 0.0f );
  // now load to our new camera
  m_view=ngl::lookAt(from,to,up);
  ngl::ShaderLib::setUniform( "camPos", from );
  // now a light
  m_lightPos.set( 0.0, 2.0f, 2.0f ,1.0f);
  // setup the default shader material and light porerties
  // these are "uniform" so will retain their values
  ngl::ShaderLib::setUniform("lightPosition",m_lightPos.toVec3());
  ngl::ShaderLib::setUniform("lightColor",400.0f,400.0f,400.0f);
  ngl::ShaderLib::setUniform("exposure",2.2f);
  ngl::ShaderLib::setUniform("albedo",0.950f, 0.71f, 0.29f);

  ngl::ShaderLib::setUniform("metallic",1.02f);
  ngl::ShaderLib::setUniform("roughness",0.38f);
  ngl::ShaderLib::setUniform("ao",0.2f);

  ngl::VAOPrimitives::createSphere("sphere",0.5f,50);

  ngl::VAOPrimitives::createCylinder("cylinder",0.5f,1.4f,40,40);

  ngl::VAOPrimitives::createCone("cone",0.5,1.4f,20,20);

  ngl::VAOPrimitives::createDisk("disk",0.8f,120);
  ngl::VAOPrimitives::createTorus("torus",0.15f,0.4f,40,40);
  ngl::VAOPrimitives::createTrianglePlane("plane",14,14,80,80,ngl::Vec3(0,1,0));
  // as re-size is not explicitly called we need to do this.
  glViewport(0,0,width(),height());
  // this timer is going to trigger an event every 40ms which will be processed in the
  //
  m_lightTimer =startTimer(40);


}


void NGLScene::loadMatricesToShader()
{
    ngl::ShaderLib::use("PBR");
    struct transform
    {
      ngl::Mat4 MVP;
      ngl::Mat4 normalMatrix;
      ngl::Mat4 M;
    };

     transform t;
     t.M=m_view*m_mouseGlobalTX*m_transform.getMatrix();

     t.MVP=m_project*t.M;
     t.normalMatrix=t.M;
     t.normalMatrix.inverse().transpose();
     ngl::ShaderLib::setUniformBuffer("TransformUBO",sizeof(transform),&t.MVP.m_00);

     ngl::ShaderLib::setUniform("lightPosition",(m_mouseGlobalTX*m_lightPos).toVec3());

}

void NGLScene::drawScene(const std::string &_shader)
{
  ngl::ShaderLib::use(_shader);
  // clear the screen and depth buffer
  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_win.spinXFace);
  rotY.rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;


  m_transform.reset();
  {
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("teapot");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-3.0f,0.0f,0.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("sphere");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(3.0f,0.0f,0.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("cylinder");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(0.0f,0.0f,3.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("cube");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-3.0f,0.0f,3.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("torus");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(3.0f,0.5f,3.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("icosahedron");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(0.0f,0.0f,-3.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("cone");
  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(-3.0f,0.5f,-3.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("tetrahedron");
  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(3.0f,0.5f,-3.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("octahedron");
  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(0.0f,0.5f,-6.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("football");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-3.0f,0.5f,-6.0f);
    m_transform.setRotation(0.0f,180.0f,0.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("disk");
  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(3.0f,0.5f,-6.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("dodecahedron");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(1.0f,0.35f,1.0f);
    m_transform.setScale(1.5f,1.5f,1.5f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("troll");
  } // and before a pop

#ifdef ADDLARGEMODELS
  m_transform.reset();
  {
    m_transform.setPosition(-1.0,-0.5,1.0);
    m_transform.setScale(0.1f,0.1f,0.1f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("dragon");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-2.5f,-0.5f,1.0f);
    m_transform.setScale(0.1f,0.1f,0.1f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("buddah");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(2.5f,-0.5f,1.0f);
    m_transform.setScale(0.1f,0.1f,0.1f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("bunny");
  } // and before a pop
#endif

  m_transform.reset();
  {
    m_transform.setPosition(0.0f,-0.5f,0.0f);
    loadMatricesToShader();
    ngl::VAOPrimitives::draw("plane");
  } // and before a pop


}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
  drawScene("PBR");

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent( QMouseEvent* _event )
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if ( m_win.rotate && _event->buttons() == Qt::LeftButton )
  {
    int diffx = _event->x() - m_win.origX;
    int diffy = _event->y() - m_win.origY;
    m_win.spinXFace += static_cast<int>( 0.5f * diffy );
    m_win.spinYFace += static_cast<int>( 0.5f * diffx );
    m_win.origX = _event->x();
    m_win.origY = _event->y();
    update();
  }
  // right mouse translate code
  else if ( m_win.translate && _event->buttons() == Qt::RightButton )
  {
    int diffX      = static_cast<int>( _event->x() - m_win.origXPos );
    int diffY      = static_cast<int>( _event->y() - m_win.origYPos );
    m_win.origXPos = _event->x();
    m_win.origYPos = _event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent( QMouseEvent* _event )
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.origX  = _event->x();
    m_win.origY  = _event->y();
    m_win.rotate = true;
  }
  // right mouse translate mode
  else if ( _event->button() == Qt::RightButton )
  {
    m_win.origXPos  = _event->x();
    m_win.origYPos  = _event->y();
    m_win.translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent( QMouseEvent* _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if ( _event->button() == Qt::LeftButton )
  {
    m_win.rotate = false;
  }
  // right mouse translate mode
  if ( _event->button() == Qt::RightButton )
  {
    m_win.translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent( QWheelEvent* _event )
{

  // check the diff of the wheel position (0 means no change)
  if ( _event->delta() > 0 )
  {
    m_modelPos.m_z += ZOOM;
  }
  else if ( _event->delta() < 0 )
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case Qt::Key_Space : m_animate^=true; break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  if (isExposed())
    update();
}

void NGLScene::updateLight()
{

  // change the light angle
  m_lightAngle+=0.1f;

  // now set this value and load to the shader
  m_lightPos.set(ngl::Vec3(4.0*cosf(m_lightAngle),2.0f,4.0f*sinf(m_lightAngle)));
}

void NGLScene::timerEvent(QTimerEvent *_event )
{
// if the timer is the light timer call the update light method
  if(_event->timerId() == m_lightTimer && m_animate==true)
  {
    updateLight();
  }
    // re-draw GL
update();
}

