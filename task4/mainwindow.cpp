#include "mainwindow.h"
#include "colorpicker.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

#include <QPushButton>

#include "terrain.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QGLFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QGLFormat::CoreProfile);
    glFormat.setSampleBuffers(true);

    cubemapTex.append(QImage(":/textures/posZ.png").convertToFormat(QImage::Format_RGB888));
    cubemapTex.append(QImage(":/textures/negX.png").convertToFormat(QImage::Format_RGB888));
    cubemapTex.append(QImage(":/textures/posY.png").convertToFormat(QImage::Format_RGB888));
    cubemapTex.append(QImage(":/textures/negY.png").convertToFormat(QImage::Format_RGB888));
    cubemapTex.append(QImage(":/textures/negZ.png").convertToFormat(QImage::Format_RGB888));
    cubemapTex.append(QImage(":/textures/posX.png").convertToFormat(QImage::Format_RGB888));

    viewer = new ModelViewer(glFormat, this);    
    connect(viewer, SIGNAL(openGLInitialized()), this, SLOT(setTerrain()));

    QGroupBox *gbStaticOptions = new QGroupBox("Static", this);
    gbStaticOptions->setFlat(true);

    sbPCount = new QSpinBox(this);
    sbPCount->setRange(4, 100000);
    sbPCount->setSingleStep(50);
    sbPCount->setValue(10000);

    sbPSSize = new QSpinBox(this);
    sbPSSize->setRange(50, 1000);
    sbPSSize->setSingleStep(50);
    sbPSSize->setValue(400);

    sbTCSize = new QSpinBox(this);
    sbTCSize->setRange(1, 400);
    sbTCSize->setSingleStep(1);
    sbTCSize->setValue(2);

    QGroupBox *gbDynamicOptions = new QGroupBox("Dynamic", this);
    gbDynamicOptions->setFlat(true);

    QComboBox *cbBType = new QComboBox(this);
    cbBType->addItem("perspective");
    cbBType->addItem("fixed size");
    cbBType->setCurrentIndex(0);
    connect(cbBType, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setBillboardType(int)));

    sbTDist = new QDoubleSpinBox(this);
    sbTDist->setRange(0, 100);
    sbTDist->setSingleStep(1);
    sbTDist->setValue(50);
    connect(sbTDist, SIGNAL(valueChanged(double)), viewer, SLOT(setDistanceThreshold(double)));

    QCheckBox *cbShowTerrain = new QCheckBox("Show skybox", this);
    cbShowTerrain->setChecked(true);
    connect(cbShowTerrain, SIGNAL(toggled(bool)), viewer, SLOT(setShowTerrain(bool)));

    QCheckBox *cbShowWireframe = new QCheckBox("Wireframe", this);
    cbShowWireframe->setChecked(false);
    connect(cbShowWireframe, SIGNAL(toggled(bool)), viewer, SLOT(setWireframeMode(bool)));

    QPushButton *pbGenerate = new QPushButton("Generate", this);
    connect(pbGenerate, SIGNAL(clicked()), this, SLOT(generateParticles()));

    //--------------------------------------------------------------------------------

    QGroupBox *gbTerrainOptions = new QGroupBox("Terrain", this);
    gbTerrainOptions->setFlat(true);

    cbTerrainTexture = new QComboBox(this);
    cbTerrainTexture->addItem("texture 1");
    cbTerrainTexture->addItem("nvidia sdk");
    cbTerrainTexture->addItem("bad cubemap");
    connect(cbTerrainTexture, SIGNAL(currentIndexChanged(int)), this, SLOT(setTerrainTexture(int)));

    QComboBox *cbTerrainMode = new QComboBox(this);
    cbTerrainMode->addItem("texture");
    cbTerrainMode->addItem("facet norm.");
    cbTerrainMode->addItem("vertex norm.");
    connect(cbTerrainMode, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setTerrainTexMode(int)));

    QDoubleSpinBox *sbContrast = new QDoubleSpinBox(this);
    sbContrast->setRange(0.1, 10.0);
    sbContrast->setSingleStep(0.1);
    sbContrast->setValue(1.0);
    connect(sbContrast, SIGNAL(valueChanged(double)), viewer, SLOT(setTerrainContrast(double)));

    sbTPers = new QSpinBox(this);
    sbTPers->setRange(1, 500);
//    sbTPers->setSingleStep(10);
    sbTPers->setValue(10);

    sbTFreq = new QSpinBox(this);
    sbTFreq->setRange(1, 500);
//    sbTFreq->setSingleStep(10);
    sbTFreq->setValue(10);

    sbTAmp = new QDoubleSpinBox(this);
    sbTAmp->setRange(0.1, 100.0);
    sbTAmp->setValue(30.0);

    sbTOct = new QSpinBox(this);
    sbTOct->setRange(1, 10);
    sbTOct->setValue(3);

    QPushButton *pbUpdateTerrain = new QPushButton("Update terrain", this);
    connect(pbUpdateTerrain, SIGNAL(clicked()), this, SLOT(generateTerrain()));

    QGridLayout *terLayout = new QGridLayout();
    terLayout->setContentsMargins(0, 0, 0, 0);
    terLayout->setSpacing(5);
    terLayout->addWidget(new QLabel("Texture:", this), 0, 0);
    terLayout->addWidget(cbTerrainTexture, 0, 1);
    terLayout->addWidget(new QLabel("Mode:", this), 1, 0);
    terLayout->addWidget(cbTerrainMode, 1, 1);
    terLayout->addWidget(new QLabel("Contrast:", this), 2, 0);
    terLayout->addWidget(sbContrast, 2, 1);
    terLayout->addWidget(new QLabel("Persistence:", this), 3, 0);
    terLayout->addWidget(sbTPers, 3, 1);
    terLayout->addWidget(new QLabel("Frequency:", this), 4, 0);
    terLayout->addWidget(sbTFreq, 4, 1);
    terLayout->addWidget(new QLabel("Amplitude:", this), 5, 0);
    terLayout->addWidget(sbTAmp, 5, 1);
    terLayout->addWidget(new QLabel("Octaves:", this), 6, 0);
    terLayout->addWidget(sbTOct, 6, 1);
    terLayout->addWidget(pbUpdateTerrain, 7, 0, 1, 2);
    gbTerrainOptions->setLayout(terLayout);

    //--------------------------------------------------------------------------------

    QGroupBox *gbCameraOptions = new QGroupBox("Camera", this);
    gbCameraOptions->setFlat(true);

    cbCameraMode = new QCheckBox("Single camera", this);
    cbCameraMode->setChecked(true);
    connect(cbCameraMode, SIGNAL(toggled(bool)), this, SLOT(setCameraMode(bool)));

    cbCurrentCamera = new QComboBox(this);
    cbCurrentCamera->addItem("particles");
    cbCurrentCamera->addItem("free");
    cbCurrentCamera->setEnabled(false);
    connect(cbCurrentCamera, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setCurrentCamera(int)));

    QGridLayout *camLayout = new QGridLayout();
    camLayout->setContentsMargins(0, 0, 0, 0);
    camLayout->setSpacing(5);
    camLayout->addWidget(cbCameraMode, 0, 0, 1, 2);
    camLayout->addWidget(new QLabel("Camera:", this), 1, 0);
    camLayout->addWidget(cbCurrentCamera, 1, 1);

    gbCameraOptions->setLayout(camLayout);


    //--------------------------------------------------------------------------------

    QGroupBox *gbOptions = new QGroupBox("Options", this);
    QGridLayout *optLayout = new QGridLayout();
    optLayout->setSpacing(5);
    optLayout->setContentsMargins(5, 5, 5, 5);
    optLayout->setColumnStretch(0, 0);
    optLayout->setColumnStretch(1, 1);
    optLayout->addWidget(gbStaticOptions, 0, 0, 1, 2);
    optLayout->addWidget(new QLabel("Max particles:", this), 1, 0);
    optLayout->addWidget(sbPCount, 1, 1);
    optLayout->addWidget(new QLabel("Cube size:", this), 2, 0);
    optLayout->addWidget(sbPSSize, 2, 1);
    optLayout->addWidget(new QLabel("Terrain grid size:", this), 3, 0);
    optLayout->addWidget(sbTCSize, 3, 1);

    optLayout->addWidget(gbDynamicOptions, 4, 0, 1, 2);
    optLayout->addWidget(new QLabel("Billboard type:", this), 5, 0);
    optLayout->addWidget(cbBType, 5, 1);
    optLayout->addWidget(new QLabel("Thres distance:", this), 6, 0);
    optLayout->addWidget(sbTDist, 6, 1);
    optLayout->addWidget(cbShowTerrain, 7, 0, 1, 2);
    optLayout->addWidget(cbShowWireframe, 8, 0, 1, 2);

    optLayout->addWidget(pbGenerate, 9, 0, 1, 2);
    optLayout->addWidget(gbTerrainOptions, 11, 0, 1, 2);
    optLayout->addWidget(gbCameraOptions, 12, 0, 1, 2);

    optLayout->setRowStretch(10, 1);
    gbOptions->setLayout(optLayout);

    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    layout->addWidget(viewer, 1);
    layout->addWidget(gbOptions, 0);
    w->setLayout(layout);

    this->setCentralWidget(w);
    this->setWindowTitle("CG Task 4");
}

void MainWindow::generateParticles() {
    sbTDist->setRange(0.0, sbPSSize->value() / 2);
    viewer->initParticles(sbPCount->value(), ":/textures/snowflakes.jpg");
    viewer->initTerrain(sbPSSize->value(), sbTCSize->value());
    generateTerrain();
    viewer->generateParticles(sbPSSize->value());
    viewer->setFocus();
}

void MainWindow::setTerrainTexture(int idx) {
    switch(idx) {
    case 0: viewer->setTerrainBox(":/textures/skybox1.png"); break;
    case 1: viewer->setTerrainBox(cubemapTex); break;
    case 2: viewer->setTerrainBox(":/textures/skybox2.jpg"); break;
    default: break;
    }
}

void MainWindow::setTerrain() {
    setTerrainTexture(cbTerrainTexture->currentIndex());
    viewer->setFrustumModel(":/models/frustum.obj");
    viewer->resetView();
}

void MainWindow::generateTerrain() {
    viewer->generateTerrain(sbTPers->value() / 100.0, sbTFreq->value() / 100.0, sbTAmp->value(), sbTOct->value());
}

void MainWindow::setCameraMode(bool m) {
    if(m) {
        viewer->setCameraMode(true);
        cbCurrentCamera->setEnabled(false);
    } else {
        cbCurrentCamera->setEnabled(true);
        viewer->setCameraMode(false);
        viewer->setCurrentCamera(cbCurrentCamera->currentIndex());
    }
}
