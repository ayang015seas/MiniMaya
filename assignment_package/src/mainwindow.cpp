#include "mainwindow.h"
#include <ui_mainwindow.h>
#include <QObject>
#include <QListWidgetItem>
#include "cameracontrolshelp.h"
#include "Vertex.h"
#include "HalfEdge.h"
#include "Face.h"
#include "mygl.h"
#include <iostream>
#include <QObject>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    ui->mygl->setFocus();

    connect(ui->mygl, SIGNAL(vertToMain(QListWidgetItem*)), this, SLOT(vertSlot(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(edgeToMain(QListWidgetItem*)), this, SLOT(edgeSlot(QListWidgetItem*)));
    connect(ui->mygl, SIGNAL(faceToMain(QListWidgetItem*)), this, SLOT(faceSlot(QListWidgetItem*)));

    connect(ui->vertsListWidget, SIGNAL(itemClicked(QListWidgetItem*)), ui->mygl, SLOT(sendVert(QListWidgetItem*)));
    connect(ui->halfEdgesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), ui->mygl, SLOT(sendEdge(QListWidgetItem*)));
    connect(ui->facesListWidget, SIGNAL(itemClicked(QListWidgetItem*)), ui->mygl, SLOT(sendFace(QListWidgetItem*)));

    connect(ui->vertPosXSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(vertXVal(double)));
    connect(ui->vertPosYSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(vertYVal(double)));
    connect(ui->vertPosZSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(vertZVal(double)));

    connect(ui->faceRedSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(changeRVal(double)));
    connect(ui->faceGreenSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(changeGVal(double)));
    connect(ui->faceBlueSpinBox, SIGNAL(valueChanged(double)), ui->mygl, SLOT(changeBVal(double)));

    connect(ui->vertButton, SIGNAL(clicked()), ui->mygl, SLOT(createVert()));
    connect(ui->triButton, SIGNAL(clicked()), ui->mygl, SLOT(triangulateFace()));
    connect(ui->quadButton, SIGNAL(clicked()), ui->mygl, SLOT(quadrangulate()));
    connect(ui->extrudeButton, SIGNAL(clicked()), ui->mygl, SLOT(extrudeSlot()));

    connect(ui->loadButton, SIGNAL(clicked()), ui->mygl, SLOT(loadFile()));
    connect(ui->jsonButton, SIGNAL(clicked()), ui->mygl, SLOT(loadJSON()));
    connect(ui->mygl, SIGNAL(jointToMain(QTreeWidgetItem*)), this, SLOT(jointSlot(QTreeWidgetItem*)));

    connect(ui->jointTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), ui->mygl, SLOT(jointClicked(QTreeWidgetItem*, int)));
    connect(ui->jointX, SIGNAL(clicked()), ui->mygl, SLOT(rotateJointX()));
    connect(ui->jointY, SIGNAL(clicked()), ui->mygl, SLOT(rotateJointY()));
    connect(ui->jointZ, SIGNAL(clicked()), ui->mygl, SLOT(rotateJointZ()));

    connect(ui->jointXNeg, SIGNAL(clicked()), ui->mygl, SLOT(rotateNegativeJointX()));
    connect(ui->jointYNeg, SIGNAL(clicked()), ui->mygl, SLOT(rotateNegativeJointY()));
    connect(ui->jointZNeg, SIGNAL(clicked()), ui->mygl, SLOT(rotateNegativeJointZ()));

    connect(ui->skinButton, SIGNAL(clicked()), ui->mygl, SLOT(skinMesh()));

}

void MainWindow::loadJSON() {
    QFileDialog dialog(this->ui->mygl);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Files (*.obj)"));

    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();

    QString fileName = fileNames[0];
    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        QString doc;
        doc = file.readAll();

        QJsonDocument tempDoc = QJsonDocument::fromJson(doc.toUtf8());
        QJsonObject tree = tempDoc.object();

        file.close();
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::exit();
}

void MainWindow::on_actionCamera_Controls_triggered()
{
    CameraControlsHelp* c = new CameraControlsHelp();
    c->show();
}

// add vertex to list gui
void MainWindow::vertSlot(QListWidgetItem* v)
{
    Vertex *vert = dynamic_cast<Vertex*>(v);
    QString id = QString::number(vert->getVertID());
    QString name = "Vertex ID: ";
    name.append(id);
    vert->setText(name);
    ui->vertsListWidget->addItem(v);
}

// add face to list gui
void MainWindow::faceSlot(QListWidgetItem* f) {
    Face *face = dynamic_cast<Face*>(f);
    QString id = QString::number(face->getFaceID());
    QString name = "Face ID: ";
    name.append(id);
    f->setText(name);
    ui->facesListWidget->addItem(f);
}

// add edge to list gui
void MainWindow::edgeSlot(QListWidgetItem* e) {
    HalfEdge *edge = dynamic_cast<HalfEdge*>(e);
    QString id = QString::number(edge->getEdgeID());
    QString name = "Edge ID: ";
    name.append(id);
    e->setText(name);
    ui->halfEdgesListWidget->addItem(e);
}

void MainWindow::jointSlot(QTreeWidgetItem* j) {
    // Joint* rootJoint = dynamic_cast<Joint*>(j);
    // j->setText(0, rootJoint->name);
    ui->jointTree->clear();
    ui->jointTree->addTopLevelItem(j);
}




