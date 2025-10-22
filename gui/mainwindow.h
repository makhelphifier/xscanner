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
class QLabel;
class TopRightInfoWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void applyAndDisplayWl();
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
    QImage m_originalImage;
    bool m_isPanning = false;
    QPoint m_lastPanPoint;
    ImageViewer *viewer;
    QAction *openAction;
    bool m_isDrawing = false;
    QPointF m_startPoint;
    QGraphicsLineItem *m_previewLine = nullptr;
    QAction *selectAction;
    QActionGroup *toolGroup;
    QLabel *sizeLabel;
    QLabel *infoLabel;
    TopRightInfoWidget *infoWidget;
    int m_windowWidth;
    int m_windowLevel;

private slots:
    void drawLine();
    void drawRect();
    void drawEllipse();
    void drawPoint();
    void openImage();
    void selectMode();
    void updateScale(qreal scale);
    void onScaleFromWidget(double scale);
    void onAutoWindowingToggled(bool checked);
    void onWindowChanged(int value);
    void onLevelChanged(int value);
};
#endif // MAINWINDOW_H
