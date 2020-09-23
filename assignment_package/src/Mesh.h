#pragma once

#include <glm/glm.hpp>
#include "HalfEdge.h"
#include "Vertex.h"
#include "Face.h"
#include "smartpointerhelp.h"
#include <vector>
#include <QListWidgetItem>
#include <QObject>
#include "drawable.h"
#include "joint.h"

// class represents the overall mesh with the cube

class Mesh : public QListWidgetItem, public Drawable {

    protected:
        unsigned int currentIndex = 0;
        OpenGLContext* cont;

    public:
        std::vector<uPtr<HalfEdge>> edgeList;
        std::vector<uPtr<Face>> faceList;
        std::vector<uPtr<Vertex>> vertList;
        std::vector<uPtr<Joint>> jointList;

        std::vector<glm::vec3> rawPos;
        std::vector<glm::vec3> rawNorm;
        std::vector<glm::vec3> rawFaces;
        bool isCube = true;

        std::vector<std::vector<glm::vec2>> posIndex;

        void create() override;
        void destroy();
        ~Mesh();
        Mesh(OpenGLContext* context, std::vector<HalfEdge> edges,
             std::vector<Face> faces,
             std::vector<Vertex> verts);
        Mesh(OpenGLContext* context);
        void createCube();
        void setFace(Face* f, std::vector<glm::vec4> &position,
                      std::vector<GLuint> &index,
                      std::vector<glm::vec4> &color,
                      std::vector<glm::vec4> &norm,
                     std::vector<glm::ivec2> &influence,
                     std::vector<glm::vec2> &weight);
        void assemble();
        void clearData();


};

