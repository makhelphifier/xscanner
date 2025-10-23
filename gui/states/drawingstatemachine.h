#ifndef DRAWINGSTATEMACHINE_H
#define DRAWINGSTATEMACHINE_H

#include <QObject>
#include <memory>

class DrawingState;
class ImageViewer;
class QMouseEvent;

class DrawingStateMachine : public QObject
{
    Q_OBJECT

public:
    explicit DrawingStateMachine(ImageViewer* viewer, QObject *parent = nullptr);
    ~DrawingStateMachine();

    void setState(DrawingState* state);
    void handleMousePress(QMouseEvent *event);
    void handleMouseMove(QMouseEvent *event);
    void handleMouseRelease(QMouseEvent *event);

private:
    ImageViewer* m_viewer;
    std::unique_ptr<DrawingState> m_currentState;
};

#endif // DRAWINGSTATEMACHINE_H
