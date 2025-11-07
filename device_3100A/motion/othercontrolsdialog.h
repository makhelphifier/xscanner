#ifndef OTHERCONTROLSDIALOG_H
#define OTHERCONTROLSDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QMouseEvent>
namespace Ui {
class OtherControlsDialog;
}

class OtherControlsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OtherControlsDialog(QWidget *parent = nullptr);
    ~OtherControlsDialog();

signals:
    void sig_resetPos(int value);
    void sig_enable(int index,bool status);
    void sig_stop(int value);
    void sig_clearSts(int value);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

public slots:
    void on_enableChanged(int index, bool status);

private slots:
    void on_btn_close_clicked();

private:
    QPoint m_pos;
    bool m_lMousePress = false;

    void connections();

    Ui::OtherControlsDialog *ui;

    QVector<bool> enableStatus;
};

#endif // OTHERCONTROLSDIALOG_H
