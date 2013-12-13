#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include "modelviewer.h"
#include "objmodel.h"
#include "colorpicker.h"

//-----------------------------------------------------

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void generateParticles();
    void setTerrain();

private:
    QList<QImage> splitCubemap(const QString &file, bool save = false) const;
    QImage getSubImage(const QImage &img, const QRect &rect) const;

    ModelViewer *viewer;
    QSpinBox *sbPCount, *sbPSSize;
    QDoubleSpinBox *sbTDist;

};

#endif // MAINWINDOW_H
