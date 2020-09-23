#include "Mesh.h"
#include "Vertex.h"
#include "Face.h"
#include <QListWidgetItem>
#include "vertpoint.h"
#include "facepoint.h"
#include "edgepoint.h"
#include "smartpointerhelp.h"

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    // slots to put a object in the gui list
    void vertSlot(QListWidgetItem* v);
    void faceSlot(QListWidgetItem* f);
    void edgeSlot(QListWidgetItem* e);
    void loadJSON();
    void jointSlot(QTreeWidgetItem* j);


private slots:
    void on_actionQuit_triggered();

    void on_actionCamera_Controls_triggered();

private:
    Ui::MainWindow *ui;


};


#endif // MAINWINDOW_H
