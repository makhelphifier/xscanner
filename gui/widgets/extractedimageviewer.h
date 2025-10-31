#ifndef EXTRACTEDIMAGEVIEWER_H
#define EXTRACTEDIMAGEVIEWER_H

#include <QDialog>
#include <QImage>

class QLabel;
class QScrollArea;

/**
 * @brief 一个简单的新窗口（对话框），用于显示提取的 QImage。
 */
class ExtractedImageViewer : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param image 要显示的图像 (应为 8-bit 灰度或彩色)
     * @param parent 父窗口 (通常是 ImageViewer)
     */
    explicit ExtractedImageViewer(const QImage &image, QWidget *parent = nullptr);
    ~ExtractedImageViewer();

private:
    QLabel *m_label;
    QScrollArea *m_scrollArea;
};

#endif // EXTRACTEDIMAGEVIEWER_H
