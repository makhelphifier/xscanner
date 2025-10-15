#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QGraphicsView>
#include <QAction>

#include <QMainWindow>
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

private:
    QGraphicsScene *scene;
    QGraphicsView *view;
    QGraphicsPixmapItem *pixmapItem;
    QAction *lineAction;
    QAction *rectAction;    // 新增：用于“矩形”的Action
    QAction *ellipseAction; // 新增：用于“椭圆”的Action

private slots:
    void drawLine();
    void drawRect();    // 新增：响应矩形Action的槽函数
    void drawEllipse(); // 新增：响应椭圆Action的槽函数
};
#endif // MAINWINDOW_H
