#include "Vertex.h"

// get vert edge
HalfEdge* Vertex::getVertEdge() {
    return this->vertEdge;
}

// get vert id
int Vertex::getVertID() {
    return this->vertID;
}

// get vert position
glm::vec3 Vertex::getVertPos() {
    return this->vertPos;
}

// set vert edge
void Vertex::setVertEdge(HalfEdge* edge) {
    this->vertEdge = edge;
}

// set vert id
void Vertex::setVertID(int ID) {
    this->vertID = ID;

    QString s = "Vertex ID: ";
    s.append(ID);
    this->name = s;
    QListWidgetItem::setText(this->name);
}

// set vert position
void Vertex::setVertPos(glm::vec3 pos) {
    this->vertPos = pos;
}

// constructor
Vertex::Vertex(const Vertex &v):
    vertPos(v.vertPos), vertID(v.vertID), vertEdge(v.vertEdge)
{
}

// constructor
Vertex::Vertex(HalfEdge* e, int id, glm::vec3 pos):
    vertPos(pos), vertID(id), vertEdge(e)
{
    QString s = "Vertex ID: ";
    s.append(id);
    this->name = s;
    QListWidgetItem::setText(this->name);
}

// constructor
Vertex::Vertex():
    vertPos(glm::vec3()), vertID(0), vertEdge(nullptr)
{
}

