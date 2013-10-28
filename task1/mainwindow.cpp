#include "mainwindow.h"

#include <QPushButton>
#include <QRadioButton>
#include <QFileDialog>
#include <QProgressDialog>
#include <QSignalMapper>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QGLFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QGLFormat::CoreProfile);
    glFormat.setSampleBuffers(true);

    viewer = new ModelViewer(glFormat, this);

    QPushButton *pbLoadModel = new QPushButton("Load model...", this);
    connect(pbLoadModel, SIGNAL(clicked()), this, SLOT(loadModel()));

    model = new OBJModel(this);
    connect(model, SIGNAL(loadStatus(bool)), this, SLOT(showModel(bool)));

    pdLoading = new QProgressDialog("Loading model...", "Cancel", 0, 100, this);
    pdLoading->setMinimumDuration(0);
    pdLoading->setWindowTitle("CG Task 1");
    connect(model, SIGNAL(loadProgress(int)), pdLoading, SLOT(setValue(int)));
    connect(pdLoading, SIGNAL(canceled()), model, SLOT(stopLoading()));

    QSignalMapper *dsm = new QSignalMapper(this);
    QRadioButton *rbUseZ = new QRadioButton("z-coord", this);
    QRadioButton *rbUseFC = new QRadioButton("gl_FragCoord.z", this);
    rbUseFC->setChecked(true);
    connect(rbUseZ, SIGNAL(clicked()), dsm, SLOT(map()));
    connect(rbUseFC, SIGNAL(clicked()), dsm, SLOT(map()));
    connect(dsm, SIGNAL(mapped(int)), viewer, SLOT(setFillMethod(int)));
    dsm->setMapping(rbUseFC, 0);
    dsm->setMapping(rbUseZ, 1);

    sbR = new QDoubleSpinBox(this);
    sbG = new QDoubleSpinBox(this);
    sbB = new QDoubleSpinBox(this);
    sbR->setRange(0, 1);
    sbR->setSingleStep(0.01);
    sbG->setRange(0, 1);
    sbG->setSingleStep(0.01);
    sbB->setRange(0, 1);
    sbB->setSingleStep(0.01);
    connect(sbR, SIGNAL(valueChanged(double)), this, SLOT(setOutlineColor()));
    connect(sbG, SIGNAL(valueChanged(double)), this, SLOT(setOutlineColor()));
    connect(sbB, SIGNAL(valueChanged(double)), this, SLOT(setOutlineColor()));

    sbNear = new QDoubleSpinBox(this);
    sbNear->setRange(0.1, 1E3);
    sbNear->setValue(0.1);
    connect(sbNear, SIGNAL(valueChanged(double)), viewer, SLOT(setNearPlane(double)));

    sbFar = new QDoubleSpinBox(this);
    sbFar->setRange(0.1, 1E3);
    sbFar->setValue(100.0);
    connect(sbFar, SIGNAL(valueChanged(double)), viewer, SLOT(setFarPlane(double)));

    connect(viewer, SIGNAL(nearPlaneChanged(double)), this, SLOT(updateNearPlane(double)));
    connect(viewer, SIGNAL(farPlaneChanged(double)), this, SLOT(updateFarPlane(double)));

    QHBoxLayout *optLayout = new QHBoxLayout();
    optLayout->setContentsMargins(0, 0, 0, 0);
    optLayout->setSpacing(5);
    optLayout->addWidget(new QLabel("Fill:", this));
    optLayout->addWidget(rbUseZ);
    optLayout->addWidget(rbUseFC);
    optLayout->addWidget(new QLabel("| Outline color: R", this));
    optLayout->addWidget(sbR);
    optLayout->addWidget(new QLabel("G", this));
    optLayout->addWidget(sbG);
    optLayout->addWidget(new QLabel("B", this));
    optLayout->addWidget(sbB);
    optLayout->addWidget(new QLabel("| Planes: near", this));
    optLayout->addWidget(sbNear);
    optLayout->addWidget(new QLabel("far", this));
    optLayout->addWidget(sbFar);

    QWidget *w = new QWidget(this);
    QGridLayout *layout = new QGridLayout();
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    layout->addWidget(viewer, 0, 0, 1, 3);
    layout->addWidget(pbLoadModel, 1, 0);
    layout->addLayout(optLayout, 1, 2);
    layout->setColumnStretch(1, 1);
    w->setLayout(layout);

    this->setCentralWidget(w);
    this->setWindowTitle("CG Task 1");
}

void MainWindow::loadModel() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select model", "", "Model files (*.obj)");
    if(fileName.isEmpty()) return;
    pdLoading->reset();
    model->loadModel(fileName.toStdString());
}

void MainWindow::showModel(bool status) {
    pdLoading->hide();
    if(!status) {
        QMessageBox::critical(this, "CG Task 1", QString("Unable to load model:\n%1").arg(model->modelError().c_str()));
    } else {
        viewer->setModel(model);
    }
}

void MainWindow::setOutlineColor() {
    viewer->setOutlineColor(sbR->value(), sbG->value(), sbB->value());
}

void MainWindow::updateNearPlane(double val) {
    sbNear->blockSignals(true);
    sbNear->setValue(val);
    sbNear->blockSignals(false);
}

void MainWindow::updateFarPlane(double val) {
    sbFar->blockSignals(true);
    sbFar->setValue(val);
    sbFar->blockSignals(false);
}
