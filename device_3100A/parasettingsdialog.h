#ifndef PARASETTINGSDIALOG_H
#define PARASETTINGSDIALOG_H

#include <QDialog>
#include <QMouseEvent>

namespace Ui {
class ParaSettingsDialog;
}

class ParaSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParaSettingsDialog(QWidget *parent = nullptr);
    ~ParaSettingsDialog();

    void resetWindow(int index);
    void setDdSpeed(QString value);
    void setAbsSpeed(QString value);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
signals:
    void sig_saveSpeed(int index,QString ddSpeed, QString absSpeed);
private slots:
    void on_btn_close_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    QPoint m_pos;
    bool m_lMousePress = false;
    Ui::ParaSettingsDialog *ui;
    int curIndex;
};

#endif // PARASETTINGSDIALOG_H
