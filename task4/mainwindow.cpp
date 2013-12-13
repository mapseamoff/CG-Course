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
    sbPCount->setRange(1, 1000000);
    sbPCount->setSingleStep(50);
    sbPCount->setValue(10000);

    sbPSSize = new QSpinBox(this);
    sbPSSize->setRange(50, 1000);
    sbPSSize->setSingleStep(50);
    sbPSSize->setValue(400);

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

    QPushButton *pbGenerate = new QPushButton("Generate", this);
    connect(pbGenerate, SIGNAL(clicked()), this, SLOT(generateParticles()));

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

    optLayout->addWidget(gbDynamicOptions, 3, 0, 1, 2);
    optLayout->addWidget(new QLabel("Billboard type:", this), 4, 0);
    optLayout->addWidget(cbBType, 4, 1);
    optLayout->addWidget(new QLabel("Thres distance:", this), 5, 0);
    optLayout->addWidget(sbTDist, 5, 1);
    optLayout->addWidget(cbShowTerrain, 6, 0, 1, 2);

    optLayout->addWidget(pbGenerate, 8, 0, 1, 2);
    optLayout->setRowStretch(7, 1);
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
    viewer->setTerrainBox(":/models/sphere.obj", splitCubemap(":/textures/skybox1.png"));
}

QImage MainWindow::getSubImage(const QImage &img, const QRect &rect) const {
    size_t offset = rect.x() * img.depth() / 8 + rect.y() * img.bytesPerLine();
    return QImage(img.bits() + offset, rect.width(), rect.height(), img.bytesPerLine(), img.format());
}

QList<QImage> MainWindow::splitCubemap(const QString &file, bool save) const {
    QImage img(file);
    int rw = 0;
    int rh = 0;
    for(int i = 0; i < img.width(); ++i) {
        if(rw == 0 && img.pixel(i, 0) != qRgb(255,255,255)) rw = i;
        if(rh == 0 && img.pixel(0, i) != qRgb(255,255,255)) rh = i;
        if(rw != 0 && rh != 0) break;
    }

    QImage posY = getSubImage(img, QRect(rw, 0, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage negY = getSubImage(img, QRect(rw, 2*rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage negX = getSubImage(img, QRect(0, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage posZ = getSubImage(img, QRect(rw, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage posX = getSubImage(img, QRect(2*rw, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);
    QImage negZ = getSubImage(img, QRect(3*rw, rh, rw, rh)).convertToFormat(QImage::Format_RGB888);

    if(save) {
        posX.save("posX.png"); negX.save("negX.png");
        posY.save("posY.png"); negY.save("negY.png");
        posZ.save("posZ.png"); negZ.save("negZ.png");
    }

    QList<QImage> res;
    res.append(posX); res.append(negX);
    res.append(posY); res.append(negY);
    res.append(posZ); res.append(negZ);
    return res;
}
