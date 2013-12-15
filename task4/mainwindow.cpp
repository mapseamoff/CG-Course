#include "mainwindow.h"
#include "colorpicker.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QGLFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QGLFormat::CoreProfile);
    glFormat.setSampleBuffers(true);

    viewer = new ModelViewer(glFormat, this);    
    connect(viewer, SIGNAL(openGLInitialized()), this, SLOT(setTerrain()));

    QGroupBox *gbStaticOptions = new QGroupBox("Static", this);
    gbStaticOptions->setFlat(true);

    sbPCount = new QSpinBox(this);
    sbPCount->setRange(1, 100000);
    sbPCount->setSingleStep(50);
    sbPCount->setValue(10000);

    sbPSSize = new QSpinBox(this);
    sbPSSize->setRange(50, 1000);
    sbPSSize->setSingleStep(50);
    sbPSSize->setValue(400);

    sbTCSize = new QSpinBox(this);
    sbTCSize->setRange(10, 400);
    sbTCSize->setSingleStep(10);
    sbTCSize->setValue(20);

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

    QGroupBox *gbTerrainOptions = new QGroupBox("Terrain", this);
    gbTerrainOptions->setFlat(true);

    sbTPers = new QDoubleSpinBox(this);
    sbTPers->setRange(0.1, 10.0);
    sbTPers->setValue(2.0);

    sbTFreq = new QDoubleSpinBox(this);
    sbTFreq->setRange(0.1, 20.0);
    sbTFreq->setValue(10.0);

    sbTAmp = new QDoubleSpinBox(this);
    sbTAmp->setRange(0.1, 20.0);
    sbTAmp->setValue(10.0);

    sbTOct = new QSpinBox(this);
    sbTOct->setRange(1, 10);
    sbTOct->setValue(3);

    QPushButton *pbUpdateTerrain = new QPushButton("Update terrain", this);
    connect(pbUpdateTerrain, SIGNAL(clicked()), this, SLOT(generateTerrain()));

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

    optLayout->setRowStretch(10, 1);

    optLayout->addWidget(gbTerrainOptions, 11, 0, 1, 2);
    optLayout->addWidget(new QLabel("Persistence:", this), 12, 0);
    optLayout->addWidget(sbTPers, 12, 1);
    optLayout->addWidget(new QLabel("Frequency:", this), 13, 0);
    optLayout->addWidget(sbTFreq, 13, 1);
    optLayout->addWidget(new QLabel("Amplitude:", this), 14, 0);
    optLayout->addWidget(sbTAmp, 14, 1);
    optLayout->addWidget(new QLabel("Octaves:", this), 15, 0);
    optLayout->addWidget(sbTOct, 15, 1);
    optLayout->addWidget(pbUpdateTerrain, 16, 0, 1, 2);

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

void MainWindow::setTerrain() {
    /*
    viewer->setTerrainBox(":/models/sphere.obj",
                          ":/textures/sp3right.jpg", ":/textures/sp3left.jpg",
                          ":/textures/sp3top.jpg", ":/textures/sp3bot.jpg",
                          ":/textures/sp3front.jpg", ":/textures/sp3back.jpg");
    */
    viewer->setTerrainBox(":/textures/skybox1.png");
}

void MainWindow::generateTerrain() {
    viewer->generateTerrain(sbTPers->value(), sbTFreq->value(), sbTAmp->value(), sbTOct->value());
}
