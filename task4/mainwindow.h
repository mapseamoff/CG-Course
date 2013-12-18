#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include "modelviewer.h"
#include "colorpicker.h"

//-----------------------------------------------------

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void setTerrainTexture(int idx);
    void generateParticles();
    void generateTerrain();
    void setTerrain();
    void setCameraMode(bool m);

private:
    ModelViewer *viewer;
    QSpinBox *sbPCount, *sbPSSize, *sbTCSize;
    QDoubleSpinBox *sbTDist;

    QComboBox *cbTerrainTexture;
    QDoubleSpinBox *sbTAmp;
    QSpinBox *sbTPers, *sbTFreq, *sbTOct;

    QCheckBox *cbCameraMode;
    QComboBox *cbCurrentCamera;

    QList<QImage> cubemapTex;
};

#endif // MAINWINDOW_H
