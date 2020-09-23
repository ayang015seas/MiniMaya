#include "facepoint.h"
#include <iostream>

// constructor
FacePoint::FacePoint(OpenGLContext* context, Face* f):
    Drawable(context), representedFace(f)
{
}

// update the current face
void FacePoint::updateFace(Face *f) {
    this->representedFace = f;
}

// ring the face with edges
void FacePoint::create() {
    std::vector<glm::vec4> pos1;
    std::vector<glm::vec4> col1;
    std::vector<GLuint> idx1;
    std::vector<glm::vec4> nor1;

      glm::vec3 color = this->representedFace->getColor();
      glm::vec3 oppositeColor = 1.0f - color;

      HalfEdge* startEdge = representedFace->getFaceEdge();
      int startEdgeID = startEdge->getEdgeID();
      HalfEdge* currEdge = representedFace->getFaceEdge();
      int currEdgeID = -1;

      do {
          glm::vec3 position = currEdge->getVert()->getVertPos();
          pos1.push_back(glm::vec4(position, 1.0f));
          currEdge = currEdge->getNext();
          currEdgeID = currEdge->getEdgeID();
      } while (currEdgeID != startEdgeID);

      for (unsigned int i = 0; i < pos1.size() - 1; i++) {
          idx1.push_back(i);
          idx1.push_back(i + 1);
      }

      idx1.push_back(pos1.size() - 1);
      idx1.push_back(0);

      for (int i = 0; i < idx1.size(); i++) {
          col1.push_back(glm::vec4(oppositeColor, 1.0f));
      }

      count = idx1.size();

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx1.size() * sizeof(GLuint), idx1.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col1.size() * sizeof(glm::vec4), col1.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos1.size() * sizeof(glm::vec4), pos1.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor1.size() * sizeof(glm::vec4), nor1.data(), GL_STATIC_DRAW);
}

// return drawmode
GLenum FacePoint::drawMode() {
    return GL_LINES;
}

// destructor
FacePoint::~FacePoint()
{
}

// constructor
FacePoint::FacePoint(OpenGLContext* context):
    Drawable(context)
{
}
