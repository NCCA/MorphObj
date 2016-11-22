#ifndef NGLSCENE_H_
#define NGLSCENE_H_
#include <ngl/AbstractVAO.h>
#include <ngl/Camera.h>
#include <ngl/Obj.h>
#include <ngl/Text.h>
#include "WindowParams.h"
#include <QTimer>
#include <QOpenGLWindow>
#include <memory>

//----------------------------------------------------------------------------------------------------------------------
/// @file NGLScene.h
/// @brief this class inherits from the Qt OpenGLWindow and allows us to use NGL to draw OpenGL
/// @author Jonathan Macey
/// @version 1.0
/// @date 10/9/13
/// Revision History :
/// This is an initial version used for the new NGL6 / Qt 5 demos
/// @class NGLScene
/// @brief our main glwindow widget for NGL applications all drawing elements are
/// put in this file
//----------------------------------------------------------------------------------------------------------------------

class NGLScene : public QOpenGLWindow
{
  Q_OBJECT
  public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief ctor for our NGL drawing class
    /// @param [in] parent the parent window to the class
    //----------------------------------------------------------------------------------------------------------------------
    NGLScene();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief dtor must close down ngl and release OpenGL resources
    //----------------------------------------------------------------------------------------------------------------------
    ~NGLScene();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the initialize class is called once when the window is created and we have a valid GL context
    /// use this to setup any default GL stuff
    //----------------------------------------------------------------------------------------------------------------------
    void initializeGL();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we want to draw the scene
    //----------------------------------------------------------------------------------------------------------------------
    void paintGL();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we resize
    //----------------------------------------------------------------------------------------------------------------------
    // Qt 5.5.1 must have this implemented and uses it
    void resizeGL(QResizeEvent *_event);
    // Qt 5.x uses this instead! http://doc.qt.io/qt-5/qopenglwindow.html#resizeGL
    void resizeGL(int _w, int _h);

private:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the windows params such as mouse and rotations etc
    //----------------------------------------------------------------------------------------------------------------------
    WinParams m_win;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief used to store the global mouse transforms
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Mat4 m_mouseGlobalTX;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Our Camera
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Camera m_cam;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the model position for mouse movement
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Vec3 m_modelPos;
    enum class Weights{POSE1,POSE2};
    enum class Direction{UP,DOWN};
    void changeWeight(Weights _w,Direction _d );

    inline void toggleAnimation(){m_animation^=true;}
    void punchLeft();
    void punchRight();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our model
    //----------------------------------------------------------------------------------------------------------------------
    std::vector< std::unique_ptr<ngl::Obj >> m_meshes;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief text for rendering
    //----------------------------------------------------------------------------------------------------------------------
    std::unique_ptr<ngl::Text> m_text;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the weight of pose one
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Real m_weight1;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the weight of pose two
    //----------------------------------------------------------------------------------------------------------------------
    ngl::Real m_weight2;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the mesh with all the data in it
    //----------------------------------------------------------------------------------------------------------------------
    std::unique_ptr<ngl::AbstractVAO> m_vaoMesh;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief left animation timer
    //----------------------------------------------------------------------------------------------------------------------
    QTimer *m_timerLeft;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief right animation timer
    //----------------------------------------------------------------------------------------------------------------------
    QTimer *m_timerRight;
    //----------------------------------------------------------------------------------------------------------------------
    /// animation flag for timers
    //----------------------------------------------------------------------------------------------------------------------
    bool m_animation;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief flag to indicate if punching left
    //----------------------------------------------------------------------------------------------------------------------
    bool m_punchLeft;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief flag to indicate if punching right
    //----------------------------------------------------------------------------------------------------------------------
    bool m_punchRight;
    //----------------------------------------------------------------------------------------------------------------------
    /// do our morphing for the 3 meshes
    //----------------------------------------------------------------------------------------------------------------------
    void createMorphMesh();

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief method to load transform matrices to the shader
    //----------------------------------------------------------------------------------------------------------------------
    void loadMatricesToShader();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Qt Event called when a key is pressed
    /// @param [in] _event the Qt event to query for size etc
    //----------------------------------------------------------------------------------------------------------------------
    void keyPressEvent(QKeyEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called every time a mouse is moved
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mouseMoveEvent (QMouseEvent * _event );
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse button is pressed
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mousePressEvent ( QMouseEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse button is released
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void mouseReleaseEvent ( QMouseEvent *_event );

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this method is called everytime the mouse wheel is moved
    /// inherited from QObject and overridden here.
    /// @param _event the Qt Event structure
    //----------------------------------------------------------------------------------------------------------------------
    void wheelEvent( QWheelEvent *_event);
  private slots :
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the timers are connected to slots to trigger the events
    //----------------------------------------------------------------------------------------------------------------------
    void updateLeft();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the timers are connected to slots to trigger the events
    //----------------------------------------------------------------------------------------------------------------------
    void updateRight();


};



#endif
