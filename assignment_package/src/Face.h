#include <QObject>
#include <QListWidgetItem>

#ifndef FACE_H
#define FACE_H

#include <glm/glm.hpp>
#include "HalfEdge.h"

// this class represents polygon faces

class HalfEdge;
class Vertex;

class Face: public QListWidgetItem {

    protected:
        HalfEdge* faceEdge;
        glm::vec3 faceColor;
        int faceID;
    public:
        // getter and setter function
        glm::vec3 surfaceNormal;
        void setFaceEdge(HalfEdge* e);
        void setFaceID(int ID);
        void setColor(glm::vec3 col);

        // constructors
        Face(HalfEdge* e, int id, glm::vec3 color);
        Face();

        // getter functions
        HalfEdge* getFaceEdge();
        int getFaceID();
        glm::vec3 getColor();
        Face(const Face &f2);

};

#endif // FACE_H

