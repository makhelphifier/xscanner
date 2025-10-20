#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsView>
#include <QAction>
#include <QObject>
#include <QEvent>
#include "gui/imageviewer.h"
#include <QAction>
#include <QMainWindow>
#include <QActionGroup>


class QGraphicsScene;
class QGraphicsPixmapItem;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsPixmapItem *pixmapItem;
    QAction *lineAction;
    QAction *rectAction;
    QAction *ellipseAction;
    QAction *pointAction;
    enum DrawMode {
        Mode_None,
        Mode_Select,
        Mode_Point,
        Mode_Line,
        Mode_Rect,
        Mode_Ellipse
    };
    DrawMode m_currentMode = Mode_None;
    QImage m_image;
    bool m_isPanning = false;
    QPoint m_lastPanPoint;
    ImageViewer *viewer;
    QAction *openAction;
    bool m_isDrawing = false;
    QPointF m_startPoint;
    QGraphicsLineItem *m_previewLine = nullptr;
    QAction *selectAction;
    QActionGroup *toolGroup;

private slots:
    void drawLine();
    void drawRect();
    void drawEllipse();
    void drawPoint();
    void openImage();
    void selectMode();
};
#endif // MAINWINDOW_H
