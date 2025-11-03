
#include "roi.h"
#include "handle.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QtMath>
#include <QGraphicsScene>
#include <QTransform>
#include <QVariant>
#include <QDebug>
#include "util/logger/logger.h"
#include <QPainter>
#include <QMenu>
#include <QPainterPathStroker>


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
    setFlag(QGraphicsItem::ItemIsMovable);
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
    painter->setBrush(QColor(255, 255, 0, 255)); // 1/255 的透明度
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
    QPointF finalPos = pos;
    // 使用 isStateWithinBounds 检查
    // 只有当 update=true 时才检查 (例如ROI拖动或外部调用)
    // update=false 时 (由 movePoint 调用)，检查将由 movePoint 自己在最后完成
    if (update && m_maxBounds.isValid()) {
        ROIState nextState = m_state;
        nextState.pos = pos;
        if (!isStateWithinBounds(nextState)) {
            return; // 拒绝移动
        }
    }

    if (m_state.pos == finalPos) // 比较最终值
        return;

    m_state.pos = finalPos; // 使用最终值
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
    QSizeF finalSize = size;
    if (update && m_maxBounds.isValid()) {
        ROIState nextState = m_state;
        nextState.size = size;
        if (!isStateWithinBounds(nextState)) {
            return; // 拒绝调整大小
        }
    }

    if (m_state.size == finalSize) // 比较最终值
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
    if (update && m_maxBounds.isValid()) {
        ROIState nextState = m_state;
        nextState.angle = angle;
        if (!isStateWithinBounds(nextState)) {
            return; // 拒绝旋转
        }
    }

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
    // 检查状态是否真的改变
    bool changed = (m_state != m_lastState);

    prepareGeometryChange(); // 准备更新外观和包围盒

    for (const auto& handleInfo : qAsConst(m_handles)) { // 使用 protected m_handles
        if (handleInfo.item) {

            if (handleInfo.type == HandleType::Free) {
                // 自由句柄的 'pos' 存储的是绝对本地坐标
                handleInfo.item->setPos(handleInfo.pos);
            } else {
                // 其他句柄(Scale, Rotate)的 'pos' 是相对(0-1)坐标
                handleInfo.item->setPosInROI(handleInfo.pos, m_state.size);
            }
        }
    }

    if (changed) {
        update(); // 请求重绘ROI
        m_lastState = m_state; // 更新 lastState
        emit regionChanged(this); // 发送 regionChanged 信号
    } else {

        emit regionChanged(this);
    }


    if (finish) {
        stateChangeFinished();
    }
}
void ROI::stateChangeFinished()
{
    emit regionChangeFinished(this); // 发送 regionChangeFinished 信号
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

void ROI::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        qDebug() << "ROI mousePressEvent (Left Button)";
        // 让状态机 (IdleState) 决定如何处理对 ROI 本体的点击
        QPointF clickPosInROICoords = event->pos();

        if (flags() & QGraphicsItem::ItemIsMovable) { // 检查 ROI 是否可移动
            m_isMoving = true;
            m_preMoveState = m_state;
            m_dragStartPos = event->scenePos(); // 记录场景位置
            emit regionChangeStarted(this);
            qDebug() << "ROI started moving itself.";
            event->accept();
        } else {
            event->ignore(); // 不可移动则忽略
        }
    } else {
        // 保留其他按钮（如右键菜单）的处理
        QGraphicsObject::mousePressEvent(event);
    }
}

void ROI::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isMoving && (event->buttons() & Qt::LeftButton)) { // 检查是否由左键触发的移动
        // 这是 ROI 本体拖动的逻辑
        QPointF delta = event->scenePos() - m_dragStartPos;
        QPointF newPos = m_preMoveState.pos + delta;
        //平移吸附逻辑
        bool snap = (QApplication::keyboardModifiers() & Qt::ControlModifier);
        if (snap && m_translateSnapSize > 0) {
            newPos.setX(snapValue(newPos.x(), m_translateSnapSize));
            newPos.setY(snapValue(newPos.y(), m_translateSnapSize));
        }
        // 直接设置位置，但不立即触发 finish 信号
        setPos(newPos, true, false);
        event->accept();
    } else {
        QGraphicsObject::mouseMoveEvent(event); // 其他情况交给基类
    }
}

void ROI::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_isMoving && event->button() == Qt::LeftButton) {
        m_isMoving = false;
        stateChangeFinished(); // 拖动结束，发送信号
        qDebug() << "ROI finished moving itself.";
        event->accept();
    } else {
        QGraphicsObject::mouseReleaseEvent(event);
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
    bool snap = (QApplication::keyboardModifiers() & Qt::ControlModifier);
    // 钳制传入的场景坐标
    QPointF clampedScenePos = scenePos;
    if (m_maxBounds.isValid()) { // (移除了 qFuzzyCompare(m_preMoveState.angle, 0.0))
        QRectF bounds = m_maxBounds.toRectF();
        clampedScenePos.setX(qMax(bounds.left(), qMin(scenePos.x(), bounds.right())));
        clampedScenePos.setY(qMax(bounds.top(), qMin(scenePos.y(), bounds.bottom())));
    }
    log_(QStringLiteral("movePoint: 接收到鼠标场景坐标 scenePos: (%1, %2) [Clamped: (%3, %4)]")
             .arg(scenePos.x())
             .arg(scenePos.y())
             .arg(clampedScenePos.x())
             .arg(clampedScenePos.y()));
    // 1. 找到被拖动句柄的信息
    int handleIndex = indexOfHandle(handle);
    log_(QStringLiteral("movePoint: 开始。 句柄索引: %1, 场景坐标: (%2, %3), 拖动结束: %4")
             .arg(handleIndex)
             .arg(scenePos.x())
             .arg(scenePos.y())
             .arg(finish ? "是" : "否"));

    if (handleIndex < 0) {
        log_(QStringLiteral("movePoint: 错误：未找到句柄索引，提前返回。"));
        return;
    }
    const HandleInfo& handleInfo = m_handles[handleIndex];
    // 3. 获取变换前的状态
    ROIState newState = m_preMoveState;
    log_(QStringLiteral("movePoint: 获取变换前状态：Pos(%1, %2), Size(%3, %4), Angle(%5)")
             .arg(newState.pos.x()).arg(newState.pos.y())
             .arg(newState.size.width()).arg(newState.size.height())
             .arg(newState.angle));
    // 2. 将鼠标场景坐标转换为ROI的本地未变换坐标系（自动处理旋转、中心）
    QPointF origin = QPointF(newState.size.width() / 2.0, newState.size.height() / 2.0);
    QTransform preMove_LocalToScene_Transform;
    preMove_LocalToScene_Transform.translate(newState.pos.x(), newState.pos.y());
    preMove_LocalToScene_Transform.translate(origin.x(), origin.y());
    preMove_LocalToScene_Transform.rotate(newState.angle);
    preMove_LocalToScene_Transform.translate(-origin.x(), -origin.y());

    // 获取其逆变换
    bool invertible = false;
    QTransform preMove_SceneToLocal_Transform = preMove_LocalToScene_Transform.inverted(&invertible);

    QPointF p1_local;
    if (invertible) {
        p1_local = preMove_SceneToLocal_Transform.map(clampedScenePos);
    } else {
        log_(QStringLiteral("movePoint: 警告：变换矩阵不可逆！"));
        p1_local = QPointF(0,0); // 或者其他错误处理
    }

    QPointF anchor_local; // 锚点在 preMoveState 本地坐标系中的位置
    QPointF anchor_scene; // 锚点在 场景 坐标系中的位置 (必须保持不变)

    // 4. 根据句柄类型执行不同的变换逻辑
    switch (handleInfo.type) {
    case HandleType::Free: {
        // log_(QStringLiteral("movePoint: 自由(Free)逻辑开始。"));
        // p1_local 是鼠标在ROI本地坐标系中的新位置
        HandleInfo& info = m_handles[handleIndex]; // 获取可写引用
        info.pos = p1_local; // 更新存储的位置

        // 如果需要吸附
        if (snap) {
            if (m_translateSnapSize > 0) {
                info.pos.setX(snapValue(p1_local.x(), m_translateSnapSize));
                info.pos.setY(snapValue(p1_local.y(), m_translateSnapSize));
            }
        }
        break;
    }
    case HandleType::Scale: {
        log_(QStringLiteral("movePoint: 缩放(Scale)逻辑开始。句柄类型: %1")
                 .arg(static_cast<int>(handleInfo.type)));

        // 1. 获取锚点
        QPointF centerRelPos = handleInfo.center;
        log_(QString(" movePoint--- Center: (%1, %2)").arg(centerRelPos.x()).arg(centerRelPos.y()));
        log_(QStringLiteral("  [Scale] 相对锚点(中心): (%1, %2)")
                 .arg(centerRelPos.x()).arg(centerRelPos.y()));

        // 1a. 计算本地锚点 和 必须保持不变的场景锚点
        anchor_local = QPointF(centerRelPos.x() * newState.size.width(),
                               centerRelPos.y() * newState.size.height());
        // preMove_LocalToScene_Transform 是我们在函数开头计算 p1_local 时得到的
        anchor_scene = preMove_LocalToScene_Transform.map(anchor_local);

        log_(QStringLiteral("  [Scale] 绝对本地锚点 (基于preMoveState): (%1, %2)")
                 .arg(anchor_local.x()).arg(anchor_local.y()));
        log_(QStringLiteral("  [Scale] 绝对场景锚点 (固定点): (%1, %2)")
                 .arg(anchor_scene.x()).arg(anchor_scene.y()));

        // 2. 从固定的锚点和移动的鼠标点创建新的本地矩形
        QRectF newLocalRect;
        // (保留之前的侧边缩放逻辑)
        bool isHorizontalScale = qFuzzyCompare(handleInfo.pos.y(), handleInfo.center.y());
        bool isVerticalScale = qFuzzyCompare(handleInfo.pos.x(), handleInfo.center.x());
        QPointF preMove_TopLeft(0, 0);
        QPointF preMove_BottomRight(newState.size.width(), newState.size.height());

        if (isHorizontalScale && !isVerticalScale) {
            log_(QStringLiteral("  [Scale] 模式: 水平缩放 (Horizontal)"));
            newLocalRect.setCoords(anchor_local.x(), preMove_TopLeft.y(), p1_local.x(), preMove_BottomRight.y());
        } else if (isVerticalScale && !isHorizontalScale) {
            log_(QStringLiteral("  [Scale] 模式: 垂直缩放 (Vertical)"));
            newLocalRect.setCoords(preMove_TopLeft.x(), anchor_local.y(), preMove_BottomRight.x(), p1_local.y());
        } else {
            log_(QStringLiteral("  [Scale] 模式: 对角缩放 (Diagonal)"));
            newLocalRect.setCoords(anchor_local.x(), anchor_local.y(), p1_local.x(), p1_local.y());
        }

        // 3. 归一化
        newLocalRect = newLocalRect.normalized();

        // 4. 从新矩形中获取新的尺寸和新的本地左上角位置
        QSizeF newSize = newLocalRect.size();
        QPointF newLocalTopLeft = newLocalRect.topLeft();
        log_(QStringLiteral("  [Scale] 新尺寸: (%1x%2), 新本地左上角: (%3, %4)")
                 .arg(newSize.width()).arg(newSize.height())
                 .arg(newLocalTopLeft.x()).arg(newLocalTopLeft.y()));

        // 5. 计算锚点在 新*局部坐标系中的位置
        QPointF anchor_in_new_local = anchor_local - newLocalTopLeft;
        log_(QStringLiteral("  [Scale] 锚点在 *新* 局部几何中的位置: (%1, %2)")
                 .arg(anchor_in_new_local.x()).arg(anchor_in_new_local.y()));

        // 6. 构建 *新* 的、纯本地的变换 (不含 pos)
        //    使用 newSize 和 preMoveState 的角度
        QPointF newOrigin = QPointF(newSize.width() / 2.0, newSize.height() / 2.0);
        QTransform T_local_new;
        T_local_new.translate(newOrigin.x(), newOrigin.y());
        T_local_new.rotate(newState.angle); // 角度不变 (因为这是 Scale 操作)
        T_local_new.translate(-newOrigin.x(), -newOrigin.y());

        // 7. 计算锚点在应用 T_local_new 后的位置 (假设 newPos = (0,0))
        QPointF anchor_transformed_local = T_local_new.map(anchor_in_new_local);
        log_(QStringLiteral("  [Scale] 锚点在本地变换后的位置 (假设pos=0): (%1, %2)")
                 .arg(anchor_transformed_local.x()).arg(anchor_transformed_local.y()));

        // 8. 计算所需的新 pos
        // newPos + anchor_transformed_local == anchor_scene
        QPointF finalPos = anchor_scene - anchor_transformed_local;
        log_(QStringLiteral("  [Scale] 最终计算的新 pos: (%1, %2)")
                 .arg(finalPos.x()).arg(finalPos.y()));
        if (snap) {
            if (m_translateSnapSize > 0) {
                finalPos.setX(snapValue(finalPos.x(), m_translateSnapSize));
                finalPos.setY(snapValue(finalPos.y(), m_translateSnapSize));
            }
            if (m_scaleSnapSize > 0) {
                newSize.setWidth(snapValue(newSize.width(), m_scaleSnapSize));
                newSize.setHeight(snapValue(newSize.height(), m_scaleSnapSize));
            }
        }
        // 9. 设置新状态
        setPos(finalPos, false, false);
        setSize(newSize, false, false);

        break;
    }
    case HandleType::Rotate: {
        log_(QStringLiteral("movePoint: 旋转(Rotate)逻辑开始。句柄类型: %1")
                 .arg(static_cast<int>(handleInfo.type)));

        // 1. 计算本地旋转中心 和 场景旋转中心
        anchor_local = QPointF(handleInfo.center.x() * newState.size.width(),
                               handleInfo.center.y() * newState.size.height());
        // preMove_LocalToScene_Transform 是我们在函数开头计算 p1_local 时得到的
        anchor_scene = preMove_LocalToScene_Transform.map(anchor_local);
        log_(QStringLiteral("  [Rotate] 本地旋转中心 (基于preMove): (%1, %2)")
                 .arg(anchor_local.x()).arg(anchor_local.y()));
        log_(QStringLiteral("  [Rotate] 场景旋转中心 (固定点): (%1, %2)")
                 .arg(anchor_scene.x()).arg(anchor_scene.y()));

        // 2. 计算新角度 (这部分原始逻辑是正确的)
        QPointF handleLocal(handleInfo.pos.x() * newState.size.width(),
                            handleInfo.pos.y() * newState.size.height());
        QPointF lp0 = handleLocal - anchor_local; // vector center-to-handle
        QPointF lp1 = p1_local - anchor_local; // vector center-to-mouse
        qreal angle0 = atan2(lp0.y(), lp0.x());
        qreal angle1 = atan2(lp1.y(), lp1.x());
        qreal angleDelta = angle1 - angle0;
        qreal newAngle = newState.angle + qRadiansToDegrees(angleDelta);
        log_(QStringLiteral("  [Rotate] 原始ROI角度: %1, 最终新角度: %2")
                 .arg(newState.angle).arg(newAngle));

        // --- [全新逻辑] ---
        // 3. 计算新 pos
        // 旋转时, 尺寸(newSize) 和 旋转中心(anchor_in_new_local) 在本地坐标系中都保持不变
        QSizeF newSize = newState.size;
        QPointF newOrigin = QPointF(newSize.width() / 2.0, newSize.height() / 2.0);
        QPointF anchor_in_new_local = anchor_local; // 旋转中心在本地的位置不变

        // 4. 构建 *新* 的、纯本地的变换 (不含 pos)
        //    使用 preMoveState 的 newSize 和 *newAngle*
        QTransform T_local_new_rotate;
        T_local_new_rotate.translate(newOrigin.x(), newOrigin.y());
        T_local_new_rotate.rotate(newAngle); // <--- 使用新角度
        T_local_new_rotate.translate(-newOrigin.x(), -newOrigin.y());

        // 5. 计算旋转中心在应用 T_local_new_rotate 后的位置 (假设 newPos = (0,0))
        QPointF anchor_transformed_local_rotate = T_local_new_rotate.map(anchor_in_new_local);
        log_(QStringLiteral("  [Rotate] 旋转中心在本地变换后的位置 (假设pos=0): (%1, %2)")
                 .arg(anchor_transformed_local_rotate.x()).arg(anchor_transformed_local_rotate.y()));

        // 6. 计算所需的新 pos
        // newPos + anchor_transformed_local_rotate == anchor_scene
        QPointF finalPos = anchor_scene - anchor_transformed_local_rotate;
        log_(QStringLiteral("  [Rotate] 最终计算的新 pos: (%1, %2)")
                 .arg(finalPos.x()).arg(finalPos.y()));
        if (snap) {
            if (m_translateSnapSize > 0) {
                finalPos.setX(snapValue(finalPos.x(), m_translateSnapSize));
                finalPos.setY(snapValue(finalPos.y(), m_translateSnapSize));
            }
            if (m_rotateSnapAngle > 0) {
                newAngle = snapValue(newAngle, m_rotateSnapAngle);
            }
        }
        // 7. 设置新状态
        setPos(finalPos, false, false);
        // setSize(newSize, false, false); // 尺寸不变
        setAngle(newAngle, false, false);

        break;
    }


    default:
        log_(QStringLiteral("movePoint: 警告：未知的句柄类型 (Type: %1)")
                 .arg(static_cast<int>(handleInfo.type)));
        break;
    }

    if (m_maxBounds.isValid()) {
        if (!isStateWithinBounds(m_state)) {
            // 如果新状态 (m_state) 无效 (即使钳制了鼠标)
            // (例如，缩放或旋转导致某个角超出了边界)
            // 我们将状态回滚到上一个已知的有效状态 (m_lastState)
            m_state = m_lastState;

            // 并且必须将 QGraphicsItem 的属性也回滚
            QGraphicsObject::setPos(m_state.pos);
            setTransformOriginPoint(m_state.size.width() / 2.0, m_state.size.height() / 2.0);
            QGraphicsObject::setRotation(m_state.angle);
        }
    }
    // 5. 统一应用状态变更（更新Handle位置、发射信号）
    // log_(QStringLiteral("movePoint: 逻辑处理完毕，调用 stateChanged(finish=%1)")
    //          .arg(finish ? "是" : "否"));
    stateChanged(finish);

    //打印矩形四个顶点的场景坐标 ===
    // 计算四个本地顶点
    qreal w = m_state.size.width();
    qreal h = m_state.size.height();
    QVector<QPointF> localVertices = {
        QPointF(0, 0),      // 0: 左上
        QPointF(w, 0),      // 1: 右上
        QPointF(w, h),      // 2: 右下
        QPointF(0, h)       // 3: 左下
    };

    log_(QStringLiteral("movePoint: 当前ROI状态 - Pos(%1, %2), Size(%3x%4), Angle(%5°)")
             .arg(m_state.pos.x()).arg(m_state.pos.y())
             .arg(w).arg(h).arg(m_state.angle));

    log_(QStringLiteral("movePoint: 矩形四个顶点的场景坐标 (scenePos):"));
    for (int i = 0; i < 4; ++i) {
        QPointF sceneVertex = mapToScene(localVertices[i]);  // 转换为场景坐标
        QString label = i == 0 ? "左上" : i == 1 ? "右上" : i == 2 ? "右下" : "左下";
        log_(QStringLiteral("  顶点 %1 (%2): (%3, %4)")
                 .arg(i).arg(label).arg(sceneVertex.x()).arg(sceneVertex.y()));
    }
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

// ++ 实现：被状态机调用的方法 ++
void ROI::handleDragStarted(Handle* handle)
{
    Q_UNUSED(handle); // 可能不需要区分是哪个handle
    m_isMoving = true; // 标记开始移动
    m_preMoveState = m_state; // 保存初始状态
    emit regionChangeStarted(this); // 发出开始信号
    qDebug() << "ROI notified: Handle drag started";
}

void ROI::handleDragFinished(Handle* handle)
{
    Q_UNUSED(handle);
    m_isMoving = false; // 标记结束移动
    // 注意：stateChangeFinished 应该在 movePoint(finish=true) 中调用
    // emit regionChangeFinished(this); // 这里不需要重复发射
    qDebug() << "ROI notified: Handle drag finished";
}


/**
 * @brief 返回ROI的精确形状（用于碰撞检测和 itemAt）
 *
 * 重写此方法对于画刷为 Qt::transparent 的项至关重要，
 * 否则 itemAt 无法选中它们。
 *
 * @return QPainterPath 描述此项轮廓的路径
 */
QPainterPath ROI::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}


/**
 * @brief 设置ROI的边界限制
 * @param bounds 边界矩形（场景坐标）
 */
void ROI::setMaxBounds(const QRectF& bounds)
{
    m_maxBounds = bounds;
}
/**
 * @brief 检查一个给定的ROI状态是否完全位于maxBounds之内
 *
 * 通过计算该状态下ROI的4个场景顶点，并检查它们是否都在m_maxBounds内
 *
 * @param state 要检查的ROIState
 * @return true 如果在边界内或未设置边界，false 如果超出边界
 */
bool ROI::isStateWithinBounds(const ROIState& state) const
{
    if (!m_maxBounds.isValid()) {
        return true; // 没有设置边界，始终有效
    }
    QRectF bounds = m_maxBounds.toRectF();

    // 1. 获取该状态的4个本地顶点
    qreal w = state.size.width();
    qreal h = state.size.height();
    QVector<QPointF> localVertices = {
        QPointF(0, 0),      // 左上
        QPointF(w, 0),      // 右上
        QPointF(w, h),      // 右下
        QPointF(0, h)       // 左下
    };

    // 2. 构建此状态的 局部坐标 -> 场景坐标 变换
    QTransform t;
    QPointF origin(w / 2.0, h / 2.0);
    t.translate(state.pos.x(), state.pos.y()); // 3. 平移到最终位置
    t.translate(origin.x(), origin.y());      // 2. 将原点移动到中心
    t.rotate(state.angle);                    // 1. 围绕中心旋转
    t.translate(-origin.x(), -origin.y());    // 0. (重置原点)

    // 3. 检查变换后的每个顶点
    for (const QPointF& localV : localVertices) {
        QPointF sceneV = t.map(localV);
        if (!bounds.contains(sceneV)) {
            // 只要有一个顶点在边界外，此状态就无效
            return false;
        }
    }

    return true; // 所有顶点都在边界内
}
/**
 * @brief 提取ROI覆盖的图像区域
 *
 * 根据ROI的状态（位置、尺寸、角度）从源图像中提取像素。
 * 如果ROI有旋转，将使用QPainter和反向变换来“采样”像素，
 * 并自动处理插值（如果启用）。
 *
 * @param sourceImage 原始的 QImage (例如 ImageViewer::m_originalImage)
 * @param useInterpolation 是否使用双线性插值（推荐在旋转时使用）
 * @return 一个新的 QImage，其尺寸与ROI相同，包含提取的像素
 */
QImage ROI::getArrayRegion(const QImage& sourceImage, bool useInterpolation) const
{
    // 1. 验证输入
    if (sourceImage.isNull() || m_state.size.isEmpty()) {
        return QImage(); // 返回空图像
    }

    // 2. 优化：无旋转的情况 (使用 QImage::copy)
    if (qFuzzyCompare(m_state.angle, 0.0)) {
        QRectF srcRect(m_state.pos, m_state.size);
        // 确保裁剪区域在源图像范围内
        srcRect = srcRect.intersected(sourceImage.rect());
        return sourceImage.copy(srcRect.toRect());
    }

    // 3. 带旋转的情况 (使用 QPainter)
    // 3a. 创建目标图像，尺寸为ROI的尺寸，格式与源相同
    QImage destImage(m_state.size.toSize(), sourceImage.format());
    destImage.fill(Qt::black); // 填充背景（0值）

    // 3b. 创建 QPainter
    QPainter painter(&destImage);

    // 3c. 设置插值（高质量）
    if (useInterpolation) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    }

    // 3d. 设置反向变换
    // 我们要将 "源图像(世界)" 绘制到 "目标图像(视口)" 上
    // 变换必须是ROI正向变换的逆序和反向操作
    QPointF origin(m_state.size.width() / 2.0, m_state.size.height() / 2.0);

    painter.translate(origin.x(), origin.y());       // 4. 移到目标中心
    painter.rotate(-m_state.angle);                  // 3. 反向旋转
    painter.translate(-origin.x(), -origin.y());     // 2. 移回目标左上角
    painter.translate(-m_state.pos.x(), -m_state.pos.y()); // 1. 反向平移

    // 3e. 绘制
    // QPainter 现在会从 destImage 的 (0,0) 点开始
    // 并使用我们设置的变换矩阵去源 QImage 中查找像素
    painter.drawImage(0, 0, sourceImage);
    painter.end();

    return destImage;
}


/**
 * @brief 将一个值吸附到最近的 'snapSize' 倍数上
 * @param value 原始值
 * @param snapSize 吸附的步进值
 * @return 吸附后的值
 */
qreal ROI::snapValue(qreal value, qreal snapSize) const
{
    if (snapSize <= 0) {
        return value;
    }
    return round(value / snapSize) * snapSize;
}


// 吸附设置方法的实现
void ROI::setTranslateSnap(qreal size)
{
    m_translateSnapSize = size;
}

void ROI::setScaleSnap(qreal size)
{
    m_scaleSnapSize = size;
}

void ROI::setRotateSnap(qreal angle)
{
    m_rotateSnapAngle = angle;
}


/**
 * @brief [重写] 处理右键菜单事件
 */
void ROI::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // 创建一个菜单
    QMenu menu;

    // 添加 "提取区域" 动作
    QAction* extractAction = menu.addAction("提取区域 (Extract Region)");

    // 连接动作的 triggered() 信号到 lambda 表达式
    connect(extractAction, &QAction::triggered, this, [this]() {
        // 发射信号，通知外界 (ImageViewer) 来处理
        emit extractRequested(this);
    });

    // (未来可以在此添加 "删除" 动作)
    // QAction* deleteAction = menu.addAction("删除");
    // connect(deleteAction, &QAction::triggered, this, [this](){
    //     emit removeRequested(this);
    // });

    // 在鼠标光标位置显示菜单
    menu.exec(event->screenPos());
}

/**
 * @brief 从ROI中移除一个句柄
 * @param handle 要移除的句柄指针
 */
void ROI::removeHandle(Handle* handle)
{
    if (handle == nullptr) {
        return;
    }

    int index = indexOfHandle(handle);
    if (index == -1) {
        qWarning() << "ROI::removeHandle: Handle not found in this ROI.";
        return;
    }

    // 1. 从 m_handles 列表中移除
    m_handles.removeAt(index);

    // 2. 通知 Handle 它已与此 ROI 断开连接
    handle->disconnectROI(this); //

    // 3. 检查 Handle 是否已成为“孤儿”（不再被任何ROI拥有）
    if (handle->rois().isEmpty()) { //
        // 如果是，则安全地删除它
        // 因为 Handle 的父项是 ROI，删除它会自动将其从场景中移除
        qDebug() << "ROI::removeHandle: Deleting orphaned handle.";
        delete handle;
    }
}


/**
 * @brief 添加一个自由句柄（仅移动，不影响ROI）
 * @param pos 句柄在ROI局部坐标系中的位置 (与size无关)
 * @param name 句柄的可选名称
 * @return 创建的句柄指针
 */
Handle* ROI::addFreeHandle(const QPointF& pos, const QString& name)
{
    HandleInfo info;
    info.type = HandleType::Free;
    info.pos = pos; // 存储绝对局部坐标
    // info.name = name; // C++ HandleInfo 结构体中没有 name 成员
    return addHandle(info);
}




