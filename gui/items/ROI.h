#ifndef ROI_H
#define ROI_H

#include <QGraphicsObject>
#include <QPen>
#include <QList>
#include "handle.h"
class Handle;
class QGraphicsSceneMouseEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

#include <QtMath>

/**
 * @brief 用于保存和恢复ROI的核心状态
 */
// 前向声明
class Handle;

// ++ 定义一个结构体来保存 ROI 的核心状态 ++
struct ROIState {
    QPointF pos;
    QSizeF size;
    qreal angle = 0.0; // 角度（度）

    bool operator!=(const ROIState& other) const {
        return pos != other.pos || size != other.size || !qFuzzyCompare(angle, other.angle);
    }
    bool operator==(const ROIState& other) const {
        return !(*this != other);
    }
};
// 告诉 Qt 如何存储/加载 ROIState，以便 QVariant 可以使用
Q_DECLARE_METATYPE(ROIState)

/**
 * @brief 存储单个句柄的完整信息
 */
struct HandleInfo {
    Handle* item = nullptr;
    HandleType type;
    QPointF pos;      // 句柄在ROI局部坐标系中的相对位置 (0.0-1.0)
    QPointF center;   // 缩放/旋转的中心点 (0.0-1.0)
};


// --- ROI 基类定义 ---

/**
 * @brief 可交互的感兴趣区域（ROI）的基类。
 *
 * ROI 是一个QGraphicsObject，可以被添加到QGraphicsScene中。
 * 它管理着自己的位置、大小和角度，并提供了一系列可拖动的句柄（Handles）
 * 来修改这些状态。它还定义了与用户交互的核心逻辑和信号。
 */
class ROI : public QGraphicsObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param pos ROI的初始位置（左下角）
     * @param size ROI的初始尺寸
     * @param parent 父图形项
     */
    explicit ROI(const QPointF& pos, const QSizeF& size, QGraphicsItem* parent = nullptr);
    virtual ~ROI() override = default;

    // --- 公有接口 (Public API) ---
    // 公共接口供状态机调用
    void handleDragStarted(Handle* handle);
    void handleDragFinished(Handle* handle);
    // 状态访问 (State Accessors)
    QPointF pos() const { return m_state.pos; }
    QSizeF size() const { return m_state.size; }
    qreal angle() const { return m_state.angle; }

    // 状态修改 (State Mutators)
    void setPos(const QPointF& pos, bool update = true, bool finish = true);
    void setSize(const QSizeF& size, bool update = true, bool finish = true);
    void setAngle(qreal angle, bool update = true, bool finish = true);

    // 变换操作 (Transformations)
    void translate(const QPointF& delta, bool snap = false, bool update = true, bool finish = true);
    void scale(qreal factor, const QPointF& center, bool snap = false, bool update = true, bool finish = true);
    void rotate(qreal angleDelta, const QPointF& center, bool snap = false, bool update = true, bool finish = true);

    // 状态保存与恢复
    ROIState saveState() const;
    void setState(const ROIState& state, bool update = true);

    // 外观设置
    void setPen(const QPen& pen);
    void setHoverPen(const QPen& pen);

    // 句柄管理 (Handle Management)
    Handle* addFreeHandle(const QPointF& pos, const QString& name = QString());
    Handle* addScaleHandle(const QPointF& pos, const QPointF& center, const QString& name = QString());
    Handle* addRotateHandle(const QPointF& pos, const QPointF& center, const QString& name = QString());
    Handle* addScaleRotateHandle(const QPointF& pos, const QPointF& center, const QString& name = QString());
    void removeHandle(Handle* handle);
    QList<Handle*> getHandles() const;

    // --- 从 QGraphicsItem 重写 ---
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    // QPainterPath shape() const override;

    enum class DragMode {
        None,
        Translate,
        Rotate,
        Scale
    };

    /**
     * @brief 当句柄开始移动时由Handle类调用
     */
    void handleMoveStarted();

    /**
     * @brief 由句柄（Handle）在被拖动时调用，以更新ROI的状态
     * @param handle 被移动的句柄
     * @param scenePos 句柄在场景坐标系中的新位置
     */
    virtual void movePoint(Handle* handle, const QPointF& scenePos, bool finish = true);


signals:
    /**
     * @brief 当用户开始拖动ROI或其句柄时发射此信号
     */
    void regionChangeStarted(ROI* sender);

    /**
     * @brief 当ROI的位置、大小或角度发生任何改变时发射此信号（包括拖动过程中）
     */
    void regionChanged(ROI* sender);

    /**
     * @brief 当用户完成对ROI的拖动或通过代码完成状态设置时发射此信号
     */
    void regionChangeFinished(ROI* sender);

    /**
     * @brief 当用户点击ROI时发射此信号
     */
    void clicked(ROI* sender, QGraphicsSceneMouseEvent* event);

    /**
     * @brief 当鼠标悬停在ROI上时发射此信号
     */
    void hoverEvent(ROI* sender);

    /**
     * @brief 当用户请求移除ROI时（例如通过右键菜单）发射此信号
     */
    void removeRequested(ROI* sender);
private slots:
    void stateChangeFinished();


protected:
    // --- 事件处理 (Event Handling) ---
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    // void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    // --- 内部方法 (Internal Methods) ---

    /**
     * @brief 由句柄（Handle）在被拖动时调用，以更新ROI的状态
     * @param handle 被移动的句柄
     * @param scenePos 句柄在场景坐标系中的新位置
     */
    // virtual void movePoint(Handle* handle, const QPointF& scenePos, bool finish = true);

    /**
     * @brief 在状态（位置、大小、角度）改变后调用此函数
     * @param finish 如果为true，则发射 regionChangeFinished 信号
     */
    void stateChanged(bool finish = true);

private:
    /**
     * @brief 根据当前状态更新所有句柄的位置
     */
    void updateHandles();

    /**
     * @brief 添加一个通用句柄
     */
    Handle* addHandle(HandleInfo info);

    /**
     * @brief 获取句柄在列表中的索引
     */
    int indexOfHandle(Handle* handle) const;


private:
    // --- 核心数据成员 ---
    ROIState m_state;              // 当前状态
    ROIState m_lastState;   // 上一次发出信号时的状态
    ROIState m_preMoveState;       // 移动前的状态，用于取消操作

    // 外观
    QPen m_pen;
    QPen m_hoverPen;
    QPen m_handlePen;
    QPen m_handleHoverPen;
    QPen m_currentPen;

    // 句柄
    QList<HandleInfo> m_handles;

    // 配置标志
    bool m_movable = true;
    bool m_resizable = true;
    bool m_rotatable = true;
    bool m_removable = false;

    // 交互状态
    bool m_isMoving = false;
    bool m_mouseHovering = false;

    // 拖动状态变量
    DragMode m_dragMode = DragMode::None; // 当前拖动模式
    QPointF m_dragStartPos;               // 拖动开始时鼠标的场景位置

};

#endif // ROI_H
