#include "dialogset.h"
#include "ui_dialogset.h"

#include "setting.h"

DialogSet::DialogSet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSet)
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

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setFixedSize(size());

    m_appPath = QApplication::applicationFilePath();
    m_settingPath = QApplication::applicationDirPath() + "/JobInfo/setting.ini";
    readSettings();
    ui->btnOK->setDefault(true);
    m_intValid.setBottom(1);
    ui->lineEditAheadDays->setValidator(&m_intValid);
    ui->lineEditIntervalHours->setValidator(&m_intValid);
    ui->lineEditRecordDays->setValidator(&m_intValid);
}

DialogSet::~DialogSet()
{
    delete ui;
}

void DialogSet::readSettings()
{
#if defined(Q_WS_WIN)
    QSettings settingAutoRun("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\"
                       "CurrentVersion\\Run\\", QSettings::NativeFormat);
    if (settingAutoRun.value("JobInfo", QString("rem \"%1\"").arg(m_appPath)).toString() 
                             == QString("\"%1\"").arg(m_appPath)) {
        ui->checkBoxAutoRun->setChecked(true);
    } else {
        ui->checkBoxAutoRun->setChecked(false);
    }
#endif
//    QSettings setting(m_settingPath, QSettings::IniFormat);
    ui->checkBoxStartMin->setChecked(g_setting->getIsStartMin());
    ui->checkBoxCloseMin->setChecked(g_setting->getIsCloseMin());    
    ui->lineEditRecordDays->setText(QString::number(g_setting->getRecordDays()));    
    ui->lineEditAheadDays->setText(QString::number(g_setting->getAheadDays()));
    ui->lineEditIntervalHours->setText(QString::number(g_setting->getIntervalHours()));
}

bool DialogSet::writeSettings()
{
#if defined(Q_WS_WIN)
    QSettings settingAutoRun("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\"
                       "CurrentVersion\\Run\\", QSettings::NativeFormat);
    if (ui->checkBoxAutoRun->checkState() == Qt::Checked) {
        settingAutoRun.setValue("JobInfo", QString("\"%1\"").arg(m_appPath));
    } else {
        settingAutoRun.setValue("JobInfo", QString("rem \"%1\"").arg(m_appPath));
    }
#endif
    QSettings setting(m_settingPath, QSettings::IniFormat);
    setting.clear();
    setting.setValue("startmin", ui->checkBoxStartMin->checkState()==Qt::Checked ? true : false);
    setting.setValue("closemin", ui->checkBoxCloseMin->checkState()==Qt::Checked ? true : false);
    setting.setValue("recorddays", ui->lineEditRecordDays->text());
    setting.setValue("aheaddays", ui->lineEditAheadDays->text());
    setting.setValue("intervalhours", ui->lineEditIntervalHours->text());

    return true;
}

void DialogSet::on_btnOK_clicked()
{
    if (ui->lineEditAheadDays->text().toInt()<=0
            || ui->lineEditIntervalHours->text().toInt()<=0
            || ui->lineEditRecordDays->text().toInt()<=0) {
        QMessageBox::information(this, "信息", "提前天数或间隔小时或记录天数都要大于0");
        return;
    }
#if defined(Q_WS_WIN)
    QSettings settingAutoRun("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\"
                       "CurrentVersion\\Run\\", QSettings::NativeFormat);
    if (ui->checkBoxAutoRun->checkState() == Qt::Checked) {
        settingAutoRun.setValue("JobInfo", QString("\"%1\"").arg(m_appPath));
    } else {
        settingAutoRun.setValue("JobInfo", QString("rem \"%1\"").arg(m_appPath));
    }
#endif
    QSettings setting(m_settingPath, QSettings::IniFormat);
    setting.setValue("startmin", ui->checkBoxStartMin->checkState()==Qt::Checked ? true : false);
    setting.setValue("closemin", ui->checkBoxCloseMin->checkState()==Qt::Checked ? true : false);    
    setting.setValue("recorddays", ui->lineEditRecordDays->text());
    setting.setValue("aheaddays", ui->lineEditAheadDays->text());
    setting.setValue("intervalhours", ui->lineEditIntervalHours->text());
    g_setting->updateSetting();
    close();
}

void DialogSet::on_btnCancel_clicked()
{
    close();
}
