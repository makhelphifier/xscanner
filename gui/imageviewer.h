#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer> // <-- 添加此行

class QGraphicsRectItem;

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    void loadImage(const QString &filePath);
    QGraphicsPixmapItem *pixmapItem() const { return m_pixmapItem; }

public slots:
    void resetView();
    void setScale(qreal scale);
    void updatePixmap(const QPixmap &pixmap);
    void setImage(const QImage &image); // <-- 添加此行
signals:
    void scaleChanged(qreal scale);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    // void mouseMoveEvent(QMouseEvent *event) override;


private:
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;
    qreal m_initialScale;
    QGraphicsRectItem *m_borderItem;
    void fitToView();
};

#endif // IMAGEVIEWER_H
