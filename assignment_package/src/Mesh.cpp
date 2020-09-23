#include "Mesh.h"
#include <glm/glm.hpp>
#include <iostream>
#include <QFile>

// constructor (unused)
Mesh::Mesh(OpenGLContext* m_context,
     std::vector<HalfEdge> edges,
     std::vector<Face> faces,
     std::vector<Vertex> verts):
    Drawable(m_context), cont(m_context)
{
    for (unsigned int i = 0; i < edges.size(); i++) {
        HalfEdge h = edges[i];
        edgeList.push_back(mkU<HalfEdge>(h));
    }

    for (unsigned int i = 0; i < faces.size(); i++) {
        Face f = faces[i];
        faceList.push_back(mkU<Face>(f));
    }

    for (unsigned int i = 0; i < verts.size(); i++) {
        Vertex v = verts[i];
        vertList.push_back(mkU<Vertex>(v));
    }
}

void Mesh::clearData() {
    destroy();
    edgeList.clear();
    faceList.clear();
    vertList.clear();
}

// constructor
Mesh::Mesh(OpenGLContext* context):
Drawable(context), cont(context)
{
}



// iterates through faces, creates VBOs
void Mesh::setFace(Face* f, std::vector<glm::vec4> &positionVec,
                    std::vector<GLuint> &indexVec,
                    std::vector<glm::vec4> &colorVec,
                    std::vector<glm::vec4> &normVec,
                   std::vector<glm::ivec2> &influence,
                   std::vector<glm::vec2> &weight) {

    glm::vec3 startPos = f->getFaceEdge()->getVert()->getVertPos();
    HalfEdge* current = f->getFaceEdge();
    int start = current->getEdgeID();
    unsigned int localIndex = 0;

    std::vector<Vertex*> vertexList;

    glm::vec3 norm;

    HalfEdge* normcurrent = f->getFaceEdge();
    Vertex* next2 = normcurrent->getNext()->getVert();
    Vertex* next3 = normcurrent->getNext()->getNext()->getVert();

    glm::vec3 cross1 = startPos - next3->getVertPos();
    glm::vec3 cross2 = next3->getVertPos() - next2->getVertPos();

    // calculate normals
    if (isCube) {
        norm = glm::cross(cross1, cross2);
    } else {
        norm = -1.f * glm::cross(cross1, cross2);
    }
    int startID = normcurrent->getEdgeID();
    int currID = -1;

    while (norm == glm::vec3(0, 0, 0)) {
        normcurrent = normcurrent->getNext();
        currID = normcurrent->getEdgeID();
        if (currID == startID) {
            break;
        }
        next2 = normcurrent->getNext()->getVert();
        next3 = normcurrent->getNext()->getNext()->getVert();

        glm::vec3 cross1 = startPos - next3->getVertPos();
        glm::vec3 cross2 = next3->getVertPos() - next2->getVertPos();
        norm = -1.f * glm::normalize(glm::cross(cross1, cross2));
    }
    f->surfaceNormal = norm;

    do {
        positionVec.push_back(glm::vec4(current->getVert()->getVertPos(), 1.0f));
        vertexList.push_back(current->getVert());

        colorVec.push_back(glm::vec4(f->getColor(), 1.0f));
        current = current->getNext();
        localIndex++;
    } while (current->getEdgeID() != start);

    for (unsigned int i = 0; i < localIndex - 2; i++) {
        indexVec.push_back(currentIndex);
        indexVec.push_back(currentIndex + i + 1);
        indexVec.push_back(currentIndex + i + 2);
    }

    currentIndex += localIndex;

    for (unsigned int i = 0; i < localIndex; i++) {
        normVec.push_back(glm::vec4(norm[0], norm[1], norm[2], 0.f));
    }

    for (unsigned int i = 0; i < localIndex - 2; i++) {
        influence.push_back(vertexList[i]->influences);
        influence.push_back(vertexList[i + 1]->influences);
        influence.push_back(vertexList[i + 2]->influences);
    }

    for (unsigned int i = 0; i < localIndex - 2; i++) {
        weight.push_back(vertexList[i]->weights);
        weight.push_back(vertexList[i + 1]->weights);
        weight.push_back(vertexList[i + 2]->weights);
    }

}

void Mesh::create() {
    std::vector<glm::vec4> pos1;
    std::vector<glm::vec4> nor1;
    std::vector<glm::vec4> col1;
    std::vector<GLuint> idx1;

    std::vector<glm::ivec2> inf1;
    std::vector<glm::vec2> weight1;

    // create vbos
    for (unsigned int i = 0; i < faceList.size(); i++) {
        setFace(faceList[i].get(), pos1, idx1, col1, nor1, inf1, weight1);
    }
    count = idx1.size();

    // bind buffers
    generateIdx();
    cont->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    cont->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx1.size() * sizeof(GLuint), idx1.data(), GL_STATIC_DRAW);

    generatePos();
    cont->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    cont->glBufferData(GL_ARRAY_BUFFER, pos1.size() * sizeof(glm::vec4), pos1.data(), GL_STATIC_DRAW);

    generateNor();
    cont->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    cont->glBufferData(GL_ARRAY_BUFFER, nor1.size() * sizeof(glm::vec4), nor1.data(), GL_STATIC_DRAW);

    generateWeights();
    cont->glBindBuffer(GL_ARRAY_BUFFER, bufWeight);
    cont->glBufferData(GL_ARRAY_BUFFER, weight1.size() * sizeof(glm::vec2), weight1.data(), GL_STATIC_DRAW);

    generateInfluences();
    cont->glBindBuffer(GL_ARRAY_BUFFER, bufInfluence);
    cont->glBufferData(GL_ARRAY_BUFFER, inf1.size() * sizeof(glm::ivec2), inf1.data(), GL_STATIC_DRAW);

    generateCol();
    cont->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    cont->glBufferData(GL_ARRAY_BUFFER, col1.size() * sizeof(glm::vec4), col1.data(), GL_STATIC_DRAW);
}


/*
 * Function takes in raw obj file data and assembles the entire mesh
 */
void Mesh::assemble() {
    std::vector<Vertex*> newVerts;
    std::vector<HalfEdge*> newEdges;
    std::vector<Face*> newFaces;

    std::map<HalfEdge*, std::pair<int, int>> edgeToSym;
    std::map<std::pair<int, int>, HalfEdge*> symPairs;

    // create the new vertices
    for (unsigned int i = 0; i < rawPos.size(); i++) {
        uPtr<Vertex> newVert = mkU<Vertex>(nullptr, vertList.size(), rawPos[i]);
        newVerts.push_back(newVert.get());
        vertList.push_back(std::move(newVert));
    }

    // create new half edges
    for (unsigned int i = 0; i < posIndex.size(); i++) {
        std::vector<glm::vec2> indices = posIndex[i];
        std::vector<HalfEdge*> faceEdges;

        for (unsigned int j = 0; j < indices.size(); j++) {
            int vertIndex = indices[j].x;
            uPtr<HalfEdge> newEdge = mkU<HalfEdge>(nullptr, nullptr, newVerts[vertIndex], nullptr, edgeList.size());
            newEdges.push_back(newEdge.get());
            faceEdges.push_back(newEdge.get());
            edgeList.push_back(std::move(newEdge));
        }

        float redVal = (float) rand() / (float)(RAND_MAX/1.0f);
        float blueVal = (float) rand() / (float)(RAND_MAX/1.0f);
        float greenVal = (float) rand() / (float)(RAND_MAX/1.0f);
        glm::vec3 color = glm::vec3(redVal, blueVal, greenVal);

        // create new faces
        uPtr<Face> newFace = mkU<Face>(faceEdges[0], faceList.size(), color);
        newFaces.push_back(newFace.get());

        // set pointers
        for (unsigned int k = 0; k < faceEdges.size() - 1; k++) {
            if (k == 0) {
                symPairs[std::make_pair(indices[indices.size() - 1].x, indices[k].x)] = faceEdges[k];
                edgeToSym[faceEdges[k]] = std::make_pair(indices[indices.size() - 1].x, indices[k].x);

            } else {
                symPairs[std::make_pair(indices[k - 1].x, indices[k].x)] = faceEdges[k];
                edgeToSym[faceEdges[k]] = std::make_pair(indices[k - 1].x, indices[k].x);
            }
            vertList[indices[k].x]->setVertEdge(faceEdges[k]);

            faceEdges[k]->setFace(newFace.get());
            faceEdges[k]->setNext(faceEdges[k + 1]);
        }

        faceEdges[faceEdges.size() - 1]->setNext(faceEdges[0]);
        faceEdges[faceEdges.size() - 1]->setFace(newFace.get());
        symPairs[std::make_pair(indices[indices.size() - 2].x, indices[indices.size() - 1].x)] = faceEdges[faceEdges.size() - 1];
        edgeToSym[faceEdges[faceEdges.size() - 1]] = std::make_pair(indices[indices.size() - 2].x, indices[indices.size() - 1].x);
        faceList.push_back(std::move(newFace));
    }

    // set sym pointers
    for (unsigned int i = 0; i < newEdges.size(); i++) {
        HalfEdge* he = newEdges[i];
        if (edgeToSym.count(he) > 0) {
            std::pair<int, int> currPair = edgeToSym[he];
            std::pair<int, int> switched = std::make_pair(currPair.second, currPair.first);
            if (symPairs.count(switched) > 0) {
                he->setSym(symPairs[switched]);
            } else {
                std::cout << "Pair Error" << std::endl;
            }
        } else {
            std::cout<< "Sym Error" << std::endl;
        }
    }
    std::cout << "Ending Assembly" << std::endl;

}

// destructor
void Mesh::destroy()
{
    currentIndex = 0;
    count = 0;
}

// destructor
Mesh::~Mesh()
{
    destroy();
}



// creates the actual cube
void Mesh::createCube() {
    uPtr<Vertex> v1u = mkU<Vertex>(Vertex(nullptr, 0, glm::vec3(-0.5f, -0.5f, -0.5f)));
    uPtr<Vertex> v2u = mkU<Vertex>(Vertex(nullptr, 1, glm::vec3(0.5f, -0.5f, -0.5f)));
    uPtr<Vertex> v3u = mkU<Vertex>(Vertex(nullptr, 2, glm::vec3(0.5f, 0.5f, -0.5f)));
    uPtr<Vertex> v4u = mkU<Vertex>(Vertex(nullptr, 3, glm::vec3(-0.5f, 0.5f, -0.5f)));

    uPtr<Vertex> v5u = mkU<Vertex>(Vertex(nullptr, 4, glm::vec3(-0.5f, -0.5f, 0.5f)));
    uPtr<Vertex> v6u = mkU<Vertex>(Vertex(nullptr, 5, glm::vec3(0.5f, -0.5f, 0.5f)));
    uPtr<Vertex> v7u = mkU<Vertex>(Vertex(nullptr, 6, glm::vec3(0.5f, 0.5f, 0.5f)));
    uPtr<Vertex> v8u = mkU<Vertex>(Vertex(nullptr, 7, glm::vec3(-0.5f, 0.5f, 0.5f)));


    uPtr<Face> f1u = mkU<Face>(Face(nullptr, 0, glm::vec3(1, 0, 0)));
    uPtr<Face> f2u = mkU<Face>(Face(nullptr, 1, glm::vec3(0, 1, 0)));
    uPtr<Face> f3u = mkU<Face>(Face(nullptr, 2, glm::vec3(0, 0, 1)));
    uPtr<Face> f4u = mkU<Face>(Face(nullptr, 3, glm::vec3(1, 0, 1)));
    uPtr<Face> f5u = mkU<Face>(Face(nullptr, 4, glm::vec3(1, 1, 0)));
    uPtr<Face> f6u = mkU<Face>(Face(nullptr, 5, glm::vec3(0, 1, 1)));

    uPtr<HalfEdge> h1u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v2u.get(), f1u.get(), 0));
    uPtr<HalfEdge> h2u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v3u.get(), f1u.get(), 1));
    uPtr<HalfEdge> h3u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v4u.get(), f1u.get(), 2));
    uPtr<HalfEdge> h4u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v1u.get(), f1u.get(), 3));


    h1u.get()->setNext(h2u.get());
    h2u.get()->setNext(h3u.get());
    h3u.get()->setNext(h4u.get());
    h4u.get()->setNext(h1u.get());


    v1u.get()->setVertEdge(h4u.get());
    v4u.get()->setVertEdge(h3u.get());
    v3u.get()->setVertEdge(h2u.get());
    v2u.get()->setVertEdge(h1u.get());

    f1u.get()->setFaceEdge(h1u.get());
    f1u->setColor(glm::vec3(1.f, 0, 0));

    // start face 2
    uPtr<HalfEdge> h5u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v6u.get(), f2u.get(), 4));
    uPtr<HalfEdge> h6u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v7u.get(), f2u.get(), 5));
    uPtr<HalfEdge> h7u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v3u.get(), f2u.get(), 6));
    uPtr<HalfEdge> h8u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v2u.get(), f2u.get(), 7));

    h5u.get()->setNext(h6u.get());
    h6u.get()->setNext(h7u.get());
    h7u.get()->setNext(h8u.get());
    h8u.get()->setNext(h5u.get());


    f2u.get()->setFaceEdge(h5u.get());
    f2u->setColor(glm::vec3(0.f, 0, 0));

    // start face 3

    uPtr<HalfEdge> h9u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v5u.get(), f3u.get(), 8));
    uPtr<HalfEdge> h10u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v8u.get(), f3u.get(), 9));
    uPtr<HalfEdge> h11u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v7u.get(), f3u.get(), 10));
    uPtr<HalfEdge> h12u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v6u.get(), f3u.get(), 11));

    h9u.get()->setNext(h10u.get());
    h10u.get()->setNext(h11u.get());
    h11u.get()->setNext(h12u.get());
    h12u.get()->setNext(h9u.get());

    v5u.get()->setVertEdge(h9u.get());
    v8u.get()->setVertEdge(h10u.get());
    v7u.get()->setVertEdge(h11u.get());
    v6u.get()->setVertEdge(h12u.get());


    f3u.get()->setFaceEdge(h9u.get());
    f3u->setColor(glm::vec3(0.f, 1.0f, 0));

    // do face 4

    uPtr<HalfEdge> h13u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v1u.get(), f4u.get(), 12));
    uPtr<HalfEdge> h14u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v4u.get(), f4u.get(), 13));
    uPtr<HalfEdge> h15u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v8u.get(), f4u.get(), 14));
    uPtr<HalfEdge> h16u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v5u.get(), f4u.get(), 15));

    h13u.get()->setNext(h14u.get());
    h14u.get()->setNext(h15u.get());
    h15u.get()->setNext(h16u.get());
    h16u.get()->setNext(h13u.get());

    f4u.get()->setFaceEdge(h13u.get());
    f4u->setColor(glm::vec3(0.f, 0, 1.0f));

    // do face 5

    uPtr<HalfEdge> h17u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v3u.get(), f5u.get(), 16));
    uPtr<HalfEdge> h18u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v7u.get(), f5u.get(), 17));
    uPtr<HalfEdge> h19u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v8u.get(), f5u.get(), 18));
    uPtr<HalfEdge> h20u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v4u.get(), f5u.get(), 19));

    h17u.get()->setNext(h18u.get());
    h18u.get()->setNext(h19u.get());
    h19u.get()->setNext(h20u.get());
    h20u.get()->setNext(h17u.get());

    f5u.get()->setFaceEdge(h17u.get());
    f5u->setColor(glm::vec3(1, 0, 1));

    // do face 6
    uPtr<HalfEdge> h21u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v1u.get(), f6u.get(), 20));
    uPtr<HalfEdge> h22u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v2u.get(), f6u.get(), 21));
    uPtr<HalfEdge> h23u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v6u.get(), f6u.get(), 22));
    uPtr<HalfEdge> h24u = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, v5u.get(), f6u.get(), 23));

    h21u.get()->setNext(h24u.get());
    h22u.get()->setNext(h21u.get());
    h23u.get()->setNext(h22u.get());
    h24u.get()->setNext(h23u.get());

    f6u.get()->setFaceEdge(h21u.get());
    f6u->setColor(glm::vec3(1.f, 1.0f, 0));

    // set sym edges;
    h1u.get()->setSym(h21u.get());
    h2u.get()->setSym(h8u.get());
    h3u.get()->setSym(h17u.get());
    h4u.get()->setSym(h14u.get());

    h5u.get()->setSym(h22u.get());
    h6u.get()->setSym(h12u.get());
    h7u.get()->setSym(h18u.get());

    h13u.get()->setSym(h24u.get());
    h16u.get()->setSym(h10u.get());
    h15u.get()->setSym(h20u.get());

    h11u.get()->setSym(h19u.get());
    h9u.get()->setSym(h23u.get());

    edgeList.push_back(std::move(h1u));
    edgeList.push_back(std::move(h2u));
    edgeList.push_back(std::move(h3u));
    edgeList.push_back(std::move(h4u));
    edgeList.push_back(std::move(h5u));
    edgeList.push_back(std::move(h6u));
    edgeList.push_back(std::move(h7u));
    edgeList.push_back(std::move(h8u));
    edgeList.push_back(std::move(h9u));
    edgeList.push_back(std::move(h10u));
    edgeList.push_back(std::move(h11u));
    edgeList.push_back(std::move(h12u));
    edgeList.push_back(std::move(h13u));
    edgeList.push_back(std::move(h14u));
    edgeList.push_back(std::move(h15u));
    edgeList.push_back(std::move(h16u));
    edgeList.push_back(std::move(h17u));
    edgeList.push_back(std::move(h18u));
    edgeList.push_back(std::move(h19u));
    edgeList.push_back(std::move(h20u));
    edgeList.push_back(std::move(h21u));
    edgeList.push_back(std::move(h22u));
    edgeList.push_back(std::move(h23u));
    edgeList.push_back(std::move(h24u));


    vertList.push_back(std::move(v1u));
    vertList.push_back(std::move(v2u));
    vertList.push_back(std::move(v3u));
    vertList.push_back(std::move(v4u));
    vertList.push_back(std::move(v5u));
    vertList.push_back(std::move(v6u));
    vertList.push_back(std::move(v7u));
    vertList.push_back(std::move(v8u));

    faceList.push_back(std::move(f1u));
    faceList.push_back(std::move(f2u));
    faceList.push_back(std::move(f3u));
    faceList.push_back(std::move(f4u));
    faceList.push_back(std::move(f5u));
    faceList.push_back(std::move(f6u));
}


