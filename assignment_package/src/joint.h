
#ifndef JOINT_H
#define JOINT_H


#include <glm/glm.hpp>
#include "smartpointerhelp.h"
#include "Vertex.h"
#include "Face.h"
#include<glm/gtc/quaternion.hpp>
#include "drawable.h"
#include<glm/common.hpp>
#include <glm/gtx/quaternion.hpp>
#include <openglcontext.h>
#include <QTreeWidgetItem>
#include "glm/gtc/matrix_transform.hpp"

class Joint : public QTreeWidgetItem, public Drawable {
public:
    QString name;
    Joint* parent;
    bool isSelected = false;
    std::vector<Joint*> children;
    glm::vec4 position;
    glm::vec4 rotation;
    glm::mat4 bindMatrix;
    glm::vec3 actualPos;
    float xRotation = 0.f;
    float yRotation = 0.f;
    float zRotation = 0.f;
    glm::mat4 originalBind;
    bool computedBind = false;

    Joint(OpenGLContext* context);
    void create() override;
    Joint(OpenGLContext* context, QString name, Joint* par, glm::vec4 pos, glm::vec4 rot, glm::mat4 bindMat);
    GLenum drawMode() override;

    glm::mat4 getLocalTransformation();
    glm::mat4 getOverallTransformation();
    void calculatePosition();


};

#endif // JOINT_H
