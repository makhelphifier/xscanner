// gui/toprightinfowidget.h

#ifndef TOPRIGHTINFOWIDGET_H
#define TOPRIGHTINFOWIDGET_H

#include <QWidget>

class QCheckBox;
class QLabel;
class QDoubleSpinBox;
class QSlider;


class TopRightInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TopRightInfoWidget(QWidget *parent = nullptr);

public slots:
    void setScale(qreal scale);
    void setWindowLevelText(const QString &text);
    void uncheckAutoWindowing();
    void setWindowValue(int value); // <-- 添加此行
    void setLevelValue(int value);  // <-- 添加此行
signals:
    void scaleEdited(double scale);
    void autoWindowingToggled(bool checked);
    void windowChanged(int value); // <-- 添加此行
    void levelChanged(int value);  // <-- 添加此行

private:
    QCheckBox *autoWindowingCheckBox;
    QLabel *windowLevelLabel;
    QDoubleSpinBox *scaleSpinBox;
    QSlider *windowSlider; // <-- 添加此行
    QSlider *levelSlider;  // <-- 添加此行
};

#endif // TOPRIGHTINFOWIDGET_H
