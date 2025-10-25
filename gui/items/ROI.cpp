
#include "roi.h"
#include "handle.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QApplication> // 用于获取键盘修饰键
#include <QtMath>       // 用于qPow
#include <QGraphicsScene> // stateChanged中需要用到
#include <QTransform>


/**
 * @brief ROI 类的构造函数
 *
 * 初始化ROI的位置、大小、画笔等默认属性，并设置图形项的标志位。
 *
 * @param pos 初始位置 (在父坐标系中)
 * @param size 初始尺寸 (宽、高)
 * @param parent 父图形项
 */
ROI::ROI(const QPointF& pos, const QSizeF& size, QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    // 1. 初始化状态
    m_state.pos = pos;
    m_state.size = size;
    m_state.angle = 0.0;

    // 2. 设置图形项在场景中的实际位置、大小和角度
    // QGraphicsItem::setPos() 用于设置项在父坐标系中的位置
    QGraphicsObject::setPos(m_state.pos);
    // QGraphicsObject::setTransformOriginPoint() 设置旋转和缩放的中心
    // 我们将中心点设置为ROI的几何中心
    setTransformOriginPoint(m_state.size.width() / 2.0, m_state.size.height() / 2.0);


    // 3. 初始化外观
    // 设置默认画笔颜色，与Python版本类似
    m_pen = QPen(QColor(255, 255, 255));
    m_hoverPen = QPen(QColor(255, 255, 0));
    m_handlePen = QPen(QColor(150, 255, 255));
    m_handleHoverPen = QPen(QColor(255, 255, 0));

    m_currentPen = m_pen; // 初始当前画笔为默认画笔

    // 4. 设置图形项的标志位
    //    - ItemIsSelectable: 使其可被选中
    //    - ItemSendsGeometryChanges: 当项的位置、变换等改变时发送通知
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    // 启用鼠标悬停事件
    setAcceptHoverEvents(true);
}

/**
 * @brief 返回ROI的边界矩形（在局部坐标系中）
 *
 * 这是Qt图形框架要求必须实现的纯虚函数。它定义了需要重绘的区域。
 * 我们的局部坐标系原点(0,0)就是ROI的左上角。
 *
 * @return QRectF 边界矩形
 */
QRectF ROI::boundingRect() const
{
    // normalized()确保矩形的宽和高是正数
    return QRectF(0, 0, m_state.size.width(), m_state.size.height()).normalized();
}

/**
 * @brief 绘制ROI
 *
 * 这是Qt图形框架要求必须实现的纯虚函数。
 * 基类默认绘制一个矩形。子类可以重写此函数来绘制不同的形状（如椭圆）。
 *
 * @param painter QPainter指针，用于绘图
 * @param option 提供了样式信息，如项的状态（是否被选中）
 * @param widget 绘制所在的窗口部件，一般为nullptr
 */
void ROI::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Q_UNUSED宏用于避免编译器产生“未使用参数”的警告
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(m_currentPen);

    // 我们在局部坐标系 (0,0) 到 (width, height) 的范围内绘制一个矩形
    painter->drawRect(boundingRect());
}




/**
 * @brief 设置ROI在父坐标系中的位置
 * @param pos 新的位置
 * @param update 如果为true，则立即处理状态变更（默认）
 * @param finish 如果为true，则认为此次变更是最终状态，会发射 regionChangeFinished 信号
 */
void ROI::setPos(const QPointF& pos, bool update, bool finish)
{
    if (m_state.pos == pos)
        return;

    m_state.pos = pos;
    QGraphicsObject::setPos(m_state.pos); // 更新QGraphicsItem的实际位置

    if (update) {
        stateChanged(finish);
    }
}

/**
 * @brief 设置ROI的尺寸
 * @param size 新的尺寸
 * @param update 如果为true，则立即处理状态变更（默认）
 * @param finish 如果为true，则认为此次变更是最终状态，会发射 regionChangeFinished 信号
 */
void ROI::setSize(const QSizeF& size, bool update, bool finish)
{
    if (m_state.size == size)
        return;

    // 在改变边界框之前必须调用此函数，这是Qt的要求
    prepareGeometryChange();

    m_state.size = size;
    // 尺寸变化后，旋转中心也需要更新
    setTransformOriginPoint(m_state.size.width() / 2.0, m_state.size.height() / 2.0);

    if (update) {
        stateChanged(finish);
    }
}

/**
 * @brief 设置ROI的旋转角度
 * @param angle 新的角度（单位：度）
 * @param update 如果为true，则立即处理状态变更（默认）
 * @param finish 如果为true，则认为此次变更是最终状态，会发射 regionChangeFinished 信号
 */
void ROI::setAngle(qreal angle, bool update, bool finish)
{
    if (m_state.angle == angle)
        return;

    m_state.angle = angle;
    QGraphicsObject::setRotation(m_state.angle); // 更新QGraphicsItem的实际角度

    if (update) {
        stateChanged(finish);
    }
}

/**
 * @brief 处理状态变更的核心函数
 *
 * 当ROI的任何状态（位置、大小、角度）改变后，此函数被调用来：
 * 1. 更新句柄位置
 * 2. 发射 regionChanged 信号
 * 3. （可选）发射 regionChangeFinished 信号
 *
 * @param finish 如果为true，则发射 regionChangeFinished 信号
 */
void ROI::stateChanged(bool finish)
{
    // 告知场景边界框可能已改变
    if (scene()) {
        scene()->update(mapToScene(boundingRect()).boundingRect());
    }

    // 更新所有句柄的位置（此函数将在下一步实现）
    updateHandles();

    // 触发重绘
    QGraphicsObject::update();

    // 发射信号，通知外部ROI已改变
    emit regionChanged(this);

    if (finish) {
        emit regionChangeFinished(this);
    }
}

/**
 * @brief 根据当前ROI的状态（主要是尺寸），更新所有句柄的局部位置。
 *
 * 每次ROI尺寸变化时，此函数会被 stateChanged() 调用，
 * 确保句柄始终保持在其设定的相对位置上（例如，右下角、中点等）。
 */
void ROI::updateHandles()
{
    // 遍历m_handles列表中的每一个HandleInfo
    for (const auto& info : m_handles) {
        if (info.item) {
            // 计算句柄在ROI局部坐标系中的新位置
            // 相对位置 (0-1) * 当前ROI尺寸 = 绝对局部位置
            QPointF newPos;
            newPos.setX(info.pos.x() * m_state.size.width());
            newPos.setY(info.pos.y() * m_state.size.height());

            // 设置句柄的位置
            info.item->setPos(newPos);
        }
    }
}


/**
 * @brief [私有] 添加一个通用句柄的核心实现
 * @param info 包含句柄所有信息的结构体
 * @return 创建或添加的句柄指针
 */
Handle* ROI::addHandle(HandleInfo info)
{
    // 如果没有提供Handle实例，就创建一个新的
    if (!info.item) {
        // 重要：将 this (ROI) 作为新Handle的父项。
        // 这确保了Handle在ROI的局部坐标系中，并且会被Qt的内存管理机制自动销毁。
        info.item = new Handle(info.type, this);
    }

    // 将此ROI注册到Handle中，让Handle知道它属于哪个ROI
    info.item->connectROI(this);

    // 设置句柄的外观 (未来可以在Handle类中实现更复杂的逻辑)
    info.item->setPen(m_handlePen);
    info.item->setHoverPen(m_handleHoverPen);

    // 将句柄信息添加到列表中
    m_handles.append(info);

    // 确保句柄绘制在ROI的上方
    info.item->setZValue(this->zValue() + 1);

    // 更新一次句柄位置并触发状态变更信号
    stateChanged(true);

    return info.item;
}


/**
 * @brief 添加一个缩放句柄
 * @param pos 句柄在ROI中的相对位置 (0-1)
 * @param center 缩放操作的相对中心点 (0-1)
 * @param name 句柄的可选名称
 * @return 创建的句柄指针
 */
Handle* ROI::addScaleHandle(const QPointF& pos, const QPointF& center, const QString& name)
{
    HandleInfo info;
    info.type = HandleType::Scale;
    info.pos = pos;
    info.center = center;
    // info.name = name; // 可以在HandleInfo中添加name成员

    return addHandle(info);
}

/**
 * @brief 添加一个旋转句柄
 * @param pos 句柄在ROI中的相对位置 (0-1)
 * @param center 旋转操作的相对中心点 (0-1)
 * @param name 句柄的可选名称
 * @return 创建的句柄指针
 */
Handle* ROI::addRotateHandle(const QPointF& pos, const QPointF& center, const QString& name)
{
    HandleInfo info;
    info.type = HandleType::Rotate;
    info.pos = pos;
    info.center = center;

    return addHandle(info);
}


/**
 * @brief 鼠标进入事件
 */
void ROI::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_mouseHovering = true;
    m_currentPen = m_hoverPen;
    emit hoverEvent(this);
    update(); // 触发重绘以更新颜色
}

/**
 * @brief 鼠标离开事件
 */
void ROI::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_mouseHovering = false;
    m_currentPen = m_pen;
    update(); // 触发重绘以恢复颜色
}

/**
 * @brief 鼠标按下事件
 */
void ROI::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    m_dragMode = DragMode::None;
    const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

    if (m_movable && modifiers == Qt::NoModifier) {
        m_dragMode = DragMode::Translate;
    } else if (m_rotatable && modifiers == Qt::AltModifier) {
        m_dragMode = DragMode::Rotate;
    } else if (m_resizable && modifiers == Qt::ShiftModifier) {
        m_dragMode = DragMode::Scale;
    }

    if (m_dragMode != DragMode::None) {
        m_isMoving = true;
        m_dragStartPos = event->scenePos();
        m_preMoveState = m_state; // 保存开始前的状态
        emit regionChangeStarted(this);
        event->accept();
    } else {
        event->ignore();
    }
}

/**
 * @brief 鼠标移动事件
 */
void ROI::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (!m_isMoving || m_dragMode == DragMode::None) {
        event->ignore();
        return;
    }

    QPointF currentPos = event->scenePos();
    QPointF delta = currentPos - m_dragStartPos;

    switch (m_dragMode) {
    case DragMode::Translate: {
        // 新位置 = 初始位置 + 鼠标移动的偏移量
        QPointF newPos = m_preMoveState.pos + delta;
        setPos(newPos, true, false); // 连续更新，但不“完成”
        break;
    }
    case DragMode::Rotate: {
        const qreal rotateSpeed = 0.5;
        // 新角度 = 初始角度 - 鼠标水平移动的距离
        qreal newAngle = m_preMoveState.angle - delta.x() * rotateSpeed;
        setAngle(newAngle, true, false);
        break;
    }
    case DragMode::Scale: {
        // 使用指数函数实现更平滑的缩放
        // 向上拖动缩小，向下拖动放大
        const qreal scaleSpeed = -0.01;
        qreal scaleFactor = qPow(2.0, delta.y() * scaleSpeed);
        QSizeF newSize = m_preMoveState.size * scaleFactor;
        setSize(newSize, true, false);
        break;
    }
    default:
        break;
    }
    event->accept();
}

/**
 * @brief 鼠标释放事件
 */
void ROI::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isMoving && event->button() == Qt::LeftButton) {
        m_isMoving = false;
        m_dragMode = DragMode::None;
        emit regionChangeFinished(this); // 发射“完成”信号
        event->accept();
    } else {
        event->ignore();
    }
}



/**
 * @brief 由Handle类调用，用于在拖动句柄开始时保存ROI的初始状态。
 */
void ROI::handleMoveStarted()
{
    m_preMoveState = m_state; // 保存当前状态
    emit regionChangeStarted(this);
}

/**
 * @brief 根据句柄的移动来更新ROI的状态。这是所有句柄交互的核心。
 * @param handle 正在被移动的句柄
 * @param scenePos 句柄在场景坐标系中的新位置
 * @param finish 拖动是否结束
 */
void ROI::movePoint(Handle* handle, const QPointF& scenePos, bool finish)
{
    // 1. 找到被拖动句柄的信息
    int handleIndex = indexOfHandle(handle);
    if (handleIndex < 0) return;
    const HandleInfo& handleInfo = m_handles[handleIndex];

    // 2. 将句柄的新场景坐标转换为ROI父项的坐标系
    QPointF parentPos = mapFromScene(scenePos);

    // 3. 获取变换前的状态
    ROIState newState = m_preMoveState;

    // 4. 根据句柄类型执行不同的变换逻辑
    switch (handleInfo.type) {
    case HandleType::Scale: {
        // --- 缩放逻辑 ---
        // a. 计算缩放中心点和句柄在“变换前”的局部坐标
        QPointF centerLocal(handleInfo.center.x() * newState.size.width(), handleInfo.center.y() * newState.size.height());
        QPointF handleLocal(handleInfo.pos.x() * newState.size.width(), handleInfo.pos.y() * newState.size.height());

        // b. 将句柄新位置(parentPos)也转换到ROI的局部坐标系（无旋转）
        QTransform t;
        t.rotate(-newState.angle);
        QPointF p1_local = t.map(parentPos - newState.pos);

        // c. 计算从中心点到句柄的原始向量和新向量
        QPointF lp0 = handleLocal - centerLocal;
        QPointF lp1 = p1_local - centerLocal;

        // d. 计算缩放比例
        qreal sx = (qFuzzyIsNull(lp0.x())) ? 1.0 : lp1.x() / lp0.x();
        qreal sy = (qFuzzyIsNull(lp0.y())) ? 1.0 : lp1.y() / lp0.y();

        // e. 计算新尺寸，并修正负值
        QSizeF newSize(newState.size.width() * sx, newState.size.height() * sy);
        if (newSize.width() < 0) newSize.setWidth(-newSize.width());
        if (newSize.height() < 0) newSize.setHeight(-newSize.height());

        // f. 计算位置修正，以保持缩放中心点在场景中位置不变
        QPointF s0_local = centerLocal;
        QPointF s1_local(handleInfo.center.x() * newSize.width(), handleInfo.center.y() * newSize.height());
        QTransform rot;
        rot.rotate(newState.angle);
        QPointF posCorrection = rot.map(s0_local - s1_local);

        // g. 设置新状态
        setPos(newState.pos + posCorrection, false, false);
        setSize(newSize, false, false);
        break;
    }

    case HandleType::Rotate: {
        // --- 旋转逻辑 ---
        // a. 计算旋转中心点和句柄在“变换前”的局部坐标
        QPointF centerLocal(handleInfo.center.x() * newState.size.width(), handleInfo.center.y() * newState.size.height());
        QPointF handleLocal(handleInfo.pos.x() * newState.size.width(), handleInfo.pos.y() * newState.size.height());

        // b. 将句柄新位置(parentPos)也转换到ROI的局部坐标系（无旋转）
        QTransform t;
        t.rotate(-newState.angle);
        QPointF p1_local = t.map(parentPos - newState.pos);

        // c. 计算从中心点到句柄的原始向量和新向量
        QPointF lp0 = handleLocal - centerLocal;
        QPointF lp1 = p1_local - centerLocal;

        // d. 使用atan2计算两个向量的角度，并求出角度差
        qreal angle0 = atan2(lp0.y(), lp0.x());
        qreal angle1 = atan2(lp1.y(), lp1.x());
        qreal angleDelta = angle1 - angle0; // 弧度

        // e. 计算ROI的新角度
        qreal newAngle = newState.angle + qRadiansToDegrees(angleDelta);

        // f. 计算位置修正，以保持旋转中心点在场景中位置不变
        QTransform t_old_rot;
        t_old_rot.rotate(newState.angle);
        QTransform t_new_rot;
        t_new_rot.rotate(newAngle);

        QPointF posCorrection = t_old_rot.map(centerLocal) - t_new_rot.map(centerLocal);

        // g. 设置新状态
        setPos(newState.pos + posCorrection, false, false);
        setAngle(newAngle, false, false);
        break;
    }

    default:
        break;
    }

    // 5. 统一应用状态变更
    stateChanged(finish);
}

/**
 * @brief [私有] 获取句柄在m_handles列表中的索引
 * @param handle 要查找的句柄指针
 * @return 索引值，如果未找到则返回-1
 */
int ROI::indexOfHandle(Handle* handle) const
{
    for (int i = 0; i < m_handles.size(); ++i) {
        if (m_handles[i].item == handle) {
            return i;
        }
    }
    return -1;
}


