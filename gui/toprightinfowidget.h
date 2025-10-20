// gui/toprightinfowidget.h

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

signals:
    void scaleEdited(double scale);

private:
    QCheckBox *autoWindowingCheckBox;
    QLabel *windowLevelLabel;
    QDoubleSpinBox *scaleSpinBox;
};

#endif // TOPRIGHTINFOWIDGET_H
