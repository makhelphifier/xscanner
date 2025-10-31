#include "gycustomcontrol.h"
#include <QPushButton>

GyHorizontalLine::GyHorizontalLine(QWidget *parent)
    : QWidget(parent),
    m_lineColor(Qt::gray),
    m_lineHeight(2),
    m_lineStyle(Qt::SolidLine),
    m_useGradient(false),
    m_gradientStartColor(Qt::blue),
    m_gradientEndColor(Qt::red),
    m_alignment(Qt::AlignVCenter)
{
    setFixedHeight(m_lineHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void GyHorizontalLine::setLineColor(const QColor &color)
{
    if (m_lineColor != color) {
        m_lineColor = color;
        update();
        emit lineColorChanged(color);
    }
}

void GyHorizontalLine::setLineHeight(int height)
{
    if (m_lineHeight != height && height > 0) {
        m_lineHeight = height;
        setFixedHeight(height);
        update();
        emit lineHeightChanged(height);
    }
}

void GyHorizontalLine::setLineStyle(Qt::PenStyle style)
{
    if (m_lineStyle != style) {
        m_lineStyle = style;
        update();
        emit lineStyleChanged(style);
    }
}

void GyHorizontalLine::setUseGradient(bool use)
{
    if (m_useGradient != use) {
        m_useGradient = use;
        update();
        emit useGradientChanged(use);
    }
}

void GyHorizontalLine::setGradientStartColor(const QColor &color)
{
    if (m_gradientStartColor != color) {
        m_gradientStartColor = color;
        if (m_useGradient) update();
        emit gradientStartColorChanged(color);
    }
}

void GyHorizontalLine::setGradientEndColor(const QColor &color)
{
    if (m_gradientEndColor != color) {
        m_gradientEndColor = color;
        if (m_useGradient) update();
        emit gradientEndColorChanged(color);
    }
}

void GyHorizontalLine::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment != alignment) {
        m_alignment = alignment;
        update();
        emit alignmentChanged(alignment);
    }
}


int GyHorizontalLine::calculateYPosition() const
{
    if (m_alignment & Qt::AlignTop) {
        return m_lineHeight / 2;
    } else if (m_alignment & Qt::AlignBottom) {
        return height() - m_lineHeight / 2;
    } else {
        return height() / 2;
    }
}

void GyHorizontalLine::drawSolidLine(QPainter &painter, int yPos)
{
    QPen pen(m_lineColor, m_lineHeight, m_lineStyle);
    painter.setPen(pen);
    painter.drawLine(0, yPos, width(), yPos);
}

void GyHorizontalLine::drawGradientLine(QPainter &painter, int yPos)
{
    QLinearGradient gradient(0, yPos, width(), yPos);
    gradient.setColorAt(0.0, m_gradientStartColor);
    gradient.setColorAt(1.0, m_gradientEndColor);

    QPen pen;
    pen.setBrush(QBrush(gradient));
    pen.setWidth(m_lineHeight);
    pen.setStyle(m_lineStyle);

    painter.setPen(pen);
    painter.drawLine(0, yPos, width(), yPos);
}

void GyHorizontalLine::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int yPos = calculateYPosition();

    if (m_useGradient) {
        drawGradientLine(painter, yPos);
    } else {
        drawSolidLine(painter, yPos);
    }
}

GyIntSpinBox::GyIntSpinBox(QWidget* parent):QSpinBox(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

QLineEdit *GyIntSpinBox::LineEdit() const
{
    return lineEdit();
}


GyDoubleSpinBox::GyDoubleSpinBox(int id , AXIS axisNum, QWidget* parent):QDoubleSpinBox(parent),m_id(id),m_axisNum(axisNum)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setFixedWidth(60);

}

QLineEdit *GyDoubleSpinBox::LineEdit() const
{
    return lineEdit();
}

AXIS GyDoubleSpinBox::Axis()
{
    return m_axisNum;
}

GyPushButton::GyPushButton(AXIS axisNum,QString title, QWidget *parent):QPushButton(title,parent),m_axisNum(axisNum)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setFixedWidth(40);
}

AXIS GyPushButton::Axis()
{
    return m_axisNum;
}
