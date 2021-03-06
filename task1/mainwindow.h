#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include <QDoubleSpinBox>

#include "modelviewer.h"
#include "objmodel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void loadModel();
    void showModel(bool status);
    void setOutlineColor();
    void updateNearPlane(double val);
    void updateFarPlane(double val);

private:
    ModelViewer *viewer;
    OBJModel *model;

    QProgressDialog *pdLoading;
    QDoubleSpinBox *sbR, *sbG, *sbB;
    QDoubleSpinBox *sbNear, *sbFar;
};

#endif // MAINWINDOW_H
