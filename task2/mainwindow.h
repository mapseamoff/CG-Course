#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include "modelviewer.h"
#include "objmodel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void loadModel(int idx);
    void setFilteringType(int idx);
    void showModel(bool status);

private:
    ModelViewer *viewer;
    OBJModel *mPlane, *mCube, *mSphere;

    QComboBox *cbModels, *cbFiltering;
    QDoubleSpinBox *sbR, *sbG, *sbB;
    QDoubleSpinBox *sbM;
    QCheckBox *ckbDrawMipLevels;

};

#endif // MAINWINDOW_H
