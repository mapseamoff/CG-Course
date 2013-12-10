#include "mainwindow.h"
#include "colorpicker.h"

#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QGLFormat glFormat;
    glFormat.setVersion(3, 3);
    glFormat.setProfile(QGLFormat::CoreProfile);
    glFormat.setSampleBuffers(true);

    viewer = new ModelViewer(glFormat, this);
    viewer->setDrawOutline(false);

    model = new OBJModel(this);
    model->loadModel(":/models/bunny_n.obj");
    connect(model, SIGNAL(loadStatus(bool)), this, SLOT(showModel(bool)));

    lightModel = new OBJModel(this);
    lightModel->loadModel(":/models/cone.obj");
    connect(lightModel, SIGNAL(loadStatus(bool)), this, SLOT(showModel(bool)));

    cbShading = new QComboBox(this);
    cbShading->addItem("Phong");
    cbShading->addItem("Blinn-Phong");
    cbShading->setCurrentIndex(0);
    connect(cbShading, SIGNAL(currentIndexChanged(int)), viewer, SLOT(setShadingMethod(int)));

    cbFill = new QComboBox(this);
    cbFill->addItem("flat");
    cbFill->addItem("per-vertex");
    cbFill->addItem("per-fragment");
    cbFill->setCurrentIndex(0);
    connect(cbFill, SIGNAL(currentIndexChanged(int)), viewer,  SLOT(setFillMethod(int)));

    ColorPicker *cpMeshColor = new ColorPicker(this);
    connect(cpMeshColor, SIGNAL(valueChanged(QVector3D)), viewer, SLOT(setMeshColor(QVector3D)));

    QCheckBox *ckbDrawMesh = new QCheckBox("Draw mesh", this);
    ckbDrawMesh->setChecked(false);
    connect(ckbDrawMesh, SIGNAL(toggled(bool)), viewer, SLOT(setDrawOutline(bool)));
//    connect(ckbDrawMesh, SIGNAL(toggled(bool)), cpMeshColor, SLOT(setEnabled(bool)));

    QGroupBox *gbMaterial = new QGroupBox("Material", this);
    gbMaterial->setFlat(true);

    cpAmbient = new ColorPicker(this);
    cpDiffuse = new ColorPicker(this);
    cpSpecular = new ColorPicker(this);
    cpLight = new ColorPicker(this);
    connect(cpAmbient, SIGNAL(valueChanged(QVector3D)), viewer, SLOT(setAmbientColor(QVector3D)));
    connect(cpDiffuse, SIGNAL(valueChanged(QVector3D)), viewer, SLOT(setDiffuseColor(QVector3D)));
    connect(cpSpecular, SIGNAL(valueChanged(QVector3D)), viewer, SLOT(setSpecularColor(QVector3D)));
    connect(cpLight, SIGNAL(valueChanged(QVector3D)), viewer, SLOT(setLightColor(QVector3D)));
    cpAmbient->setColor(Qt::black);
    cpDiffuse->setColor(Qt::gray);
    cpSpecular->setColor(Qt::white);
    cpLight->setColor(Qt::white);

    QDoubleSpinBox *sbSP = new QDoubleSpinBox(this);
    sbSP->setRange(1.0, 100.0);
    sbSP->setValue(20.0);
    sbSP->setSingleStep(1.0);
    connect(sbSP, SIGNAL(valueChanged(double)), viewer, SLOT(setSpecularPower(double)));

    QGroupBox *gbLight = new QGroupBox("Light", this);
    gbLight->setFlat(true);

    QDoubleSpinBox *sbLP = new QDoubleSpinBox(this);
    sbLP->setRange(0.0, 100.0);
    sbLP->setValue(50.0);
    sbLP->setSingleStep(1.0);
    connect(sbLP, SIGNAL(valueChanged(double)), viewer, SLOT(setLightPower(double)));

    pwLightPos = new PositionWidget(-99.0, 99.0, 4.0, "Position:", this);
    pwLightPos->setSingleStep(0.1);
    connect(pwLightPos, SIGNAL(valueChanged(QVector3D)), this, SLOT(setLightPosition(QVector3D)));

    pwLightDir = new PositionWidget(-99.0, 99.0, 0.0, "Points at:", this);
    pwLightDir->setSingleStep(0.05);
    connect(pwLightDir, SIGNAL(valueChanged(QVector3D)), this, SLOT(setLightDirection(QVector3D)));

    sbSA = new QDoubleSpinBox(this);
    sbSA->setRange(0.0, 180.0);
    sbSA->setValue(15.0);
    sbSA->setSingleStep(0.5);
    connect(sbSA, SIGNAL(valueChanged(double)), viewer, SLOT(setLightCutoff(double)));

    QCheckBox *cbLI = new QCheckBox("Interpolation (flashlight effect)", this);
    cbLI->setToolTip("Interpolation inside light cone. Gives more interesting effect than OpenGL's GL_SPOT_EXPONENT");
    cbLI->setChecked(true);
    connect(cbLI, SIGNAL(toggled(bool)), viewer, SLOT(setSpotMethod(bool)));

    QDoubleSpinBox *sbLE = new QDoubleSpinBox(this);
    sbLE->setRange(0.0, 128.0);
    sbLE->setValue(1.0);
    sbLE->setSingleStep(0.5);
    connect(sbLE, SIGNAL(valueChanged(double)), viewer, SLOT(setLightExponent(double)));

    QGroupBox *gbOptions = new QGroupBox("Options", this);
    QGridLayout *optLayout = new QGridLayout();
    optLayout->setSpacing(5);
    optLayout->setContentsMargins(5, 5, 5, 5);
    optLayout->setColumnStretch(0, 0);
    optLayout->setColumnStretch(1, 1);
    optLayout->addWidget(new QLabel("Type:", this), 0, 0);
    optLayout->addWidget(cbShading, 0, 1);
    optLayout->addWidget(new QLabel("Shading:", this), 1, 0);
    optLayout->addWidget(cbFill, 1, 1);
    optLayout->addWidget(ckbDrawMesh, 2, 0);
    optLayout->addWidget(cpMeshColor, 2, 1, Qt::AlignRight);
    optLayout->addWidget(gbMaterial, 3, 0, 1, 2);
    optLayout->addWidget(new QLabel("Ambient color:", this), 4, 0);
    optLayout->addWidget(cpAmbient, 4, 1, Qt::AlignRight);
    optLayout->addWidget(new QLabel("Diffuse color:", this), 5, 0);
    optLayout->addWidget(cpDiffuse, 5, 1, Qt::AlignRight);
    optLayout->addWidget(new QLabel("Specular color:", this), 6, 0);
    optLayout->addWidget(cpSpecular, 6, 1, Qt::AlignRight);
    optLayout->addWidget(new QLabel("Specular power:", this), 7, 0);
    optLayout->addWidget(sbSP, 7, 1, Qt::AlignRight);
    optLayout->addWidget(gbLight, 8, 0, 1, 2);
    optLayout->addWidget(new QLabel("Light color:", this), 9, 0);
    optLayout->addWidget(cpLight, 9, 1, Qt::AlignRight);
    optLayout->addWidget(new QLabel("Light power:", this), 10, 0);
    optLayout->addWidget(sbLP, 10, 1, Qt::AlignRight);
    optLayout->addWidget(pwLightPos, 11, 0, 1, 2);
    optLayout->addWidget(pwLightDir, 12, 0, 1, 2);
    optLayout->addWidget(new QLabel("Spot angle:", this), 13, 0);
    optLayout->addWidget(sbSA, 13, 1, Qt::AlignRight);
    optLayout->addWidget(cbLI, 14, 0, 1, 2);
    optLayout->addWidget(new QLabel("Spot exponent:", this), 15, 0);
    optLayout->addWidget(sbLE, 15, 1, Qt::AlignRight);
    optLayout->setRowStretch(16, 1);
    gbOptions->setLayout(optLayout);

    QWidget *w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);
    layout->addWidget(viewer, 1);
    layout->addWidget(gbOptions, 0);
    w->setLayout(layout);

    this->setCentralWidget(w);
    this->setWindowTitle("CG Task 3");
}

void MainWindow::showModel(bool status) {
    if(!status) QMessageBox::critical(this, "CG Task 3", "Unable to load model");
    else if(model->status() && lightModel->status()) {
        viewer->setAmbientColor(cpAmbient->getColorF());
        viewer->setDiffuseColor(cpDiffuse->getColorF());
        viewer->setSpecularColor(cpSpecular->getColorF());
        viewer->setLightColor(cpLight->getColorF());
        viewer->setLightCutoff(sbSA->value());
        viewer->setModel(model);
        viewer->setLighModel(lightModel);
    }
}

void MainWindow::setLightPosition(QVector3D lpos) {
    viewer->setLightPosition(lpos);
    viewer->setLightDirection(pwLightDir->getValue(), pwLightDir->getValue() - lpos);
}

void MainWindow::setLightDirection(QVector3D point) {
    viewer->setLightDirection(point, point - pwLightPos->getValue());
}

//----------------------------------------------------------------------------

#define CREATE_LIGHT_POS_SB(sb, min, max, val) { \
    sb = new QDoubleSpinBox(this); \
    sb->setRange(min, max); \
    sb->setValue(val); \
    connect(sb, SIGNAL(valueChanged(double)), this, SLOT(emitValue())); \
    }

PositionWidget::PositionWidget(double min, double max, double ival, const QString &name, QWidget *parent) : QWidget(parent) {
    CREATE_LIGHT_POS_SB(sbLX, min, max, ival);
    CREATE_LIGHT_POS_SB(sbLY, min, max, ival);
    CREATE_LIGHT_POS_SB(sbLZ, min, max, ival);
    QGridLayout *lpLayout = new QGridLayout();
//    lpLayout->addWidget(new QLabel(name, this), 0, 0, 1, 6);
//    lpLayout->addWidget(new QLabel("X:", this), 1, 0);
//    lpLayout->addWidget(sbLX, 1, 1, Qt::AlignRight);
//    lpLayout->addWidget(new QLabel("Y:", this), 1, 2);
//    lpLayout->addWidget(sbLY, 1, 3, Qt::AlignRight);
//    lpLayout->addWidget(new QLabel("Z:", this), 1, 4);
//    lpLayout->addWidget(sbLZ, 1, 5, Qt::AlignRight);
    lpLayout->addWidget(new QLabel(name, this), 0, 0);
    lpLayout->addWidget(new QLabel("X:", this), 0, 1, Qt::AlignRight);
    lpLayout->addWidget(sbLX, 0, 2, Qt::AlignRight);
    lpLayout->addWidget(new QLabel("Y:", this), 1, 1, Qt::AlignRight);
    lpLayout->addWidget(sbLY, 1, 2, Qt::AlignRight);
    lpLayout->addWidget(new QLabel("Z:", this), 2, 1, Qt::AlignRight);
    lpLayout->addWidget(sbLZ, 2, 2, Qt::AlignRight);
    lpLayout->setContentsMargins(0, 0, 0, 0);
    lpLayout->setSpacing(5);
    this->setLayout(lpLayout);
}

QVector3D PositionWidget::getValue() const {
    return QVector3D(sbLX->value(), sbLY->value(), sbLZ->value());
}

void PositionWidget::setSingleStep(double val) {
    sbLX->setSingleStep(val);
    sbLY->setSingleStep(val);
    sbLZ->setSingleStep(val);
}

void PositionWidget::emitValue() {
    emit valueChanged(getValue());
}
