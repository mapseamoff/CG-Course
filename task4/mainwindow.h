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
    void generateParticles();
    void generateTerrain();
    void setTerrain();

private:
    ModelViewer *viewer;
    QSpinBox *sbPCount, *sbPSSize, *sbTCSize;
    QDoubleSpinBox *sbTDist;

    QDoubleSpinBox *sbTPers, *sbTFreq, *sbTAmp;
    QSpinBox *sbTOct;

};

#endif // MAINWINDOW_H
