#pragma once
#include "Vertex.h"
#include "Face.h"
#include <glm/glm.hpp>
#include "smartpointerhelp.h"
#include "HalfEdge.h"

// get next edge
HalfEdge* HalfEdge::getNext() {
    return this->next;
}

// get sym edge
HalfEdge* HalfEdge::getSym() {
    return this->sym;
}

// get edge vertex
Vertex* HalfEdge::getVert() {
    return this->vert;
}

// get face
Face* HalfEdge::getFace() {
    return this->edgeFace;
}

// get edge ID
int HalfEdge::getEdgeID() {
    return this->edgeInt;
}

// set next pointer
void HalfEdge::setNext(HalfEdge* e) {
    this->next = e;
}

// set sym edge
void HalfEdge::setSym(HalfEdge* e) {
    e->sym = this;
    this->sym = e;
}

// set vertex
void HalfEdge::setVert(Vertex* v) {
    this->vert = v;
}

// set face
void HalfEdge::setFace(Face* f) {
    this->edgeFace = f;
}

// set edge id
void HalfEdge::setEdgeID(int id) {
    this->edgeInt = id;
    QListWidgetItem::setText("Edge ID: " + id);
}

// constructor
HalfEdge::HalfEdge(HalfEdge* next, HalfEdge* sym, Vertex* v, Face* edgeF, int ID):
next(next), sym(sym), vert(v), edgeFace(edgeF), edgeInt(ID)
{
    QListWidgetItem::setText("Edge ID: " + ID);
}

// constructor
HalfEdge::HalfEdge(const HalfEdge &h):
    next(h.next), sym(h.sym), vert(h.vert), edgeFace(h.edgeFace), edgeInt(h.edgeInt)
{
}

// empty constructor
HalfEdge::HalfEdge():
    next(nullptr), sym(nullptr), vert(nullptr), edgeFace(nullptr), edgeInt(0)
{
}


