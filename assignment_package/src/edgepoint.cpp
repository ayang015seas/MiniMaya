#include "edgepoint.h"
#include <iostream>

/*
 * EdgePoint is the class that highlights the edge in GUI
 */

/*
 * Constructor
 */
EdgePoint::EdgePoint(OpenGLContext* context, HalfEdge* e):
    Drawable(context), representedEdge(e)
{
}

/*
 * Change the current edge
 */
void EdgePoint::updateEdge(HalfEdge *e) {
    this->representedEdge = e;
}

/*
 * Create in order to populate VBOs
 */
void EdgePoint::create() {
    // we need to push the two vertex points into the buffers
    glm::vec3 currPos = representedEdge->getVert()->getVertPos();
    HalfEdge* curr = representedEdge;
    glm::vec3 target;

    count = 2;

    std::vector<glm::vec4> pos1;
    std::vector<glm::vec4> nor1;
    std::vector<glm::vec4> col1;
    std::vector<GLuint> idx1;

    idx1.push_back(0);
    idx1.push_back(1);

    col1.push_back(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    col1.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

    HalfEdge* sym = curr->getSym();
    target = sym->getVert()->getVertPos();

    pos1.push_back(glm::vec4(currPos, 1.0f));
    pos1.push_back(glm::vec4(target, 1.0f));

    // bind all buffers
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

/*
 * Drawmode set to Lines
 */
GLenum EdgePoint::drawMode() {
    return GL_LINES;
}

/*
 * Destructor
 */
EdgePoint::~EdgePoint()
{
}

/*
 * Another constructor
 */
EdgePoint::EdgePoint(OpenGLContext* context):
    Drawable(context)
{
}
