#pragma once
#include <QObject>
#include <QListWidgetItem>

#ifndef VERTEX_H
#define VERTEX_H
#include <glm/glm.hpp>
#include "HalfEdge.h"
#include <map>
#include "joint.h"

class HalfEdge;
class Face;

class Joint;
// class to represent vertices
class Vertex: public QListWidgetItem {
    protected:
        glm::vec3 vertPos;
        int vertID;
        HalfEdge* vertEdge;
        QString name;


    public:
        HalfEdge* getVertEdge();
        std::map<Joint*, float> associatedJoints;
        std::map<Joint*, int> vertIDMap;
        glm::ivec2 influences = glm::ivec2(0, 0);
        glm::vec2 weights = glm::vec2(0, 0);

        int getVertID();
        glm::vec3 getVertPos();

        void setVertEdge(HalfEdge* edge);
        void setVertID(int ID);
        void setVertPos(glm::vec3 pos);
        Vertex(HalfEdge* e, int id, glm::vec3 pos);
        Vertex();
        Vertex(const Vertex &v);
};


#endif // VERTEX_H
