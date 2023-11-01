#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDir>

glm::vec3 sun = glm::vec3(0., 0., 0.);
const int sun_radius = 128;

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this), m_progSky(this),
      m_terrain(this), m_player(glm::vec3(48.f, 129.f, 48.f), m_terrain),
      m_planet(this, sun, sun_radius), m_quad(this),
      m_textureAlbedo(this), m_textureNormals(this),
      m_time(QDateTime::currentMSecsSinceEpoch()), last_time(QDateTime::currentMSecsSinceEpoch())
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}

QString MyGL::getCurrentPath() const {
    QString path = QDir::currentPath();
    path = path.left(path.lastIndexOf("/"));
#ifdef __APPLE__
    path = path.left(path.lastIndexOf("/"));
    path = path.left(path.lastIndexOf("/"));
    path = path.left(path.lastIndexOf("/"));
#endif
    return path;
}

void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glEnable(GL_CULL_FACE);
//    glFrontFace(GL_CW);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
//    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_progSky.create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");
    m_quad.createVBOdata();
    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    // create textures
    QString path1 = getCurrentPath();
    path1.append("/assignment_package/textures/minecraft_textures_all.png");
    QString path2 = getCurrentPath();
    path2.append("/assignment_package/textures/minecraft_normals_all.png");
    m_textureAlbedo.create(path1.toStdString().c_str());
    m_textureAlbedo.load(0);
    m_textureNormals.create(path2.toStdString().c_str());
    m_textureNormals.load(1);

//    m_terrain.CreateTestScene();
//    m_terrain.CreateNewScene();
    m_planet.createPlanet();
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(glm::inverse(viewproj));

    // Sky demo
    m_progSky.useMe();
    this->glUniform2i(m_progSky.unifDimensions, width(), height());
    this->glUniform3f(m_progSky.unifEye, m_player.mcr_position.x, m_player.mcr_position.y, m_player.mcr_position.z);

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    m_terrain.checkTerrain(m_player.mcr_position);


    int64_t currMSec = QDateTime::currentMSecsSinceEpoch();
    int64_t deltaTime = currMSec - last_time;
    last_time = currMSec;
    int time_passed = currMSec - m_time;
    m_progLambert.setTime(time_passed);

    // update the center of the sun
    time++;
    m_planet.move(time);
    m_progLambert.setSun(m_planet.center);
    m_progSky.setSun(m_planet.center);
    m_player.tick(deltaTime, m_inputs);
    m_progSky.setPlayer(m_player.mcr_position);
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    m_progFlat.setViewProjMatrix(viewproj);
    m_progLambert.setViewProjMatrix(viewproj);
    m_progInstanced.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(glm::inverse(viewproj));

    // Sky
    m_progSky.useMe();
    this->glUniform3f(m_progSky.unifEye, m_player.mcr_position.x, m_player.mcr_position.y, m_player.mcr_position.z);
    this->glUniform1f(m_progSky.unifTime, time);

    // bind textures
    m_textureAlbedo.bind(0);
    m_textureNormals.bind(1);

    m_progSky.drawSky(m_quad);
    renderTerrain();

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
//    m_progFlat.draw(m_worldAxes); // will cause error because i edited draw function
    m_progLambert.setModelMatrix(glm::mat4());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    m_terrain.draw(&m_progLambert, m_player.mcr_position);
    m_planet.draw(&m_progLambert);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_inputs.flightMode = !m_inputs.flightMode;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.shiftPressed = true;
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_Shift) {
        m_inputs.shiftPressed = false;
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // TODO
    QPoint currentPosition = e->pos();
    QPoint center(width() / 2, height() / 2);

    float sensitivity = 0.1f;
    float theta = static_cast<float>(currentPosition.x() - center.x());
    float phi = static_cast<float>(currentPosition.y() - center.y());

    theta *= sensitivity;
    phi *= sensitivity;
    TotalRotRight += phi;

    if (TotalRotRight > 90.f) {
        float diff = 90.f - TotalRotRight;
        phi += diff;
        TotalRotRight = 90.f;
    } else if (TotalRotRight < -90.f) {
        float diff = -90.f - TotalRotRight;
        phi += diff;
        TotalRotRight = -90.f;
    }

    m_inputs.mouseY = -phi;
    m_inputs.mouseX = -theta;

    m_player.rotateOnRightLocal(m_inputs.mouseY);
    m_player.rotateOnUpGlobal(m_inputs.mouseX);

    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // TODO
    if (e->button() == Qt::LeftButton) {
        m_player.removeBlock();
    } else if (e->button() == Qt::RightButton) {
        m_player.placeBlock();
    }
}
