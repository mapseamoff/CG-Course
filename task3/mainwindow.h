#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include "modelviewer.h"
#include "objmodel.h"
#include "colorpicker.h"

class PositionWidget : public QWidget {
    Q_OBJECT

public:
    PositionWidget(double min, double max, double ival, const QString &name, QWidget *parent = 0);

    QVector3D getValue() const;
    void setSingleStep(double val);

signals:
    void valueChanged(QVector3D);

private slots:
    void emitValue();

private:
    QDoubleSpinBox *sbLX, *sbLY, *sbLZ;
};

//-----------------------------------------------------

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void showModel(bool status);
    void setLightDirection(QVector3D point);

private:
    ModelViewer *viewer;
    OBJModel *model;

    QComboBox *cbShading, *cbFill;
    PositionWidget *pwLightPos, *pwLightDir;
    QDoubleSpinBox *sbSA;
    ColorPicker *cpAmbient, *cpDiffuse, *cpSpecular, *cpLight;

};

#endif // MAINWINDOW_H
