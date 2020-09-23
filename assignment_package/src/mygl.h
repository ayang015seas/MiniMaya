#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/squareplane.h>
#include "camera.h"
#include "Mesh.h"
#include "Vertex.h"
#include "Face.h"
#include "HalfEdge.h"
#include <QObject>
#include <QListWidgetItem>
#include "vertpoint.h"
#include "edgepoint.h"
#include "facepoint.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>


class MyGL
    : public OpenGLContext
{
    Q_OBJECT

signals:
    void vertToMain(QListWidgetItem* v);
    void edgeToMain(QListWidgetItem* e);
    void faceToMain(QListWidgetItem* f);
    void jointToMain(QTreeWidgetItem* j);

private:
    SquarePlane m_geomSquare;// The instance of a unit cylinder we can use to render any cylinder
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera m_glCamera;


public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    std::map<Face*, float> centroidX;
    std::map<Face*, float> centroidY;
    std::map<Face*, float> centroidZ;

    std::map<Face*, std::vector<HalfEdge*>> midPointMap;
    std::map<Vertex*, Vertex*> midVertMap;
    std::map<HalfEdge*, HalfEdge*> splitEdges;
    std::map<Vertex*, Vertex*> moddedVertMap;

    glm::mat4 bindTransformations[100];
    glm::mat4 overallTransformations[100];

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    ShaderProgram getLam();
    void drawPoint(VertPoint *v);
    void drawLine(EdgePoint e);
    Joint* loadQObject(QVariantMap target, Joint* parent);

    Mesh currentMesh;
    // pointer to currently selected object

    Vertex* vertDisplayPointer;
    Face* faceDisplayPointer;
    HalfEdge* edgeDisplayPointer;
    Joint* currentJoint = nullptr;

    std::vector<uPtr<HalfEdge>> tempEdgeList;
    std::vector<uPtr<Vertex>> tempVertList;

    void calcCentroids(Mesh *m);
    void createVert(HalfEdge* target, Mesh *m);
    void quad(Mesh* m, Face* target);
    void quadAll(Mesh *m);

    void extrude(Face* f, Mesh* m, float distance);
    void modifyOriginal(Vertex* v, HalfEdge* away, HalfEdge* toward);

    // these classes display the actual object
    VertPoint vertexDisplay;
    FacePoint faceDisplay;
    EdgePoint edgeDisplay;

    Joint* rootJoint = nullptr;

    void triangulateFaceHelper(Face* face, Vertex* v1, Vertex* v2, HalfEdge* startEdge);
    void addMidPoints(Mesh *m);

public slots:
    // slots to put the vertex in the list
    void sendVert(QListWidgetItem* v);
    void sendFace(QListWidgetItem* f);
    void sendEdge(QListWidgetItem* e);

    // button functions
    void quadrangulate();
    void extrudeSlot();
    void loadFile();
    void loadJSON();

    // slots for the triangulate and add vertex buttons
    void createVert();
    void triangulateFace();

    // slots to select an object
    void vertClick(QListWidgetItem* v);
    void edgeClick(QListWidgetItem* e);
    void faceClick(QListWidgetItem* f);

    // slots to change vertex positions
    void vertXVal(double val);
    void vertYVal(double val);
    void vertZVal(double val);

    // slots to change face values
    void changeRVal(double val);
    void changeGVal(double val);
    void changeBVal(double val);

    void jointClicked(QTreeWidgetItem* item, int col);

    void rotateJointX();
    void rotateJointY();
    void rotateJointZ();

    void rotateNegativeJointX();
    void rotateNegativeJointY();
    void rotateNegativeJointZ();

    void skinMesh();

protected:
    void keyPressEvent(QKeyEvent *e);
    int edgeCount = 0;
    int vertCount = 0;
    int faceCount = 0;
};


#endif // MYGL_H
