#include "vertpoint.h"
#include <iostream>

// constructor
VertPoint::VertPoint(OpenGLContext* context, Vertex* v):
    Drawable(context), representedVertex(v), cont(context)
{
}

// update the vertex
void VertPoint::updateVertex(Vertex *v) {
    this->representedVertex = v;
    create();
}

// constructor
VertPoint::VertPoint():
Drawable(nullptr), representedVertex(nullptr), cont(nullptr)
{
}

// create by pushing in 1 point into VBOs
void VertPoint::create() {
    glm::vec3 currPos = representedVertex->getVertPos();

    count = 1;

    std::vector<glm::vec4> pos1;
    std::vector<glm::vec4> nor1;
    std::vector<glm::vec4> col1;
    std::vector<GLuint> idx1;

    idx1.push_back(0);

    col1.push_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    pos1.push_back(glm::vec4(currPos, 1.0f));

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx1.size() * sizeof(GLuint), idx1.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos1.size() * sizeof(glm::vec4), pos1.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor1.size() * sizeof(glm::vec4), nor1.data(), GL_STATIC_DRAW);

    generateCol();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col1.size() * sizeof(glm::vec4), col1.data(), GL_STATIC_DRAW);

}

// destructor
VertPoint::~VertPoint()
{
}

// draw mode returns points
GLenum VertPoint::drawMode() {
    return GL_POINTS;
}

// destructor
void VertPoint::destroy() {
    this->~VertPoint();
}

// constructor
VertPoint::VertPoint(OpenGLContext* context):
    Drawable(context), representedVertex(nullptr), cont(context)
{
}

