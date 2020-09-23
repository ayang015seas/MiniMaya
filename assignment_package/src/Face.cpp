#include "Face.h"

// set the face edge
void Face::setFaceEdge(HalfEdge* e) {
    this->faceEdge = e;
}
// set the face id
void Face::setFaceID(int ID) {
    this->faceID = ID;
    QListWidgetItem::setText("Face ID: " + ID);
}

// set the face color
void Face::setColor(glm::vec3 col) {
    this->faceColor = col;
}

// get the face edge
HalfEdge* Face::getFaceEdge() {
    return this->faceEdge;
}

// set the face id
int Face::getFaceID() {
    return this->faceID;
}

// set the face color
glm::vec3 Face::getColor() {
    return this->faceColor;
}

// construct the face
Face::Face(HalfEdge* e, int id, glm::vec3 color):
    faceEdge(e), faceColor(color), faceID(id) {
    QListWidgetItem::setText("Face ID: " + id);
}

// copy constructor
Face::Face(const Face &f2):
    faceEdge(f2.faceEdge), faceColor(f2.faceColor), faceID(f2.faceID)
{
}

// empty constructor
Face::Face():
    faceEdge(nullptr), faceColor(glm::vec3()), faceID(0)
{
}
