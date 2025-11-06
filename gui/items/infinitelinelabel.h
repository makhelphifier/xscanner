#ifndef INFINITELINELABEL_H
#define INFINITELINELABEL_H

#include <QGraphicsTextItem>
#include <QObject>

class InfiniteLineItem; // 前向声明

/**
 * @brief 一个附加到 InfiniteLineItem 上的文本标签。
 *
 * 它作为 InfiniteLineItem 的子项，自动随其移动和旋转。
 * 它连接到父项的信号，以在父项移动或视图更改时
 * 更新自己的文本内容和在父项上的相对位置。
 */
class InfiniteLineLabel : public QGraphicsTextItem
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 必须是一个 InfiniteLineItem
     */
    explicit InfiniteLineLabel(InfiniteLineItem* parent);

public slots:
    /**
     * @brief 槽：当父线条的值改变时，更新显示的文本。
     * @param value 新的线条值 (x 或 y)。
     */
    void updateText(qreal value);

    /**
     * @brief 槽：当视图（平移/缩放）改变时，更新标签的相对位置。
     *
     * 这确保标签始终位于视图的可见区域（例如，靠近左上角）。
     */
    void onViewChanged();

    /**
     * @brief 当线条角度改变时，更新文本格式（例如 "X:" vs "Y:"）。
     * @param angle 新的角度 (0 或 90)。
     */
    void setFormat(qreal angle);

private:
    InfiniteLineItem* m_line; // 指向父项（线条）
    QString m_format;         // 文本格式 (例如 "X: %1" 或 "Y: %1")
};

#endif // INFINITELINELABEL_H
