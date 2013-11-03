#include "colorpicker.h"

#include <QColorDialog>
#include <QFontMetrics>
#include <QMouseEvent>

ColorPicker::ColorPicker(QWidget *parent) : QLabel(parent) {
    setFrameStyle(QFrame::Panel);
    setColor(Qt::black);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ColorPicker::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        color = QColorDialog::getColor(color, this);
        if(color.isValid()) {
            setColor(color);
            emit valueChanged(getColorF());
        }
    }
}

QVector3D ColorPicker::getColorF() const {
    return QVector3D(color.redF(), color.greenF(), color.blueF());
}

QColor ColorPicker::getColor() const {
    return color;
}

void ColorPicker::setColor(const QColor &c) {
    color = c;

    QPalette curPalette = palette();
    curPalette.setColor(QPalette::Window, color);
    this->setPalette(curPalette);
}

QSize ColorPicker::sizeHint() const {
    QFontMetrics fm(font());
    int w = fm.width("AAAAA");
    int h = fm.height();
    return QSize(w, h);
}
