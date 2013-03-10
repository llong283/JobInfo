#include "DialogJobFairRemind.h"
#include "ui_DialogJobFairRemind.h"

DialogJobFairRemind::DialogJobFairRemind(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogJobFairRemind)
{
    ui->setupUi(this);
    ui->labelId->hide();
    ui->lineEditId->hide();
    ui->dateEdit->setCalendarPopup(true);
    
    //º”‘ÿqss
    QFile file(":/qss/JobInfo.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    } 
    file.close();    
}

DialogJobFairRemind::~DialogJobFairRemind()
{
    delete ui;
}

void DialogJobFairRemind::init(int id)
{
    m_id = id;
    QTextDocument *tDoc = ui->textEditNote->document();
    qDebug() << tDoc->defaultFont() << ui->textEditNote->font() << __FILE__ << __LINE__;
    ui->textEditNote->setFont(tDoc->defaultFont());
    ui->textEditNote->setStyleSheet(tDoc->defaultStyleSheet());
    qDebug() << ui->textEditNote->font() << __FILE__ << __LINE__;
    if (id != -1) {
        QSqlQueryModel queryModel;
        queryModel.setQuery(QString("select * from JobFairRemind where Id=%1").arg(m_id));
        if (queryModel.lastError().isValid()) {
            qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
            return;
        }
        if (queryModel.rowCount() == 0 || queryModel.rowCount() > 1) {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
        }
        QSqlRecord record = queryModel.record(0);
        ui->lineEditId->setText(record.value(JobFairRemind_Id).toString());
        ui->lineEditTitle->setText(record.value(JobFairRemind_Title).toString());
        ui->dateEdit->setDate(record.value(JobFairRemind_Time).toDate());
        ui->textEditNote->setText(record.value(JobFairRemind_Note).toString());
    } else {
        ui->lineEditId->clear();
        ui->lineEditTitle->clear();
        ui->dateEdit->setDate(QDate::currentDate());
        ui->textEditNote->clear();
    }
}

bool DialogJobFairRemind::openDatabase()
{
    QString name = QSqlDatabase::database().connectionName();
    QSqlDatabase::removeDatabase(name);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString fileName(QApplication::applicationDirPath() + "/JobInfo/JobInfo.db");
    QFileInfo fileInfo(fileName);
    QDir dir(fileInfo.absoluteDir());
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }
    db.setDatabaseName(fileName);    
    if (!db.open()) {
        QMessageBox::information(this, "–≈œ¢", db.lastError().text());
        return false;
    }    
    return true;
}

void DialogJobFairRemind::on_btnOK_clicked()
{
    openDatabase();
    QString sql;
    int id = ui->lineEditId->text().toInt();
    QString title = ui->lineEditTitle->text();
    QString time = ui->dateEdit->date().toString("yyyy-MM-dd");
    QString note = ui->textEditNote->toPlainText();
    if (m_id == -1) {
        sql = QString("insert into JobFairRemind (Title, Time, Note) values('%1', '%2', '%3')")
                .arg(title).arg(time).arg(note);
    } else {
        sql = QString("update JobFairRemind set Title='%1', Time='%2', Note='%3' where Id=%4")
                .arg(title).arg(time).arg(note).arg(id);
    }
    QSqlQueryModel queryModel;
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
    emit signFairChanged();
    close();
}

void DialogJobFairRemind::on_btnCancel_clicked()
{
    close();
}
