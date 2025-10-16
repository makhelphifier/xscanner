#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    void loadImage(const QString &filePath);
    QGraphicsPixmapItem *pixmapItem() const { return m_pixmapItem; }

public slots:
    void resetView();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    qreal m_initialScale;

    void fitToView();
};

#endif // IMAGEVIEWER_H
