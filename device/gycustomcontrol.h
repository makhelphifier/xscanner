#ifndef GYCUSTOMCONTROL_H
#define GYCUSTOMCONTROL_H

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPen>
#include <QPainter>

class GyHorizontalLine : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    Q_PROPERTY(int lineHeight READ lineHeight WRITE setLineHeight NOTIFY lineHeightChanged)
    Q_PROPERTY(Qt::PenStyle lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)
    Q_PROPERTY(bool useGradient READ useGradient WRITE setUseGradient NOTIFY useGradientChanged)
    Q_PROPERTY(QColor gradientStartColor READ gradientStartColor WRITE setGradientStartColor NOTIFY gradientStartColorChanged)
    Q_PROPERTY(QColor gradientEndColor READ gradientEndColor WRITE setGradientEndColor NOTIFY gradientEndColorChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

public:
    explicit GyHorizontalLine(QWidget *parent = nullptr);

    QColor lineColor() const { return m_lineColor; }
    int lineHeight() const { return m_lineHeight; }
    Qt::PenStyle lineStyle() const { return m_lineStyle; }
    bool useGradient() const { return m_useGradient; }
    QColor gradientStartColor() const { return m_gradientStartColor; }
    QColor gradientEndColor() const { return m_gradientEndColor; }
    Qt::Alignment alignment() const { return m_alignment; }

public slots:
    void setLineColor(const QColor &color);

    void setLineHeight(int height);

    void setLineStyle(Qt::PenStyle style);

    void setUseGradient(bool use);

    void setGradientStartColor(const QColor &color);

    void setGradientEndColor(const QColor &color);

    void setAlignment(Qt::Alignment alignment);

signals:
    void lineColorChanged(const QColor &color);
    void lineHeightChanged(int height);
    void lineStyleChanged(Qt::PenStyle style);
    void useGradientChanged(bool use);
    void gradientStartColorChanged(const QColor &color);
    void gradientEndColorChanged(const QColor &color);
    void alignmentChanged(Qt::Alignment alignment);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int calculateYPosition() const;
    void drawSolidLine(QPainter &painter, int yPos);
    void drawGradientLine(QPainter &painter, int yPos);

private:
    QColor m_lineColor;
    int m_lineHeight;
    Qt::PenStyle m_lineStyle;
    bool m_useGradient;
    QColor m_gradientStartColor;
    QColor m_gradientEndColor;
    Qt::Alignment m_alignment;
};


class GyIntSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    GyIntSpinBox(QWidget* parent=nullptr);
    ~GyIntSpinBox(){}

public:
    QLineEdit *LineEdit() const;

};

class GyDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    GyDoubleSpinBox(QWidget* parent=nullptr);
    ~GyDoubleSpinBox(){}

public:
    QLineEdit *LineEdit() const;

};

#endif // GYCUSTOMCONTROL_H
