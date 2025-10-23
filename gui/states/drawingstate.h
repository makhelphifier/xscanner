#ifndef DRAWINGSTATE_H
#define DRAWINGSTATE_H

#include <QObject>

class QMouseEvent;
class ImageViewer;

class DrawingState : public QObject
{
    Q_OBJECT

public:
    explicit DrawingState(ImageViewer* viewer, QObject *parent = nullptr);
    virtual ~DrawingState() = default;

    virtual void mousePressEvent(QMouseEvent *event) = 0;
    virtual void mouseMoveEvent(QMouseEvent *event) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *event) = 0;

protected:
    ImageViewer* m_viewer;
};

#endif // DRAWINGSTATE_H
