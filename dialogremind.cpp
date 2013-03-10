#include "dialogremind.h"
#include "ui_dialogremind.h"

#include "setting.h"

enum TableJobFair {
    TableJobFair_Id,
    TableJobFair_Time,
    TableJobFair_Title,
    TableJobFair_Note
};

DialogRemind::DialogRemind(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRemind)
{
    ui->setupUi(this);

    //加载qss
    QFile file(":/qss/JobInfo.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    } 
    file.close();    

    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

    QStringList fairHeaderLabels;
    fairHeaderLabels << "编号" << "时间" << "招聘会" << "备注";
    ui->tableWidgetJobFair->setColumnCount(fairHeaderLabels.count());
    ui->tableWidgetJobFair->setHorizontalHeaderLabels(fairHeaderLabels);
//    ui->tableWidgetJobFair->horizontalHeader()->hide();
    ui->tableWidgetJobFair->verticalHeader()->hide();
    ui->tableWidgetJobFair->setSortingEnabled(true);
    ui->tableWidgetJobFair->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetJobFair->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidgetJobFair->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetJobFair->setFocusPolicy(Qt::NoFocus);
    ui->tableWidgetJobFair->hideColumn(TableJobFair_Id);
}

DialogRemind::~DialogRemind()
{
    delete ui;
}

void DialogRemind::init(bool hasNewJobInfo)
{
    if (hasNewJobInfo) {
        ui->label->setText("有新的招聘信息");
        QPalette palette;
        palette.setBrush(QPalette::WindowText, QBrush(Qt::red));
        ui->label->setPalette(palette);
    } else {
        QPalette palette;
        palette.setBrush(QPalette::WindowText, QBrush(Qt::black));
        ui->label->setPalette(palette);
        ui->label->setText("暂时没有新的招聘信息");
    }
    openDatabase();
    updateJobFair();
}

bool DialogRemind::openDatabase()
{
    QString name = QSqlDatabase::database().connectionName();
    QSqlDatabase::removeDatabase(name);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QApplication::applicationDirPath() + "/JobInfo/JobInfo.db");
    if (!db.open()) {
        QMessageBox::information(this, "msg", "open failed");
        return false;
    }    
    return true;
}

void DialogRemind::updateJobFair()
{
    ui->tableWidgetJobFair->clearContents();
    ui->tableWidgetJobFair->setRowCount(0);
    
    openDatabase();
    QDate currentDate = QDate::currentDate();
    QSqlQueryModel queryModel;
    QString sql = QString("select * from JobFairRemind where Time between '%1' and '%2'")
            .arg(currentDate.toString("yyyy-MM-dd"))
            .arg(currentDate.addDays(g_setting->getAheadDays()).toString("yyyy-MM-dd"));
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
    int count = queryModel.rowCount();
    ui->tableWidgetJobFair->setRowCount(count);
    for (int row=0; row<count; row++) {
        QSqlRecord record = queryModel.record(row);
        ui->tableWidgetJobFair->setItem(row, TableJobFair_Id, new QTableWidgetItem(
                                            record.value(JobFairRemind_Id).toString()));
        ui->tableWidgetJobFair->setItem(row, TableJobFair_Time, new QTableWidgetItem(
                                            record.value(JobFairRemind_Time).toDate().toString("yyyy-MM-dd")));
        ui->tableWidgetJobFair->setItem(row, TableJobFair_Title, new QTableWidgetItem(
                                            record.value(JobFairRemind_Title).toString()));
        ui->tableWidgetJobFair->setItem(row, TableJobFair_Note, new QTableWidgetItem(
                                            record.value(JobFairRemind_Note).toString()));
        ui->tableWidgetJobFair->item(row, TableJobFair_Note)->setToolTip(
                    "备注：\n" + record.value(JobFairRemind_Note).toString());
    }
}

void DialogRemind::on_btnView_clicked()
{
    emit signView();
    close();
}
