#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QActionGroup>
#include "gui/views/imageviewer.h"
#include "gui/widgets/toprightinfowidget.h"
#include <QLabel>
#include "log4qt/level.h"

class TopRightInfoWidget;
class LogWidget;


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

    void onPixelInfoChanged(int x, int y, int value);
    void onWindowLevelChanged(int width, int level);
    void onAutoWindowingToggled(bool enabled);

    void updateScale(qreal scale);
    void onScaleFromWidget(double scale);
    void onLogLevelChanged(Log4Qt::Level level);

private:
    ImageViewer *viewer;
    QAction *openAction;
    QAction *selectAction;
    QAction *lineAction;
    QAction *rectAction;
    QAction *ellipseAction;
    QAction *wlAction;
    QAction *pointAction;
    QAction *hLineAction;
    QAction *vLineAction;
    QAction *angledLineAction;
    QAction *angleAction;
    QAction *polylineAction;
    QAction *freehandAction;
    QAction *textAction;
    QAction *clearAction;
    QActionGroup *toolGroup;
    QLabel *sizeLabel;
    QLabel *infoLabel;
    TopRightInfoWidget *infoWidget;
    LogWidget *m_logWidget;
};

#endif // MAINWINDOW_H
