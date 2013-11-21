#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QLabel>
#include <QVector3D>

class ColorPicker : public QLabel {
    Q_OBJECT

public:
    ColorPicker(QWidget *parent = 0);

    QVector3D getColorF() const;
    QColor getColor() const;
    void setColor(const QColor &c);

    QSize sizeHint() const;

protected:
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void valueChanged(QVector3D);

private:
    QColor color;
};

#endif // COLORPICKER_H
