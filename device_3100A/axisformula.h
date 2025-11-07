#ifndef AXISFORMULA_H
#define AXISFORMULA_H
#include <QFile>
#include <QTextStream>
#include <QDialog>
#include <QMessageBox>
#include <QTableWidget>
#include <QDebug>

namespace Ui {
class AxisFormula;
}

class AxisFormula : public QDialog
{
    Q_OBJECT

public:
    // 定义结构体
    struct TableRow {
        QString AxisX;
        QString AxisY;

    };
public:
    explicit AxisFormula(QWidget *parent = nullptr);
    ~AxisFormula();

    bool saveTableToCSV(QTableWidget *table, const QString &filePath);
    bool loadTableFromCSV(QTableWidget *table, const QString &filePath);


    QVector<AxisFormula::TableRow>  csvParsing();

    QVector<AxisFormula::TableRow>  readTableWidget();
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

public slots:
    void slotSetAxis(QString,QString);
    void slotSetinterpositionAxis(QString,QString);
signals:
    void sig_getaxis();
    void sig_interpositionAxis();
private:
    Ui::AxisFormula *ui;

    QString filename ="";

    QVector<TableRow> tableData;

};

#endif // AXISFORMULA_H
