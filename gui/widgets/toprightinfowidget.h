// gui/widgets/toprightinfowidget.h

#ifndef TOPRIGHTINFOWIDGET_H
#define TOPRIGHTINFOWIDGET_H

#include <QWidget>

class QCheckBox;
class QLabel;
class QDoubleSpinBox;

class TopRightInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TopRightInfoWidget(QWidget *parent = nullptr);

public slots:
    void setScale(qreal scale);
    void setWindowLevelText(const QString &text);
    void uncheckAutoWindowing();
    void setWindowValue(double value);
    void setLevelValue(double value);
    void setWindowRange(double min, double max);
    void setLevelRange(double min, double max);
    void setAutoWindowingChecked(bool checked);
    void checkAutoWindowing();

signals:
    void scaleEdited(double scale);
    void autoWindowingToggled(bool checked);
    void windowChanged(double value);
    void levelChanged(double value);

private:
    QCheckBox *autoWindowingCheckBox;
    QLabel *windowLevelLabel;
    QDoubleSpinBox *scaleSpinBox;
    QDoubleSpinBox *windowSpinBox;
    QDoubleSpinBox *levelSpinBox;
};

#endif // TOPRIGHTINFOWIDGET_H
