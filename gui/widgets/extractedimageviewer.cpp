#include "extractedimageviewer.h"
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

ExtractedImageViewer::ExtractedImageViewer(const QImage &image, QWidget *parent)
    : QDialog(parent)
{
    // 设置此属性，确保窗口关闭时自动删除，防止内存泄漏
    setAttribute(Qt::WA_DeleteOnClose);

    m_label = new QLabel;
    m_label->setPixmap(QPixmap::fromImage(image));
    m_label->setScaledContents(false); // 保持原始像素大小

    // 添加滚动区域，以便查看大于窗口的图像
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidget(m_label);
    m_scrollArea->setWidgetResizable(true); // 让标签填充滚动区域

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_scrollArea);
    setLayout(layout);

    // 设置一个合理的默认尺寸和窗口标题
    // 将尺寸限制在屏幕的 80%
    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    int defaultWidth = qMin(image.width() + 20, static_cast<int>(screenSize.width() * 0.8));
    int defaultHeight = qMin(image.height() + 20, static_cast<int>(screenSize.height() * 0.8));

    resize(defaultWidth, defaultHeight);

    setWindowTitle(QString("提取区域 (Extracted Region) - %1x%2")
                       .arg(image.width())
                       .arg(image.height()));
}

ExtractedImageViewer::~ExtractedImageViewer()
{
    qDebug() << "ExtractedImageViewer destroyed.";
}
