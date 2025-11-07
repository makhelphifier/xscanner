#include "axisformula.h"
#include "ui_axisformula.h"

AxisFormula::AxisFormula(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AxisFormula)
{
    ui->setupUi(this);

    QString appDir = qApp->applicationDirPath();
    filename = QString("%1/axisformula.csv").arg(appDir);;
    loadTableFromCSV(ui->tableWidget,filename);
    readTableWidget();
    csvParsing();
}

AxisFormula::~AxisFormula()
{
    delete ui;
}

void AxisFormula::on_pushButton_clicked()
{
    emit sig_getaxis();
    // // 在表格末尾添加一行
    // int row = ui->tableWidget->rowCount();
    // ui->tableWidget->insertRow(row); // 插入新行
    // // 设置该行的各个单元格内容
    // ui->tableWidget->setItem(row, 0, new QTableWidgetItem("第一列数据"));
    // ui->tableWidget->setItem(row, 1, new QTableWidgetItem("第二列数据"));
    // ui->tableWidget->setItem(row, 2, new QTableWidgetItem("第三列数据"));
}

#include <QMessageBox>
void AxisFormula::on_pushButton_2_clicked()
{
    emit sig_interpositionAxis();

}


void AxisFormula::on_pushButton_3_clicked()
{
    // // 获取当前选中行
    // int currentRow = ui->tableWidget->currentRow();
    // if (currentRow >= 0) {  // 确保有选中行
    //     ui->tableWidget->removeRow(currentRow);
    // } else {
    //     QMessageBox::information(this, "提示", "请先选择要删除的行");
    //     return;
    // }

    // QMessageBox::information(nullptr, "成功", QString("已删除 %1 行").arg(currentRow));
    // 获取所有选中的不重复行号
    QList<int> rowsToDelete;
    for (QTableWidgetItem* item : ui->tableWidget->selectedItems()) {
        if (!rowsToDelete.contains(item->row())) {
            rowsToDelete.append(item->row());
        }
    }

    if (rowsToDelete.isEmpty()) {
        QMessageBox::information(nullptr, "提示", "请先选择要删除的行");
        return;
    }

    // 从大到小排序，确保从下往上删除
    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

    // 禁用更新以提高性能
    ui->tableWidget->setUpdatesEnabled(false);

    // 删除行
    for (int row : rowsToDelete) {
        ui->tableWidget->removeRow(row);
    }

    // 重新启用更新
    ui->tableWidget->setUpdatesEnabled(true);

    QMessageBox::information(nullptr, "成功", QString("已删除 %1 行").arg(rowsToDelete.size()));
}




bool AxisFormula::saveTableToCSV(QTableWidget *table, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);

    // 写入表头
    QStringList headers;
    for (int col = 0; col < table->columnCount(); ++col) {
        headers << table->horizontalHeaderItem(col)->text();
    }
    out << headers.join(",") << "\n";

    // 写入数据
    for (int row = 0; row < table->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem *item = table->item(row, col);
            QString text = item ? item->text() : "";
            // 处理CSV中的特殊字符（如逗号、引号）
            if (text.contains(',') || text.contains('"') || text.contains('\n')) {
                text.replace("\"", "\"\"");
                text = "\"" + text + "\"";
            }
            rowData << text;
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    return true;
}

QVector<AxisFormula::TableRow>   AxisFormula::csvParsing()
{
    QVector<AxisFormula::TableRow> locationAndErr;
    AxisFormula::TableRow table;
    // 打开 CSV 文件
    // QFile file("D:/study/Project/csvparsing/csvParsing/data1.csv");
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    }
    qDebug() << "打开文件";

    QTextStream in(&file);

    // 正则表达式匹配 CSV 行（支持带引号的字段）
    QRegularExpression regex("(\"[^\"]*\"|[^,]+)");

    int row = 0;
    // 逐行读取文件
    while (!in.atEnd()) {
        QString line = in.readLine(); // 读取一行
        // qDebug()<<"line:"<<line;
        QRegularExpressionMatchIterator matches = regex.globalMatch(line);

        QStringList fields;
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString field = match.captured(0).trimmed();
            if (field.startsWith('"') && field.endsWith('"')) {
                field = field.mid(1, field.length() - 2); // 去除引号
            }
            fields.append(field);
        }
        // qDebug()<<"row:"<<row;
        if(row >= 1)
        {
            // 确保至少有两列数据
            if (fields.size() >= 2) {
                QString column1 = fields.at(0); // 第一列数据
                QString column2 = fields.at(1); // 第二列数据
                // qDebug()<<"row:"<<row;
                table.AxisX =column1;
                table.AxisY = column2;
                locationAndErr.append(table);
                // qDebug() << "Column AxisX:" << column1 << "Column AxisY:" << column2;
            }
        }

        row++;
    }

    // 关闭文件
    file.close();

    return locationAndErr;
}


bool AxisFormula::loadTableFromCSV(QTableWidget *table, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    table->clearContents();
    table->setRowCount(0);

    QTextStream in(&file);
    bool firstLine = true;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;

        QStringList fields;
        bool inQuotes = false;
        QString currentField;

        // 解析CSV行（处理带引号和逗号的情况）
        for (int i = 0; i < line.length(); ++i) {
            QChar c = line.at(i);
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                fields << currentField;
                currentField.clear();
            } else {
                currentField += c;
            }
        }
        fields << currentField;

        if (firstLine) {
            // 第一行是表头
            table->setColumnCount(fields.size());
            table->setHorizontalHeaderLabels(fields);
            firstLine = false;
        } else {
            // 数据行
            int row = table->rowCount();
            table->insertRow(row);
            for (int col = 0; col < fields.size(); ++col) {
                QString text = fields.at(col);
                // 移除多余的引号
                if (text.startsWith('"') && text.endsWith('"')) {
                    text = text.mid(1, text.length() - 2);
                    text.replace("\"\"", "\"");
                }
                table->setItem(row, col, new QTableWidgetItem(text));
            }
        }
    }

    file.close();
    return true;
}

QVector<AxisFormula::TableRow>  AxisFormula::readTableWidget()
{

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        TableRow rowData;

        QTableWidgetItem *XItem = ui->tableWidget->item(row, 0);
        rowData.AxisX = XItem ? XItem->text(): "";

        QTableWidgetItem *YItem = ui->tableWidget->item(row, 1);
        rowData.AxisY = YItem ? YItem->text() : "";

        tableData.append(rowData);
    }

    // 使用示例
    // for (const TableRow &row : tableData) {
    //     qDebug() << "readTableWidget X:" << row.AxisX
    //              << "readTableWidget Y:" << row.AxisY;
    // }
    return tableData;
}

void AxisFormula::on_buttonBox_clicked(QAbstractButton *button)
{
    qDebug()<<"button:"<<button->text();

    if(button->text() == "OK")
    {
        saveTableToCSV(ui->tableWidget,filename);
    }
}

void AxisFormula::slotSetAxis(QString axisX, QString axisY)
{
    // 在表格末尾添加一行
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row); // 插入新行
    // 设置该行的各个单元格内容
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(axisX));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(axisY));

}

void AxisFormula::slotSetinterpositionAxis(QString axisX, QString axisY)
{
    if (ui->tableWidget->currentRow() < 0) {

        QMessageBox::information(nullptr, "提示", "请先选中一行");
        // 没有选中行
        return;
    }
    // 在第2行位置插入新行（0-based索引）
    int insertPosition = ui->tableWidget->currentRow()+1 ;
    ui->tableWidget->insertRow(insertPosition);
    ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem(axisX));
    ui->tableWidget->setItem(insertPosition, 1, new QTableWidgetItem(axisY));

    // 设置插入行的数据
    // ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem("插入的行"));
    QMessageBox::information(nullptr, "成功", QString("已插入 %1 行").arg(insertPosition));
}

//根据步长添加N个X轴位置
void AxisFormula::on_pushButton_4_clicked()
{
    if (ui->tableWidget->currentRow() < 0) {

        QMessageBox::information(nullptr, "提示", "请先选中一行");
        // 没有选中行
        return;
    }
    // 在第2行位置插入新行（0-based索引） 选中行的位置、 根据选中行的位置添加x、y轴

    int seleRow = ui->tableWidget->currentRow();

    QTableWidgetItem *XItem = ui->tableWidget->item(seleRow, 0);
    float Xpos = XItem->text().toFloat();
    qDebug()<<"xpos:"<<Xpos;

    QTableWidgetItem *YItem = ui->tableWidget->item(seleRow, 1);
    float Ypos = YItem->text().toFloat();
    qDebug()<<"Ypos:"<<Ypos;


    for(int i = 0;i < ui->XCount->text().toInt();i++)
    {
        int insertPosition = ui->tableWidget->currentRow()+1+i ;
        ui->tableWidget->insertRow(insertPosition);
        float x= Xpos+ui->xStep->text().toFloat()*(i+1);

        qDebug()<<"insertRow:"<<insertPosition<<" x:"<<x<<" Ypos:"<<Ypos;
        ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem(QString::number(x,'f',4)));
        ui->tableWidget->setItem(insertPosition, 1, new QTableWidgetItem(QString::number(Ypos,'f',4)));

    }


    // 设置插入行的数据
    // ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem("插入的行"));
    QMessageBox::information(nullptr, "成功", QString("已插入 %1 行").arg(ui->XCount->text().toInt()));
}

//根据步长添加N个X轴位置
void AxisFormula::on_pushButton_5_clicked()
{
    if (ui->tableWidget->currentRow() < 0) {

        QMessageBox::information(nullptr, "提示", "请先选中一行");
        // 没有选中行
        return;
    }
    // 在第2行位置插入新行（0-based索引） 选中行的位置、 根据选中行的位置添加x、y轴

    int seleRow = ui->tableWidget->currentRow();

    QTableWidgetItem *XItem = ui->tableWidget->item(seleRow, 0);
    float Xpos = XItem->text().toFloat();
    qDebug()<<"xpos:"<<Xpos;

    QTableWidgetItem *YItem = ui->tableWidget->item(seleRow, 1);
    float Ypos = YItem->text().toFloat();
    qDebug()<<"Ypos:"<<Ypos;


    for(int i = 0;i < ui->yCount->text().toInt();i++)
    {
        int insertPosition = ui->tableWidget->currentRow()+1+i ;
        ui->tableWidget->insertRow(insertPosition);
        float y= Ypos+ui->ystep->text().toFloat()*(i+1);

        qDebug()<<"insertRow:"<<insertPosition<<" Xpos:"<<Xpos<<" y:"<<y;
        ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem(QString::number(Xpos,'f',4)));
        ui->tableWidget->setItem(insertPosition, 1, new QTableWidgetItem(QString::number(y,'f',4)));

    }


    // 设置插入行的数据
    // ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem("插入的行"));
    QMessageBox::information(nullptr, "成功", QString("已插入 %1 行").arg(ui->yCount->text().toInt()));
}

//添加自动点位
void AxisFormula::on_pushButton_6_clicked()
{
    if (ui->tableWidget->currentRow() < 0) {

        QMessageBox::information(nullptr, "提示", "请先选中一行");
        // 没有选中行
        return;
    }
    // 在第2行位置插入新行（0-based索引） 选中行的位置、 根据选中行的位置添加x、y轴

    int seleRow = ui->tableWidget->currentRow();

    QTableWidgetItem *XItem = ui->tableWidget->item(seleRow, 0);
    float Xpos = XItem->text().toFloat();
    qDebug()<<"xpos:"<<Xpos;

    QTableWidgetItem *YItem = ui->tableWidget->item(seleRow, 1);
    float Ypos = YItem->text().toFloat();
    qDebug()<<"Ypos:"<<Ypos;

    int totalCount= ui->yCount_3->text().toInt();//行数
    int xorder = ui->xorder->text().toInt();//0 增 ，1 减
    int yorder = ui->yorder->text().toInt();//0 增 ，1 减
    int xcount = ui->XCount_2->text().toInt();//列数
    float interval = ui->interval->text().toInt();//间距
    float xend = Xpos + xcount*interval;

    int count = 0;

    for(int i = 0;i < totalCount ; i++)
    {
        xorder = i%2;

        for(int j = 0;j<=xcount;j++)
        {

            if(xorder == 0)
            {

                // qDebug()<<"count:"<<count<<" x:"<<Xpos+((j)*interval)<<"  y:"<<Ypos<<" xorder:"<<xorder;
                if(i == 0 && j == 0)
                    continue;

                int insertPosition = ui->tableWidget->currentRow()+1+count ;
                ui->tableWidget->insertRow(insertPosition);

                qDebug()<<"insertRow:"<<insertPosition<<" Xpos:"<<Xpos+((j)*interval)<<" y:"<<Ypos;
                ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem(QString::number(Xpos+((j)*interval),'f',4)));
                ui->tableWidget->setItem(insertPosition, 1, new QTableWidgetItem(QString::number(Ypos,'f',4)));

            }else
            {

                if(i == 0 && j == 0)
                    continue;

                // qDebug()<<"count:"<<count<<" x:"<<xend-((j)*interval)<<"  y:"<<Ypos<<" xorder:"<<xorder;
                int insertPosition = ui->tableWidget->currentRow()+1+count ;
                ui->tableWidget->insertRow(insertPosition);

                qDebug()<<"insertRow:"<<insertPosition<<" Xpos:"<<xend-((j)*interval)<<" y:"<<Ypos;
                ui->tableWidget->setItem(insertPosition, 0, new QTableWidgetItem(QString::number(xend-((j)*interval),'f',4)));
                ui->tableWidget->setItem(insertPosition, 1, new QTableWidgetItem(QString::number(Ypos,'f',4)));


            }

            count++;

        }
        if(yorder == 1)
        {
            Ypos -=4;
        }else if(yorder == 0)
        {
            Ypos +=4;
        }
    }

}

