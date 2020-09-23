#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QObject>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTreeWidgetItem>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_geomSquare(this),
      m_progLambert(this), m_progFlat(this),
      m_glCamera(), currentMesh(this), vertexDisplay(this),
      faceDisplay(FacePoint(this)), edgeDisplay(EdgePoint(this))
{
    this->vertDisplayPointer = nullptr;
    this->faceDisplayPointer = nullptr;
    this->edgeDisplayPointer = nullptr;

    setFocusPolicy(Qt::StrongFocus);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_geomSquare.destroy();
}

ShaderProgram MyGL::getLam() {
    return this->m_progFlat;
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.5, 0.5, 0.5, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instances of Cylinder and Sphere.
    m_geomSquare.create();
    currentMesh.createCube();
    currentMesh.create();


    for (unsigned int i = 0; i < currentMesh.vertList.size(); i++) {
        emit vertToMain(currentMesh.vertList[i].get());
        vertCount++;
    }

    for (unsigned int i = 0; i < currentMesh.faceList.size(); i++) {
        emit faceToMain(currentMesh.faceList[i].get());
        faceCount++;
    }

    for (unsigned int i = 0; i < currentMesh.edgeList.size(); i++) {
        emit edgeToMain(currentMesh.edgeList[i].get());
        edgeCount++;
    }

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");


    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_glCamera = Camera(w, h);
    glm::mat4 viewproj = m_glCamera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);

    printGLErrorLog();
}

Joint* MyGL::loadQObject(QVariantMap target, Joint* parent) {
    QVariantList childList = target[QString("children")].toList();
    QVariantList posList = target[QString("pos")].toList();
    QVariantList rotList = target[QString("rot")].toList();
    QString name = target[QString("name")].toString();

    qDebug() << name;
    std::vector<float> posVec;
    std::vector<float> rotVec;

    for (QVariantList::const_iterator iter = posList.begin(); iter != posList.end(); ++iter) {
        QString curr = iter->toString();
        posVec.push_back(curr.toFloat());
    }

    for (QVariantList::const_iterator iter = rotList.begin(); iter != rotList.end(); ++iter) {
        QString curr = iter->toString();
        rotVec.push_back(curr.toFloat());
    }

    glm::vec4 rotation = glm::vec4(rotVec[0], rotVec[1], rotVec[2], rotVec[3]);
    glm::vec4 position = glm::vec4(posVec[0], posVec[1], posVec[2], 1.0f);

    uPtr<Joint> newJoint = mkU<Joint>(this, name, parent, position, rotation, glm::mat4(1.0f));

    // set the root joint
    if (parent == nullptr) {
        rootJoint = newJoint.get();
    }

    newJoint.get()->setText(0, name);

    for (QVariantList::const_iterator iter = childList.begin(); iter != childList.end(); ++iter) {
        QVariantMap m = iter->toMap();
        Joint* child = loadQObject(m, newJoint.get());
        newJoint.get()->addChild(child);
        newJoint.get()->children.push_back(child);
    }
    Joint* jPointer = newJoint.get();
    if (newJoint == nullptr) {
        std::cout << "Loaded Null Joint" << std::endl;
    }
    currentMesh.jointList.push_back(std::move(newJoint));
    return jPointer;
}


void MyGL::loadJSON() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Files (*.json)"));

    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    } else {
        return;
    }

    QString fileName = fileNames[0];
    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly) && !fileName.isNull()) {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        }
//        currentMesh.jointList = std::vector<uPtr<Joint>>();
//        std::cout << "CLEARED" << std::endl;

        QString doc;
        doc = file.readAll();

        QJsonDocument tempDoc = QJsonDocument::fromJson(doc.toUtf8());
        QJsonObject tree = tempDoc.object();
        QJsonValue root = tree.value(QString("root"));
        QJsonObject rootObj = root.toObject();
        emit jointToMain(loadQObject(rootObj.toVariantMap(), nullptr));

        std::cout << "Read JSON" << std::endl;
        file.close();
        update();
    }
}

void MyGL::jointClicked(QTreeWidgetItem* item, int col) {
    if (currentJoint != nullptr) {
        currentJoint->isSelected = false;
    }
    std::cout << "Joint Selected" << std::endl;
    Joint* newCurrentJoint = dynamic_cast<Joint*>(item);
    this->currentJoint = newCurrentJoint;
    newCurrentJoint->isSelected = true;
    update();
}


// loads in the qfile
void MyGL::loadFile() {
    vertCount = 0;
    edgeCount = 0;
    faceCount = 0;

    vertDisplayPointer = nullptr;
    edgeDisplayPointer = nullptr;
    faceDisplayPointer = nullptr;

    currentMesh.rawPos.clear();
    currentMesh.rawNorm.clear();
    currentMesh.rawFaces.clear();
    currentMesh.posIndex.clear();

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Files (*.obj)"));

    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;

    if (dialog.exec()) {
        fileNames = dialog.selectedFiles();
    } else {
        return;
    }

    QString fileName = fileNames[0];

    if (fileName.isEmpty() || fileName.isNull())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        } else {
            std::cout << "Successfully opened file" << std::endl;
            while (!file.atEnd()) {
                QString line = file.readLine().trimmed();
                QStringList splitLine = line.split(QRegularExpression(" "));
                std::string stdLine = line.toStdString();
                if (stdLine.length() == 0) {
                    continue;
                }

                if (stdLine.at(0) == 'v') {
                    if (stdLine.at(1) == 'n') {
                        std::string xPos = splitLine[1].toStdString();
                        std::string yPos = splitLine[2].toStdString();
                        std::string zPos = splitLine[3].toStdString();
                        float xNorm = std::stof (xPos);
                        float yNorm = std::stof (yPos);
                        float zNorm = std::stof (zPos);
                        currentMesh.rawNorm.push_back(glm::vec3(xNorm, yNorm, zNorm));

                    } else if (stdLine.at(1) == 't') {
                    } else {
                        std::string xPos = splitLine[1].toStdString();
                        std::string yPos = splitLine[2].toStdString();
                        std::string zPos = splitLine[3].toStdString();
                        float xCoord = std::stof (xPos);
                        float yCoord = std::stof (yPos);
                        float zCoord = std::stof (zPos);
                        currentMesh.rawPos.push_back(glm::vec3(xCoord, yCoord, zCoord));
                    }
                } else if (stdLine.at(0) == 'f') {
                    std::vector<glm::vec2> faceIndexes;
                    for (int i = 1; i < splitLine.size(); i++) {
                        QStringList indices = splitLine[i].split(QRegularExpression("/"));
                        int v1 = std::stoi(indices[0].toStdString());
                        int v2 = std::stoi(indices[2].toStdString());
                        faceIndexes.push_back(glm::vec2(v1 - 1, v2 - 1));
                    }
                    currentMesh.posIndex.push_back(faceIndexes);
                }
            }
            file.close();
            std::cout << "File Load Complete" << std::endl;
        }
    }

    // clear data and use obj file to assemble new mesh
    currentMesh.isCube = false;
    currentMesh.clearData();
    currentMesh.assemble();

    // send new vertices to GUI
    for (unsigned int i = 0; i < currentMesh.vertList.size(); i++) {
        emit vertToMain(currentMesh.vertList[i].get());
        vertCount++;
    }

    for (unsigned int i = 0; i < currentMesh.faceList.size(); i++) {
        emit faceToMain(currentMesh.faceList[i].get());
        faceCount++;
    }

    for (unsigned int i = 0; i < currentMesh.edgeList.size(); i++) {
        emit edgeToMain(currentMesh.edgeList[i].get());
        edgeCount++;
    }

    // draw using paint
    currentMesh.create();
    update();
}

// slot to extrude the face
void MyGL::extrudeSlot() {
    currentMesh.destroy();
    if (faceDisplayPointer != nullptr) {
        extrude(faceDisplayPointer, &currentMesh, 1.5);
    }
    currentMesh.create();
    if (vertDisplayPointer != nullptr) {
        sendVert(vertDisplayPointer);
    }
    if (edgeDisplayPointer != nullptr) {
        sendEdge(edgeDisplayPointer);
    }
    if (faceDisplayPointer != nullptr) {
        sendFace(faceDisplayPointer);
    }
    update();
}

// helper function to print out vec3
void printOut(glm::vec3 position) {
    std::cout << "(" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

// helper function to print out vec3
void MyGL::extrude(Face* f, Mesh *m, float distance) {
    std::vector<HalfEdge*> topEdges;
    std::vector<HalfEdge*> bottomEdges;
    std::vector<HalfEdge*> s1Edges;
    std::vector<HalfEdge*> s2Edges;
    std::vector<HalfEdge*> origSyms;
    std::vector<HalfEdge*> leftSyms;

    std::vector<Face*> newFaces;
    std::vector<Vertex*> newVerts;
    HalfEdge* currEdge = f->getFaceEdge();
    HalfEdge* startEdge = f->getFaceEdge();

    // create all elements of 1 face
    do {
        HalfEdge* origSym = currEdge->getSym();
        uPtr<Vertex> currVert = mkU<Vertex>(nullptr, vertCount, currEdge->getVert()->getVertPos());
        vertCount++;

        uPtr<HalfEdge> he1 = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;

        uPtr<HalfEdge> he2 = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;

        uPtr<HalfEdge> he3 = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;

        uPtr<HalfEdge> he4 = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;

        uPtr<HalfEdge> replacementSym = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;

        origSyms.push_back(origSym);
        HalfEdge* prevEdge = origSym;
        do {
            prevEdge = prevEdge->getNext();
        } while (prevEdge->getNext() != origSym);


        uPtr<Face> f1 = mkU<Face>(he1.get(), faceCount, f->getColor());
        faceCount++;

        // set vertex positions
        glm::vec3 newVertPos = currEdge->getVert()->getVertPos() + (f->surfaceNormal * distance);
        currEdge->getVert()->setVertPos(newVertPos);

        he1->setFace(f1.get());
        he2->setFace(f1.get());
        he3->setFace(f1.get());
        he4->setFace(f1.get());

        he1.get()->setNext(he2.get());
        he2.get()->setNext(he3.get());
        he3.get()->setNext(he4.get());
        he4.get()->setNext(he1.get());

        he1.get()->setVert(currVert.get());
        he3.get()->setVert(currEdge->getSym()->getVert());
        he2.get()->setVert(currEdge->getVert());

        HalfEdge* startAdditionalSym = prevEdge->getSym();
        HalfEdge* additionalSymRight = prevEdge->getSym();

        do {
            additionalSymRight = additionalSymRight->getNext();
        } while (additionalSymRight->getNext() != startAdditionalSym);

        if (additionalSymRight != currEdge->getNext()->getSym()) {
            additionalSymRight->setVert(currVert.get());
        }

        currVert->setVertEdge(he1.get());

        he3.get()->setSym(currEdge);
        he1.get()->setSym(origSym);


        HalfEdge* additionalSymLeft = origSym;
        additionalSymLeft = additionalSymLeft->getNext()->getSym();
        leftSyms.push_back(additionalSymLeft);
        prevEdge->setVert(currVert.get());

        topEdges.push_back(he3.get());
        bottomEdges.push_back(he1.get());
        s1Edges.push_back(he4.get());
        s2Edges.push_back(he2.get());

        newVerts.push_back(currVert.get());

        emit faceToMain(f1.get());
        emit vertToMain(currVert.get());

        emit edgeToMain(he1.get());
        emit edgeToMain(he2.get());
        emit edgeToMain(he3.get());
        emit edgeToMain(he4.get());

        m->edgeList.push_back(std::move(he1));
        m->edgeList.push_back(std::move(he2));
        m->edgeList.push_back(std::move(he3));
        m->edgeList.push_back(std::move(he4));
        m->vertList.push_back(std::move(currVert));
        m->faceList.push_back(std::move(f1));

        currEdge = currEdge->getNext();
    } while (currEdge != startEdge);

    // set all pointers
    for (unsigned int i = 1; i < newVerts.size(); i++) {
        s1Edges[i]->setVert(newVerts[i - 1]);
        origSyms[i]->setVert(newVerts[i - 1]);

        s1Edges[i]->setSym(s2Edges[i - 1]);
        leftSyms[i]->setVert(newVerts[i - 1]);
    }

    s1Edges[0]->setSym(s2Edges[newVerts.size() - 1]);
    s1Edges[0]->setVert(newVerts[newVerts.size() - 1]);
    origSyms[0]->setVert(newVerts[newVerts.size() - 1]);
    leftSyms[0]->setVert(newVerts[newVerts.size() - 1]);

}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function update() is called, paintGL is called implicitly.
void MyGL::paintGL()
{

    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setViewProjMatrix(m_glCamera.getViewProj());
    m_progLambert.setCamPos(m_glCamera.eye);
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    //Create a model matrix. This one rotates the square by PI/4 radians then translates it by <-2,0,0>.
    //Note that we have to transpose the model matrix before passing it to the shader
    //This is because OpenGL expects column-major matrices, but you've
    //implemented row-major matrices.
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-2,0,0)) * glm::rotate(glm::mat4(), 0.25f * 3.14159f, glm::vec3(0,1,0));
    //Send the geometry's transformation matrix to the shader
    m_progLambert.setModelMatrix(glm::mat4(1.f));
    m_progFlat.setModelMatrix(glm::mat4(1.f));

    int matrixNum = 0;
    if (currentMesh.jointList.size() > 0) {
        // std::cout << "SIZE " << currentMesh.jointList.size() << std::endl;
        for (unsigned int i = 0; i < currentMesh.jointList.size(); i++) {
            matrixNum++;
            bindTransformations[i] = currentMesh.jointList[i]->originalBind;
            overallTransformations[i] = currentMesh.jointList[i]->getOverallTransformation();
        }
    }


    m_progLambert.setBindMatrix(bindTransformations, matrixNum);
    m_progLambert.setOverallTransformation(overallTransformations, matrixNum);

    //Draw the example sphere using our lambert shader
    currentMesh.destroy();
    currentMesh.create();

     m_progLambert.draw(currentMesh);

    if (vertDisplayPointer != nullptr) {
        m_progFlat.draw(vertexDisplay);
        glEnable(GL_DEPTH_TEST);
    }
    if (edgeDisplayPointer != nullptr) {
        edgeDisplay.create();
        m_progFlat.draw(edgeDisplay);
        glEnable(GL_DEPTH_TEST);
    }

    if (faceDisplayPointer != nullptr) {
        faceDisplay.create();
        m_progFlat.draw(faceDisplay);
        glEnable(GL_DEPTH_TEST);
    }

    if (currentMesh.jointList.size() != 0) {
        glDisable(GL_DEPTH_TEST);

        for (int i = 0; i < currentMesh.jointList.size(); i++) {
            Joint* j = currentMesh.jointList[i].get();
            j->calculatePosition();
        }

        for (int i = 0; i < currentMesh.jointList.size(); i++) {
            Joint* j = currentMesh.jointList[i].get();
            j->create();
            m_progFlat.draw(*j);
        }

        glEnable(GL_DEPTH_TEST);
    }

    glEnable(GL_DEPTH_TEST);
}

// update the vertex display
void MyGL::sendVert(QListWidgetItem* v) {
    Vertex *vert = dynamic_cast<Vertex*>(v);
    this->vertDisplayPointer = vert;
    this->vertexDisplay.updateVertex(vert);

    if (edgeDisplayPointer != nullptr) {
        this->edgeDisplay.create();
    }

    if (faceDisplayPointer != nullptr) {
        this->faceDisplay.create();
    }

    glDisable(GL_DEPTH_TEST);
    update();
}

// quadrangulate high level function
void MyGL::quadrangulate() {
    centroidX.clear();
    centroidY.clear();
    centroidZ.clear();

    midPointMap.clear();
    midVertMap.clear();
    splitEdges.clear();

    moddedVertMap.clear();

    calcCentroids(&currentMesh);
    addMidPoints(&currentMesh);
    quadAll(&currentMesh);

    if (vertDisplayPointer != nullptr) {
        sendVert(vertDisplayPointer);
    }
    if (edgeDisplayPointer != nullptr) {
        sendEdge(edgeDisplayPointer);
    }
    if (faceDisplayPointer != nullptr) {
        sendFace(faceDisplayPointer);
    }
    update();
}

// set face to be highlighted
void MyGL::sendFace(QListWidgetItem* f) {
    Face *fa = dynamic_cast<Face*>(f);
    this->faceDisplayPointer = fa;
    this->faceDisplay.updateFace(fa);

    if (vertDisplayPointer != nullptr) {
        this->vertexDisplay.create();
    }

    if (edgeDisplayPointer != nullptr) {
        this->edgeDisplay.create();
    }

    glDisable(GL_DEPTH_TEST);
    update();
}

// set edge to be highlighted
void MyGL::sendEdge(QListWidgetItem* e) {
    HalfEdge *he = dynamic_cast<HalfEdge*>(e);
    this->edgeDisplayPointer = he;
    this->edgeDisplay.updateEdge(he);

    if (vertDisplayPointer != nullptr) {
        this->vertexDisplay.create();
    }

    if (faceDisplayPointer != nullptr) {
        this->faceDisplay.create();
    }

    glDisable(GL_DEPTH_TEST);
    update();
}

// draw designated point
void MyGL::drawPoint(VertPoint *v) {
    VertPoint v1 = VertPoint(this);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-2,0,0)) * glm::rotate(glm::mat4(), 0.25f * 3.14159f, glm::vec3(0,1,0));
    m_progFlat.setModelMatrix(model);
    m_progFlat.draw(*v);
    glDisable(GL_DEPTH_TEST);
    update();
}

// default normal half edge split function
void MyGL::createVert() {
    if (edgeDisplayPointer != nullptr) {
        HalfEdge* curr = edgeDisplayPointer;
        HalfEdge* sym = curr->getSym();

        Vertex* currVert = curr->getVert();
        Vertex* symVert = sym->getVert();

        uPtr<HalfEdge> currNext = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;
        uPtr<HalfEdge> symNext = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;

        currNext.get()->setVert(curr->getVert());
        symNext.get()->setVert(sym->getVert());

        currNext.get()->setNext(curr->getNext());
        symNext.get()->setNext(sym->getNext());

        currNext.get()->setFace(curr->getFace());
        symNext.get()->setFace(sym->getFace());

        glm::vec3 newPos = (currVert->getVertPos() + symVert->getVertPos()) * 0.50f;
        vertCount++;

        uPtr<Vertex> newVert = mkU<Vertex>(nullptr, vertCount++, newPos);
        newVert.get()->setVertEdge(curr);

        curr->setVert(newVert.get());
        sym->setVert(newVert.get());

        curr->setNext(currNext.get());
        sym->setNext(symNext.get());

        currNext.get()->setSym(sym);
        symNext.get()->setSym(curr);

        emit vertToMain(newVert.get());
        emit edgeToMain(currNext.get());
        emit edgeToMain(symNext.get());

        currentMesh.edgeList.push_back(std::move(currNext));
        currentMesh.edgeList.push_back(std::move(symNext));
        currentMesh.vertList.push_back(std::move(newVert));

        update();
    }
}

// helper function to triangulate a single face
void MyGL::triangulateFaceHelper(Face* face, Vertex* firstVert, Vertex* secondVert, HalfEdge* startEdge) {
    uPtr<HalfEdge> HE_A = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, nullptr, nullptr, edgeCount));
    edgeCount++;
    uPtr<HalfEdge> HE_B = mkU<HalfEdge>(HalfEdge(nullptr, nullptr, nullptr, nullptr, edgeCount));
    edgeCount++;
    uPtr<Face> faceNew = mkU<Face>(Face(nullptr, faceCount, face->getColor()));
    faceCount++;

    HE_A.get()->setVert(firstVert);
    HE_B.get()->setVert(secondVert);
    HE_A.get()->setSym(HE_B.get());
    HE_A.get()->setFace(faceNew.get());
    HE_B.get()->setFace(face);

    startEdge->getNext()->setFace(faceNew.get());
    startEdge->getNext()->getNext()->setFace(faceNew.get());

    faceNew.get()->setFaceEdge(startEdge->getNext());

    HE_B.get()->setNext(startEdge->getNext()->getNext()->getNext());

    startEdge->getNext()->getNext()->setNext(HE_A.get());
    HE_A.get()->setNext(startEdge->getNext());
    startEdge->setNext(HE_B.get());

    emit faceToMain(faceNew.get());
    emit edgeToMain(HE_A.get());
    emit edgeToMain(HE_B.get());

    currentMesh.edgeList.push_back(std::move(HE_A));
    currentMesh.edgeList.push_back(std::move(HE_B));
    currentMesh.faceList.push_back(std::move(faceNew));

}

// function to quadranglate all faces
void MyGL::quadAll(Mesh* m) {
    std::vector<Face*> currentFaces;
    for (unsigned int i = 0; i < m->faceList.size(); i++) {
        currentFaces.push_back(m->faceList[i].get());
    }

    for (unsigned int i = 0; i < currentFaces.size(); i++) {
        quad(m, currentFaces[i]);
    }
    m->destroy();
    m->create();
    std::cout << "Completed Quad" << std::endl;
}

// function to quadranglate a single face
void MyGL::quad(Mesh* m, Face* target) {
    std::vector<HalfEdge*> n = midPointMap[target];
    glm::vec3 position = glm::vec3(centroidX[target], centroidY[target], centroidZ[target]);
    uPtr<Vertex> centroidVert = mkU<Vertex>(nullptr, vertCount, position);
    vertCount++;

    if (n.size() == 0) {
        std::cout << "Quad Error" << std::endl;
    } else {
        std::vector<HalfEdge*> toCenter;
        std::vector<HalfEdge*> fromCenter;
        std::vector<Face*> newFaces;

        unsigned int pointCount = 0;
        HalfEdge* start = target->getFaceEdge();
        int startID = start->getEdgeID();
        int currID = -1;
        newFaces.push_back(target);

        do {
            Vertex* point = start->getVert();
            if (midVertMap.count(point) > 0) {
                uPtr<HalfEdge> toCentroid = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
                edgeCount++;
                uPtr<HalfEdge> symCentroid = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
                edgeCount++;

                uPtr<Face> newFace = mkU<Face>(toCentroid.get(), faceCount, target->getColor());

                toCentroid.get()->setVert(centroidVert.get());
                symCentroid.get()->setVert(start->getVert());

                modifyOriginal(start->getSym()->getVert(), start, start->getSym());
                toCentroid.get()->setSym(symCentroid.get());

                symCentroid.get()->setNext(start->getNext());
                start->setNext(toCentroid.get());

                toCenter.push_back(toCentroid.get());
                fromCenter.push_back(symCentroid.get());

                if (pointCount != 0) {
                    newFaces.push_back(newFace.get());
                    emit faceToMain(newFace.get());
                    m->faceList.push_back(std::move(newFace));
                    faceCount++;
                }

                start = symCentroid.get();
                emit edgeToMain(toCentroid.get());
                emit edgeToMain(symCentroid.get());

                m->edgeList.push_back(std::move(toCentroid));
                m->edgeList.push_back(std::move(symCentroid));
                pointCount++;
            }

            start = start->getNext();
            currID = start->getEdgeID();
        } while(startID != currID);

        toCenter[0]->setNext(fromCenter[pointCount - 1]);
        toCenter[0]->setFace(target);
        fromCenter[pointCount - 1]->setFace(target);

        for (unsigned int i = 1; i < pointCount; i++) {
            toCenter[i]->setNext(fromCenter[i - 1]);
            HalfEdge* curr = toCenter[i];
            int startID2 = toCenter[i]->getEdgeID();
            int currID2 = -1;
            do {
                newFaces[i]->setFaceEdge(curr);
                if (newFaces[i] == nullptr) {
                    std::cout << "GOT NULL" << std::endl;
                }
                curr->setFace(newFaces[i]);
                curr = curr->getNext();
                currID2 = curr->getEdgeID();
            } while(currID2 != startID2);
        }

        emit vertToMain(centroidVert.get());
        m->vertList.push_back(std::move(centroidVert));
    }
}

// function to push in original vertices
void MyGL::modifyOriginal(Vertex *v, HalfEdge *away, HalfEdge *toward) {
    if (moddedVertMap.count(v)) {
        return;
    } else {
        moddedVertMap[v] = v;
    }

    float centCount = 0;
    float xCPos = 0.f;
    float yCPos = 0.f;
    float zCPos = 0.f;

    float xMidPos = 0.f;
    float yMidPos = 0.f;
    float zMidPos = 0.f;

    float midCount = 0;

    Face* startFace = away->getFace();
    Face* currFace = nullptr;
    HalfEdge* currEdge = away;

    do {
        currFace = currEdge->getFace();
        xCPos += centroidX[currFace];
        yCPos += centroidY[currFace];
        zCPos += centroidZ[currFace];

        currEdge = currEdge->getSym();
        currEdge = currEdge->getNext();
        currFace = currEdge->getFace();

        xMidPos += currEdge->getVert()->getVertPos().x;
        yMidPos += currEdge->getVert()->getVertPos().y;
        zMidPos += currEdge->getVert()->getVertPos().z;
        centCount++;
        midCount++;
    } while (currFace != startFace);

    glm::vec3 sumMids = glm::vec3(xMidPos, yMidPos, zMidPos);
    glm::vec3 sumCents = glm::vec3(xCPos, yCPos, zCPos);


    glm::vec3 finalPos = (((midCount - 2.0f) / midCount) * (v->getVertPos())) +
            (glm::vec3(xMidPos, yMidPos, zMidPos) / (midCount * midCount)) +
            (glm::vec3(xCPos, yCPos, zCPos) / (midCount * midCount));

    v->setVertPos(finalPos);
}

// function to triangulate all faces
void MyGL::triangulateFace() {
    std::vector<Vertex*> vertList;
    unsigned int vertCount = 0;
    if (faceDisplayPointer != nullptr) {
        HalfEdge* startEdge = faceDisplayPointer->getFaceEdge();

        HalfEdge* currentEdge = faceDisplayPointer->getFaceEdge();
        int startEdgeID = currentEdge->getEdgeID();
        Face* currFace = currentEdge->getFace();

        do {
            vertList.push_back(currentEdge->getVert());
            currentEdge = currentEdge->getNext();
            vertCount++;
        } while(currentEdge->getEdgeID() != startEdgeID);

        for (unsigned int i = 0; i < vertCount - 3; i++) {
            triangulateFaceHelper(currFace, vertList[0], vertList[i + 2], startEdge);
        }
        update();
    }
}

// function calculates all centroids, stores them in map
void MyGL::calcCentroids(Mesh *m) {
    for (unsigned int i = 0; i < m->faceList.size(); i++) {
        Face* f = m->faceList[i].get();
        float xTotal = 0.f;
        float yTotal = 0.f;
        float zTotal = 0.f;
        float totalVerts = 0.f;
        HalfEdge* currentEdge = f->getFaceEdge();
        int startID = currentEdge->getEdgeID();
        do {
            Vertex* currVert = currentEdge->getVert();
            xTotal += currVert->getVertPos().x;
            yTotal += currVert->getVertPos().y;
            zTotal += currVert->getVertPos().z;
            totalVerts++;
            currentEdge = currentEdge->getNext();
        } while (currentEdge->getEdgeID() != startID);

        centroidX[f] = xTotal/totalVerts;
        centroidY[f] = yTotal/totalVerts;
        centroidZ[f] = zTotal/totalVerts;
    }
}

// function to create catmull clark split edges
void MyGL::createVert(HalfEdge* target, Mesh *m) {

    if (target != nullptr) {
        HalfEdge* curr = target;
        HalfEdge* sym = curr->getSym();

        Vertex* currVert = curr->getVert();
        Vertex* symVert = sym->getVert();

        uPtr<HalfEdge> currNext = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;
        uPtr<HalfEdge> symNext = mkU<HalfEdge>(nullptr, nullptr, nullptr, nullptr, edgeCount);
        edgeCount++;

        currNext.get()->setVert(curr->getVert());
        symNext.get()->setVert(sym->getVert());

        currNext.get()->setNext(curr->getNext());
        symNext.get()->setNext(sym->getNext());

        currNext->getVert()->setVertEdge(curr->getNext());

        currNext.get()->setFace(curr->getFace());
        symNext.get()->setFace(sym->getFace());

        Face* f1 = curr->getFace();
        Face* f2 = sym->getFace();

        float f1x = centroidX[f1];
        float f1y = centroidY[f1];
        float f1z = centroidZ[f1];

        float f2x = centroidX[f2];
        float f2y = centroidY[f2];
        float f2z = centroidZ[f2];

        glm::vec3 c1 = glm::vec3(f1x, f1y, f1z);
        glm::vec3 c2 = glm::vec3(f2x, f2y, f2z);

        glm::vec3 smoothedMidpoint = (currVert->getVertPos() + symVert->getVertPos() + c1 + c2) * 0.25f;

        midPointMap[curr->getFace()].push_back(curr);
        midPointMap[sym->getFace()].push_back(sym);

        uPtr<Vertex> newVert = mkU<Vertex>(nullptr, vertCount, smoothedMidpoint);
        vertCount++;

        newVert.get()->setVertEdge(curr);

        midVertMap[newVert.get()] = newVert.get();

        curr->setVert(newVert.get());
        sym->setVert(newVert.get());

        curr->setNext(currNext.get());
        sym->setNext(symNext.get());

        currNext.get()->setSym(sym);
        symNext.get()->setSym(curr);

        emit vertToMain(newVert.get());
        emit edgeToMain(currNext.get());
        emit edgeToMain(symNext.get());

        tempEdgeList.push_back(std::move(currNext));
        tempEdgeList.push_back(std::move(symNext));
        tempVertList.push_back(std::move(newVert));
        update();

    }
}


// function that add midpoints to every single edge
void MyGL::addMidPoints(Mesh *m) {
    tempEdgeList.clear();
    tempVertList.clear();
    for (unsigned int i = 0; i < m->edgeList.size(); i++) {
        HalfEdge* e = m->edgeList[i].get();
        e->getEdgeID();

        HalfEdge* sym = e->getSym();

        if (!(splitEdges.count(sym) || splitEdges.count(e))) {
            createVert(e, m);
            splitEdges[sym] = sym;
            splitEdges[e] = e;
        }
    }
    for (unsigned int i = 0; i < tempEdgeList.size(); i++) {
        emit edgeToMain(tempEdgeList[i].get());
        m->edgeList.push_back(std::move(tempEdgeList[i]));
    }
    for (unsigned int i = 0; i < tempVertList.size(); i++) {
        emit vertToMain(tempVertList[i].get());
        m->vertList.push_back(std::move(tempVertList[i]));
    }

    tempEdgeList.clear();
    tempVertList.clear();
    update();
}

// draws the line display
void MyGL::drawLine(EdgePoint e) {
    m_progLambert.draw(e);
    update();
}

// control slots

// test to see which vertex is clicked
void MyGL::vertClick(QListWidgetItem(* v)) {
    Vertex *vert = dynamic_cast<Vertex*>(v);
    vertDisplayPointer = vert;
}

// test to see which edge is clicked
void MyGL::edgeClick(QListWidgetItem(* e)) {
    HalfEdge *he = dynamic_cast<HalfEdge*>(e);
    edgeDisplayPointer = he;
}

// change x value of vertex position
void MyGL::vertXVal(double val) {
    if (vertDisplayPointer != nullptr) {
        glm::vec3 pos = vertDisplayPointer->getVertPos();
        pos.x = val * 0.5f;
        vertDisplayPointer->setVertPos(pos);
        sendVert(vertDisplayPointer);
        currentMesh.destroy();
        currentMesh.create();
        update();
    }
}

// change y value of vertex position
void MyGL::vertYVal(double val) {
    if (vertDisplayPointer != nullptr) {
        glm::vec3 pos = vertDisplayPointer->getVertPos();
        pos.y = val * 0.5f;
        vertDisplayPointer->setVertPos(pos);
        sendVert(vertDisplayPointer);
        currentMesh.destroy();
        currentMesh.create();
        update();
    }
}

// change z value of vertex position
void MyGL::vertZVal(double val) {
    if (vertDisplayPointer != nullptr) {
        glm::vec3 pos = vertDisplayPointer->getVertPos();
        pos.z = val * 0.5f;
        vertDisplayPointer->setVertPos(pos);
        sendVert(vertDisplayPointer);
        currentMesh.destroy();
        currentMesh.create();
        update();
    }
}

// change r value of face color
void MyGL::changeRVal(double val) {
    if (faceDisplayPointer != nullptr) {
        glm::vec3 col = faceDisplayPointer->getColor();
        col.r = val;
        faceDisplayPointer->setColor(col);
        currentMesh.destroy();
        currentMesh.create();
        update();
    }
}

// change g value of face color
void MyGL::changeGVal(double val) {
    if (faceDisplayPointer != nullptr) {
        glm::vec3 col = faceDisplayPointer->getColor();
        col.g = val;
        faceDisplayPointer->setColor(col);
        currentMesh.destroy();
        currentMesh.create();
        update();
    }
}

// change b value of face color
void MyGL::changeBVal(double val) {
    if (faceDisplayPointer != nullptr) {
        glm::vec3 col = faceDisplayPointer->getColor();
        col.b = val;
        faceDisplayPointer->setColor(col);
        currentMesh.destroy();
        currentMesh.create();
        update();
    }
}


// check to see if face is clicked
void MyGL::faceClick(QListWidgetItem(* f)) {
    Face *fa = dynamic_cast<Face*>(f);
    this->faceDisplayPointer = fa;
}

void MyGL::rotateJointX() {
    if (currentJoint != nullptr) {
        currentJoint->xRotation += 5.0f;
        update();
    }
}

void MyGL::rotateJointY() {
    if (currentJoint != nullptr) {
        currentJoint->yRotation += 5.0f;
        update();
    }
}

void MyGL::rotateJointZ() {
    if (currentJoint != nullptr) {
        currentJoint->zRotation += 5.0f;
        update();
    }
}

void MyGL::rotateNegativeJointX() {
    if (currentJoint != nullptr) {
        currentJoint->xRotation -= 5.0f;
        update();
    }
}

void MyGL::rotateNegativeJointY() {
    if (currentJoint != nullptr) {
        currentJoint->yRotation -= 5.0f;
        update();
    }
}

void MyGL::rotateNegativeJointZ() {
    if (currentJoint != nullptr) {
        currentJoint->zRotation -= 5.0f;
        update();
    }
}


float calculateDistance(glm::vec3 pos1, glm::vec3 pos2) {


    return glm::distance(pos1, pos2);
};

void MyGL::skinMesh() {

    for (unsigned int i = 0; i < currentMesh.vertList.size(); i++) {
        float minDist = 1000000000.f;
        float secondMin = 100000000.f;

        Joint* firstJoint;
        Joint* secondJoint;

        int firstIndex;
        int secondIndex;

        Vertex* currentVert = currentMesh.vertList[i].get();
        for (unsigned int j = 0; j < currentMesh.jointList.size(); j++) {
            glm::vec3 pos1 = currentVert->getVertPos();
            glm::vec3 pos2 = currentMesh.jointList[j]->actualPos;

            float calculatedDistance = glm::distance(pos1, glm::vec3(currentMesh.jointList[j]->actualPos));
            if (calculatedDistance < minDist && calculatedDistance < secondMin) {
                secondMin = minDist;
                minDist = calculatedDistance;
                secondJoint = firstJoint;
                secondIndex = firstIndex;
                firstJoint = currentMesh.jointList[j].get();
                firstIndex = j;
            } else if (calculatedDistance < secondMin && calculatedDistance > minDist) {
                secondMin = calculatedDistance;
                secondJoint = currentMesh.jointList[j].get();
                secondIndex = j;
            }
        }
//        currentVert->associatedJoints.insert(std::pair<Joint*, float>(firstJoint, maxDistance / (maxDistance + secondDistance)));
//        currentVert->associatedJoints.insert(std::pair<Joint*, float>(secondJoint, secondDistance / (maxDistance + secondDistance)));

//        currentVert->associatedJoints.insert(std::pair<Joint*, float>(firstJoint, maxDistance / (maxDistance + secondDistance)));
//        currentVert->associatedJoints.insert(std::pair<Joint*, float>(secondJoint, secondDistance / (maxDistance + secondDistance)));

        // std::cout << "Distance " << maxDistance << std::endl;

        currentVert->influences = glm::ivec2(firstIndex, secondIndex);
//        std::cout << "Index 1 " << firstIndex << std::endl;
//        std::cout << "Index 2 " << secondIndex << std::endl;

//         std::cout << "Weight 1 " << maxDistance / (maxDistance + secondDistance) << std::endl;
//         std::cout << "Weight 2 " << secondDistance / (maxDistance + secondDistance) << std::endl;

        currentVert->weights = glm::vec2(secondMin / (secondMin + minDist), minDist / (secondMin + minDist));
    }
    std::cout << "Mesh Skinned" << std::endl;
    currentMesh.destroy();
    currentMesh.create();
    update();
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    float amount = 2.0f;
    if (e->modifiers() & Qt::ShiftModifier) {
        amount = 10.0f;
    }

    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_glCamera.RotatePhi(-amount * 0.04f);
    } else if (e->key() == Qt::Key_Left) {
        m_glCamera.RotatePhi(amount * 0.04f);
    } else if (e->key() == Qt::Key_Up) {
        m_glCamera.RotateTheta(-amount * 0.04f);
    } else if (e->key() == Qt::Key_Down) {
        m_glCamera.RotateTheta(amount * 0.04f);
    } else if (e->key() == Qt::Key_1) {
        m_glCamera.fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        m_glCamera.fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        m_glCamera.Zoom(amount);
    } else if (e->key() == Qt::Key_S) {
        m_glCamera.Zoom(-amount);
    } else if (e->key() == Qt::Key_D) {
        m_glCamera.TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        m_glCamera.TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        m_glCamera.TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        m_glCamera.TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        m_glCamera = Camera(this->width(), this->height());
    } else if (e->key() == 78) {
        if (edgeDisplayPointer != nullptr) {
            edgeDisplayPointer = edgeDisplayPointer->getNext();
            sendEdge(edgeDisplayPointer);
        }
    } else if (e->key() == 77) {
        if (edgeDisplayPointer != nullptr) {
            edgeDisplayPointer = edgeDisplayPointer->getSym();
            sendEdge(edgeDisplayPointer);
        }
    } else if (e->key() == 70) {
        if (edgeDisplayPointer != nullptr) {
            faceDisplayPointer = edgeDisplayPointer->getFace();
            sendFace(faceDisplayPointer);
        }
    } else if (e->key() == 86) {
        if (edgeDisplayPointer != nullptr) {
            vertDisplayPointer = edgeDisplayPointer->getVert();
            sendVert(vertDisplayPointer);
        }
    } else if (e->key() == 72 && amount == 2.0f) {
        if (vertDisplayPointer != nullptr) {
            edgeDisplayPointer = vertDisplayPointer->getVertEdge();
            sendEdge(edgeDisplayPointer);
        }
    } else if (e->key() == 72 && amount == 10.0f) {
        if (faceDisplayPointer != nullptr) {
            edgeDisplayPointer = faceDisplayPointer->getFaceEdge();
            sendEdge(edgeDisplayPointer);
        }
    }
    m_glCamera.RecomputeAttributes();
    update();  // Calls paintGL, among other things
}
