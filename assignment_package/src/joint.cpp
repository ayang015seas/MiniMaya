#include "joint.h"
#include <iostream>
#include <QString>

Joint::Joint(OpenGLContext* context):
    Drawable(context), name(""), parent(nullptr), position(glm::vec4()), rotation(glm::vec4()), bindMatrix(glm::mat4())
{
}

Joint::Joint(OpenGLContext* context, QString name, Joint* par, glm::vec4 pos, glm::vec4 rot, glm::mat4 bindMat):
    Drawable(context), name(name), parent(par), position(pos), rotation(rot), bindMatrix(bindMat)
{
}

glm::mat4 Joint::getLocalTransformation() {
    glm::quat rotationQuat;
    rotationQuat = glm::angleAxis(rotation.x, glm::vec3(rotation.y, rotation.z, rotation.w));

    glm::quat rotQuat = glm::angleAxis(rotation.x, glm::vec3(rotation.y, rotation.z, rotation.w));
    glm::quat xRotate = glm::angleAxis(xRotation, glm::vec3(1.f, 0.f, 0.f));
    glm::quat yRotate = glm::angleAxis(yRotation, glm::vec3(0.f, 1.f, 0.f));
    glm::quat zRotate = glm::angleAxis(zRotation, glm::vec3(0.f, 0.f, 1.f));

    glm::quat finalQuat = rotQuat * xRotate * yRotate * zRotate;
    glm::mat4 rotationMatrix = glm::toMat4(finalQuat);
    glm::mat4 translateMat = glm::translate(glm::mat4(1.f), glm::vec3(position));

    return rotationMatrix * translateMat;
}

glm::mat4 Joint::getOverallTransformation() {
    Joint* currParent = this->parent;
    glm::mat4 localTrans = this->getLocalTransformation();
    while (currParent != nullptr) {
        localTrans = currParent->getLocalTransformation() * localTrans;
        currParent = currParent->parent;
    }
    if (!computedBind) {
        this->originalBind = glm::inverse(localTrans);
        computedBind = true;
    }
    return localTrans;
}

void Joint::calculatePosition() {
    Joint* par = this->parent;

    if (par == nullptr) {
        par = this;
    } else {
        // make sure that the parent is the absolute origin point
        while (par->parent != nullptr) {
            par = par->parent;
        }
    }
    // glm::vec4 originalPos = par->position + glm::vec4(3.0f, 0.f, 0.f, 0.f);
    glm::vec4 originalPos = glm::vec4(0.f, 0.f, 0.f, 1.f);
    actualPos = glm::vec3(getOverallTransformation() * originalPos);
}

void Joint::create() {
    count = 26;

    std::vector<glm::vec4> pos1;
    std::vector<glm::vec4> nor1;
    std::vector<glm::vec4> col1;
    std::vector<GLuint> idx1;

    glm::vec4 red = glm::vec4(1.f, 0.f, 0.f, 1.f);
    glm::vec4 green = glm::vec4(0.f, 1.f, 0.f, 1.f);
    glm::vec4 blue = glm::vec4(0.f, 0.f, 1.f, 1.f);

    glm::vec3 ind0 = this->actualPos + glm::vec3(0.f, 0.f, 0.5f);
    glm::vec3 ind1 = this->actualPos + glm::vec3(0.5f, 0.f, 0.0f);
    glm::vec3 ind2 = this->actualPos - glm::vec3(0.0f, 0.f, 0.5f);
    glm::vec3 ind3 = this->actualPos - glm::vec3(0.5f, 0.f, 0.0f);

    glm::vec3 ind4 = this->actualPos + glm::vec3(0.0f, 0.5f, 0.0f);
    glm::vec3 ind5 = this->actualPos - glm::vec3(0.0f, 0.5f, 0.0f);

    idx1.push_back(0);
    idx1.push_back(1);

    col1.push_back(red);
    col1.push_back(red);

    pos1.push_back(glm::vec4(ind0, 1.0f));
    pos1.push_back(glm::vec4(ind1, 1.0f));

    idx1.push_back(2);
    idx1.push_back(3);

    col1.push_back(red);
    col1.push_back(red);

    pos1.push_back(glm::vec4(ind1, 1.0f));
    pos1.push_back(glm::vec4(ind2, 1.0f));


    idx1.push_back(4);
    idx1.push_back(5);

    col1.push_back(red);
    col1.push_back(red);

    pos1.push_back(glm::vec4(ind2, 1.0f));
    pos1.push_back(glm::vec4(ind3, 1.0f));

    idx1.push_back(6);
    idx1.push_back(7);

    col1.push_back(red);
    col1.push_back(red);

    pos1.push_back(glm::vec4(ind3, 1.0f));
    pos1.push_back(glm::vec4(ind0, 1.0f));

    // do y axis
    idx1.push_back(8);
    idx1.push_back(9);

    col1.push_back(green);
    col1.push_back(green);

    pos1.push_back(glm::vec4(ind2, 1.0f));
    pos1.push_back(glm::vec4(ind4, 1.0f));

    idx1.push_back(10);
    idx1.push_back(11);

    col1.push_back(green);
    col1.push_back(green);

    pos1.push_back(glm::vec4(ind4, 1.0f));
    pos1.push_back(glm::vec4(ind0, 1.0f));

    idx1.push_back(12);
    idx1.push_back(13);

    col1.push_back(green);
    col1.push_back(green);

    pos1.push_back(glm::vec4(ind0, 1.0f));
    pos1.push_back(glm::vec4(ind5, 1.0f));

    idx1.push_back(14);
    idx1.push_back(15);

    col1.push_back(green);
    col1.push_back(green);

    pos1.push_back(glm::vec4(ind5, 1.0f));
    pos1.push_back(glm::vec4(ind2, 1.0f));

    // do z axis
    idx1.push_back(16);
    idx1.push_back(17);

    col1.push_back(blue);
    col1.push_back(blue);

    pos1.push_back(glm::vec4(ind5, 1.0f));
    pos1.push_back(glm::vec4(ind1, 1.0f));

    idx1.push_back(18);
    idx1.push_back(19);

    col1.push_back(blue);
    col1.push_back(blue);

    pos1.push_back(glm::vec4(ind1, 1.0f));
    pos1.push_back(glm::vec4(ind4, 1.0f));

    idx1.push_back(20);
    idx1.push_back(21);

    col1.push_back(blue);
    col1.push_back(blue);

    pos1.push_back(glm::vec4(ind4, 1.0f));
    pos1.push_back(glm::vec4(ind3, 1.0f));

    idx1.push_back(22);
    idx1.push_back(23);

    col1.push_back(blue);
    col1.push_back(blue);

    pos1.push_back(glm::vec4(ind3, 1.0f));
    pos1.push_back(glm::vec4(ind5, 1.0f));


    std::vector<glm::vec4> col2;
    for (unsigned int i = 0; i < 24; i++) {
        col2.push_back(glm::vec4(1.f, 1.f, 1.f, 1.f));
    }

    if (this->parent == nullptr) {
        count = 24;
    } else {
        idx1.push_back(24);
        idx1.push_back(25);

        col1.push_back(glm::vec4(1.f, 0.f, 0.f, 1.f));
        col1.push_back(glm::vec4(1.f, 1.f, 0.f, 1.f));

        col2.push_back(glm::vec4(1.f, 1.f, 1.f, 1.f));
        col2.push_back(glm::vec4(1.f, 1.f, 1.f, 1.f));

        pos1.push_back(glm::vec4(this->parent->actualPos, 1.0f));
        pos1.push_back(glm::vec4(this->actualPos, 1.0f));
    }

    generateIdx();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx1.size() * sizeof(GLuint), idx1.data(), GL_STATIC_DRAW);

    generatePos();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos1.size() * sizeof(glm::vec4), pos1.data(), GL_STATIC_DRAW);

    generateNor();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    mp_context->glBufferData(GL_ARRAY_BUFFER, nor1.size() * sizeof(glm::vec4), nor1.data(), GL_STATIC_DRAW);

    if (isSelected) {
        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, col2.size() * sizeof(glm::vec4), col2.data(), GL_STATIC_DRAW);
    } else {
        generateCol();
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
        mp_context->glBufferData(GL_ARRAY_BUFFER, col1.size() * sizeof(glm::vec4), col1.data(), GL_STATIC_DRAW);
    }

}

GLenum Joint::drawMode() {
    return GL_LINES;
}






