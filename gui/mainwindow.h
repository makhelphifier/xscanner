#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QActionGroup>
#include "gui/imageviewer.h"
#include "gui/toprightinfowidget.h"
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:

    void openImage();
    void selectMode();

    void onPixelInfoChanged(int x, int y, int value);
    void onWindowLevelChanged(int width, int level);
    void onAutoWindowingToggled(bool enabled);

    void updateScale(qreal scale);
    void onScaleFromWidget(double scale);

private:
    ImageViewer *viewer;
    QAction *openAction;
    QAction *selectAction;
    QAction *lineAction;
    QAction *rectAction;
    QAction *ellipseAction;
    QAction *wlAction;
    QAction *pointAction;
    QActionGroup *toolGroup;
    QLabel *sizeLabel;
    QLabel *infoLabel;
    TopRightInfoWidget *infoWidget;
};

#endif // MAINWINDOW_H
