#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QActionGroup>
#include <QRectF>
#include "gui/views/imageviewer.h"
#include "gui/widgets/toprightinfowidget.h"
#include <QLabel>
#include "log4qt/level.h"

class TopRightInfoWidget;
class LogWidget;
class ImageViewModel;
class HistogramWidget;

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

    void onPixelInfoChanged(int x, int y, double value);
    void onWindowLevelChanged(double width, double level);
    void onAutoWindowingToggled(bool enabled);

    void onImageLoaded(double min, double max, int bits, QRectF imageRect);

    void updateScale(qreal scale);
    void onScaleFromWidget(double scale);
    void onLogLevelChanged(Log4Qt::Level level);

private:
    ImageViewModel *m_imageViewModel;
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
    HistogramWidget *m_histogramWidget;
};

#endif // MAINWINDOW_H
