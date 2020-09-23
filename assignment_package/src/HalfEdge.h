#include <QObject>
#include <QListWidgetItem>

#ifndef HALFEDGE_H
#define HALFEDGE_H

#include <glm/glm.hpp>
#include "smartpointerhelp.h"
#include "Vertex.h"
#include "Face.h"

class Face;
class Vertex;

// class to represent polygon edges
class HalfEdge: public QListWidgetItem {

protected:
    HalfEdge* next;
    HalfEdge* sym;
    Vertex* vert;
    Face* edgeFace;
    int edgeInt;
    QString name;

public:
    // getter functions
    HalfEdge* getNext();
    HalfEdge* getSym();
    Vertex* getVert();
    Face* getFace();
    int getEdgeID();

    // setter functions
    void setNext(HalfEdge* e);
    void setSym(HalfEdge* e);
    void setVert(Vertex* v);
    void setFace(Face* f);
    void setEdgeID(int id);

    // constructors
    HalfEdge(HalfEdge* next, HalfEdge* sym, Vertex* v, Face* edgeF, int ID);
    HalfEdge();
    HalfEdge(const HalfEdge &h);

};

#endif // HALFEDGE_H
