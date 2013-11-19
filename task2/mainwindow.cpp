#include "mainwindow.h"
#include "colorpicker.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QGLFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QGLFormat::CoreProfile);
    glFormat.setSampleBuffers(true);

    viewer = new ModelViewer(glFormat, this);

    mPlane = new OBJModel(this);
    mPlane->loadModel(":/models/plane.obj", ":/textures/lenna_head.jpg");
    connect(mPlane, SIGNAL(loadStatus(bool)), this, SLOT(showModel(bool)));

    mCube = new OBJModel(this);
    mCube->loadModel(":/models/cube.obj", ":/textures/lenna_head.jpg");
    connect(mCube, SIGNAL(loadStatus(bool)), this, SLOT(showModel(bool)));

    mSphere = new OBJModel(this);
    mSphere->loadModel(":/models/sphere.obj", ":/textures/earth_texture_grid.bmp");
    connect(mSphere, SIGNAL(loadStatus(bool)), this, SLOT(showModel(bool)));

    sbM = new QDoubleSpinBox(this);
    sbM->setRange(0.01, 1E3);
    sbM->setValue(1.0);
    sbM->setSingleStep(0.1);
    connect(sbM, SIGNAL(valueChanged(double)), viewer, SLOT(setTexCoordsMultiplier(double)));

    cbFiltering = new QComboBox(this);
    cbFiltering->addItem("nearest", GL_NEAREST);
    cbFiltering->addItem("linear", GL_LINEAR);
    cbFiltering->addItem("nearest-mipmap-nearest", GL_NEAREST_MIPMAP_NEAREST);
    cbFiltering->addItem("nearest-mipmap-linear", GL_NEAREST_MIPMAP_LINEAR);
    cbFiltering->addItem("linear-mipmap-nearest", GL_LINEAR_MIPMAP_NEAREST);
    cbFiltering->addItem("linear-mipmap-linear", GL_LINEAR_MIPMAP_LINEAR);
    cbFiltering->setCurrentIndex(0);
    connect(cbFiltering, SIGNAL(currentIndexChanged(int)), this, SLOT(setFilteringType(int)));

    cbModels = new QComboBox(this);
    cbModels->addItem("Plane");
    cbModels->addItem("Cube");
    cbModels->addItem("Sphere");
    cbModels->setCurrentIndex(1);
    cbModels->setEnabled(false);
    connect(cbModels, SIGNAL(currentIndexChanged(int)), this, SLOT(loadModel(int)));

    ColorPicker *cpMeshColor = new ColorPicker(this);
    connect(cpMeshColor, SIGNAL(valueChanged(QVector3D)), viewer, SLOT(setMeshColor(QVector3D)));

    QCheckBox *ckbDrawMesh = new QCheckBox("Draw mesh", this);
    ckbDrawMesh->setChecked(true);
    connect(ckbDrawMesh, SIGNAL(toggled(bool)), viewer, SLOT(setDrawOutline(bool)));
//    connect(ckbDrawMesh, SIGNAL(toggled(bool)), cpMeshColor, SLOT(setEnabled(bool)));

    ckbDrawMipLevels = new QCheckBox("Show computed mip levels", this);
    ckbDrawMipLevels->setChecked(false);
    ckbDrawMipLevels->setEnabled(false);
    connect(ckbDrawMipLevels, SIGNAL(toggled(bool)), viewer, SLOT(setDrawMipLevels(bool)));

    QCheckBox *ckbDrawMipmapTexture = new QCheckBox("Show real mipmap texture", this);
    ckbDrawMipmapTexture->setChecked(false);
    connect(ckbDrawMipmapTexture, SIGNAL(toggled(bool)), viewer, SLOT(setDrawRealMipmap(bool)));

    QGroupBox *gbOptions = new QGroupBox("Options", this);
    QGridLayout *optLayout = new QGridLayout();
    optLayout->setSpacing(5);
    optLayout->setContentsMargins(5, 5, 5, 5);
    optLayout->setColumnStretch(0, 0);
    optLayout->setColumnStretch(1, 1);
    optLayout->addWidget(new QLabel("Model:", this), 0, 0);
    optLayout->addWidget(cbModels, 0, 1);
    optLayout->addWidget(new QLabel("UV Multiplier:", this), 1, 0);
    optLayout->addWidget(sbM, 1, 1);
    optLayout->addWidget(new QLabel("Filtering:", this), 2, 0);
    optLayout->addWidget(cbFiltering, 2, 1);
    optLayout->addWidget(ckbDrawMesh, 3, 0);
    optLayout->addWidget(cpMeshColor, 3, 1, Qt::AlignRight);
    optLayout->addWidget(ckbDrawMipmapTexture, 4, 0, 1, 2);
    optLayout->addWidget(ckbDrawMipLevels, 5, 0, 1, 2);
    optLayout->setRowStretch(6, 1);
    gbOptions->setLayout(optLayout);

    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    layout->addWidget(viewer, 1);
    layout->addWidget(gbOptions, 0);
//    QGridLayout *layout = new QGridLayout();
//    layout->setContentsMargins(5, 5, 5, 5);
//    layout->setSpacing(5);
//    layout->addWidget(viewer, 0, 0, 1, 3);
//    layout->addWidget(pbLoadModel, 1, 0);
//    layout->addLayout(optLayout, 1, 2);
//    layout->setColumnStretch(1, 1);
    w->setLayout(layout);

    this->setCentralWidget(w);
    this->setWindowTitle("CG Task 2");

}

void MainWindow::loadModel(int idx) {
    switch(idx) {
    case 0: viewer->setModel(mPlane); break;
    case 1: viewer->setModel(mCube); break;
    case 2: viewer->setModel(mSphere); break;
    }
}

void MainWindow::setFilteringType(int idx) {
    ckbDrawMipLevels->setEnabled(cbFiltering->itemText(idx).contains("mipmap"));
    viewer->setFilteringType(cbFiltering->itemData(idx).toInt());
}

void MainWindow::showModel(bool status) {
    if(!status) {
        QMessageBox::critical(this, "CG Task 2", "Unable to load model");
    } else if(mCube->status() && mPlane->status() && mSphere->status()) {
        cbModels->setEnabled(true);
        loadModel(cbModels->currentIndex());
    }
}
