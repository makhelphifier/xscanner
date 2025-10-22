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
    void setWindowValue(int value);
    void setLevelValue(int value);
    void setWindowRange(int min, int max);
    void setLevelRange(int min, int max);
signals:
    void scaleEdited(double scale);
    void autoWindowingToggled(bool checked);
    void windowChanged(int value);
    void levelChanged(int value);

private:
    QCheckBox *autoWindowingCheckBox;
    QLabel *windowLevelLabel;
    QDoubleSpinBox *scaleSpinBox;
    QSlider *windowSlider;
    QSlider *levelSlider;
};

#endif // TOPRIGHTINFOWIDGET_H
