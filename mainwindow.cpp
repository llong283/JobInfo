#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dialogremind.h"
#include "dialogset.h"
#include "setting.h"

enum TableJobFairRemind {
    TableJobFairRemind_Id,
    TableJobFairRemind_Time,
    TableJobFairRemind_Title,
    TableJobFairRemind_Note
};

enum TableJobInfo {
    TableJobInfo_Id,
    TableJobInfo_Time,
    TableJobInfo_Title,
    TableJobInfo_View
};
enum TableJobFair {
    TableJobFair_Id,
    TableJobFair_Time,
    TableJobFair_Title,
    TableJobFair_View
};

const QString cons_jobinfo_id = "classId=020601";
const QString cons_jobfair_id = "classId=020604";
const QString cons_jobinfo_str = "JobInfo";
const QString cons_jobfair_str = "JobFair";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_dJobFairRemind(NULL),
    m_dRemind(NULL),
    m_currentPage(1),
    m_hasNewJobInfo(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qApp->installTranslator(&m_tr);
    m_tr.load(":/other/qt_zh_CN.qm");
    
    initWidget();
    
    createDatabase();
    processOldJobInfo();
    
    connect(g_setting, SIGNAL(signSettingChanged(bool,bool,bool)), 
            this, SLOT(slotProcessSettingChanged(bool, bool, bool)));

    connect(&m_downloadManager, SIGNAL(finished(QMap<QUrl,QString>)), 
            this, SLOT(slotRenderGraduate(QMap<QUrl,QString>)));
    m_timer.setInterval(g_setting->getIntervalHours() * 60 * 60 * 1000);
//    m_timer.setInterval(3 * 1000);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotAutoCheck()));
    m_timer.start();
    slotAutoCheck();

    slotUpdateJobInfo();
    slotUpdateJobFair();
    slotUpdateJobFairRemind();   
    //没有新的招聘信息，则提醒招聘会
    if (m_dRemind == NULL) {
        slotReminder(false);
    }
    if (g_setting->getIsStartMin()) {
        hide();
    } else {
        show();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *se)
{
    QMainWindow::showEvent(se);
    updateTableJobInfoWidth();
}

void MainWindow::resizeEvent(QResizeEvent *re)
{
    QMainWindow::resizeEvent(re);
    updateTableJobInfoWidth();
}

void MainWindow::closeEvent(QCloseEvent *ce)
{    
    if (!g_setting->getIsCloseMin()) {
        if (QMessageBox::Yes == QMessageBox::information(
                    this, "信息", "确定要退出?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {            
            QApplication::quit();
        }        
    } else {
        QMainWindow::closeEvent(ce);
    }
}

//重写设置可见
void MainWindow::setVisible(bool visible)
{
    m_minAction->setVisible(visible);              //主窗口可见时，最小化有效
    m_restoreAction->setVisible(!visible);         //主窗口不可见时，还原有效

    QMainWindow::setVisible(visible);                //调用基类函数
}

void MainWindow::updateTableJobInfoWidth()
{
    int width;
    if (ui->tabWidget->currentWidget()==ui->tabJobInfo) {
        width = ui->tableWidgetJobInfo->width();
    } else {
        width = ui->tableWidgetJobFair->width();
    }
    int timeW = 130;
    int viewW = 60;
    int verticalWidth = 60;
    ui->tableWidgetJobInfo->setColumnWidth(TableJobInfo_Time, timeW);
    ui->tableWidgetJobInfo->setColumnWidth(TableJobInfo_View, viewW);
    ui->tableWidgetJobInfo->setColumnWidth(TableJobInfo_Title, width - timeW - viewW - verticalWidth);
    ui->tableWidgetJobFair->setColumnWidth(TableJobFair_Time, timeW);
    ui->tableWidgetJobFair->setColumnWidth(TableJobFair_View, viewW);
    ui->tableWidgetJobFair->setColumnWidth(TableJobFair_Title, width - timeW - viewW - verticalWidth);
}

void MainWindow::initWidget()
{
    //加载qss
    QFile file(":/qss/JobInfo.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    } 
    file.close();   
    
    QApplication::setQuitOnLastWindowClosed(false);
    
    m_minAction = new QAction("最小化", this);
    connect(m_minAction,SIGNAL(triggered()), this, SLOT(hide()));

    m_restoreAction = new QAction("还原", this);
    connect(m_restoreAction,SIGNAL(triggered()), this, SLOT(slotOpen()));

    m_quitAction = new QAction("退出", this);
    connect(m_quitAction,SIGNAL(triggered()),qApp,SLOT(quit()));
    //创建托盘图标菜单并添加动作
    trayIconMenu = new QMenu(QApplication::desktop());
    trayIconMenu->addAction(m_restoreAction);
    trayIconMenu->addAction(m_minAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(ui->actionUpdate);
    trayIconMenu->addAction(ui->actionSet);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(m_quitAction);
    //创建并设置托盘图标
    trayIcon = new QSystemTrayIcon(this);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->setToolTip("招聘提醒");
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/pic/tray.png"));
    trayIcon->show();        

    QStringList jobInfoHeaderLabels;
    QStringList jobFairHeaderLabels;
    QStringList jobFairRemindHeaderLabels;
    jobInfoHeaderLabels << "编号" << "发布时间" << "名称" << "查看";
    ui->tableWidgetJobInfo->setColumnCount(jobInfoHeaderLabels.count());
    ui->tableWidgetJobInfo->setHorizontalHeaderLabels(jobInfoHeaderLabels);
    ui->tableWidgetJobInfo->setSortingEnabled(true);
    ui->tableWidgetJobInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetJobInfo->hideColumn(TableJobInfo_Id);
    ui->tableWidgetJobInfo->horizontalHeader()->setSortIndicator(TableJobInfo_Id, Qt::DescendingOrder);
    jobFairHeaderLabels << "编号" << "宣讲时间" << "名称" << "查看";    
    ui->tableWidgetJobFair->setColumnCount(jobFairHeaderLabels.count());
    ui->tableWidgetJobFair->setHorizontalHeaderLabels(jobFairHeaderLabels);
    ui->tableWidgetJobFair->setSortingEnabled(true);
    ui->tableWidgetJobFair->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetJobFair->hideColumn(TableJobInfo_Id);
    ui->tableWidgetJobFair->horizontalHeader()->setSortIndicator(TableJobFair_Id, Qt::DescendingOrder);
    jobFairRemindHeaderLabels << "编号" << "时间" << "招聘会" << "备注";
    ui->tableWidgetJobFairRemind->setColumnCount(jobFairRemindHeaderLabels.count());
    ui->tableWidgetJobFairRemind->setHorizontalHeaderLabels(jobFairRemindHeaderLabels);
    ui->tableWidgetJobFairRemind->setSortingEnabled(true);
    ui->tableWidgetJobFairRemind->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetJobFairRemind->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableWidgetJobFairRemind->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetJobFairRemind->hideColumn(TableJobFairRemind_Id);
    
    ui->tabWidget->setCurrentWidget(ui->tabJobInfo);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(ui->tabWidget);
    splitter->addWidget(ui->groupBoxJobFairRemind);    
    ui->widget->layout()->addWidget(splitter);
    
    //窗口居中
    QRect screenRect;
    QPoint pos;
    screenRect = QApplication::desktop()->availableGeometry();
    pos = QPoint((screenRect.width() - frameSize().width()) / 2,
                 (screenRect.height() - frameSize().height()) / 2);
    move(pos);    
}

void MainWindow::slotTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        slotOpen();
        break;
    default:
        break;
    }
}

void MainWindow::slotOpen()
{
    activateWindow();
    showNormal();
}

void MainWindow::slotAutoCheck()
{
    qDebug() << "auto check" << __FILE__ << __LINE__;
    m_mainUrl = "http://graduate.cqu.edu.cn/fourthIndex.do?classId=020601";
    m_downloadManager.append(QUrl(m_mainUrl));
    m_downloadManager.append(QUrl("http://graduate.cqu.edu.cn/fourthIndex.do?classId=020604"));
}

void MainWindow::slotProcessSettingChanged(bool isRecordChanged, bool isAheadChanged, bool isIntervalChanged)
{    
    if (isRecordChanged) {
        openDatabase();
        QSqlQueryModel queryModel;        
        //清空招聘信息，重新更新
        queryModel.setQuery("delete from JobInfo");
        if (queryModel.lastError().isValid()) {
            qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
            return;
        }
        slotUpdateJobInfo();
        slotUpdateJobFair();
        m_lastId = 0;
        m_timer.start();
        slotAutoCheck();
    } else if (isIntervalChanged) {
        m_timer.setInterval(g_setting->getIntervalHours() * 60 * 60 * 1000);
        m_timer.start();
        slotAutoCheck();
    }
}

bool MainWindow::openDatabase()
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
        QMessageBox::information(this, "信息", db.lastError().text());
        return false;
    }    
    return true;
}

void MainWindow::createDatabase()
{
    openDatabase();
    QString sql;
    QSqlQueryModel queryModel;
    sql = QString("create table if not exists JobFairRemind("
                  "Id integer primary key autoincrement,"
                  "Title varchar,"
                  "Time date,"
                  "Note varchar)");
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
    sql = QString("create table if not exists JobInfo("
                  "Id integer primary key not null,"
                  "Title varchar,"
                  "Addr varchar,"
                  "Time date,"
                  "IsRead bool default(0),"
                  "Type varchar)");
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
}

void MainWindow::processOldJobInfo()
{
    openDatabase();
    QString sql;
    QSqlQueryModel queryModel;
    sql = QString("select * from JobInfo order by Id desc");
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
    //初始化最新id
    if (queryModel.rowCount() > 0) {
        m_lastId = queryModel.record(0).value(JobInfo_Id).toLongLong();        
    } else {
        m_lastId = 0;
    }
    
    //清空旧信息
    sql = QString("delete from JobInfo where Time<='%1'")
            .arg(QDate::currentDate().addDays(0-g_setting->getRecordDays()).toString("yyyy-MM-dd"));
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
}

void MainWindow::slotRenderGraduate(const QMap<QUrl, QString> &downloadList)
{
//    bool hasNewJobInfo = false;
    QDateTime oldestJobInfoTime;
    QDateTime oldestJobFairTime;
    bool hasMoreJobInfo = true;
//    bool hasMoreJobFair = oldestJobFairTime >= QDateTime::currentDateTime().addDays(0-g_setting->getRecordDays());
    bool hasMoreJobFair = true;

    qDebug() << "render graduate" << "count" << downloadList.count() << __FILE__ << __LINE__;
    for (QMap<QUrl, QString>::const_iterator iter = downloadList.begin(); iter != downloadList.end(); iter++) {
        QUrl url = iter.key();
        m_webPage.mainFrame()->setHtml(iter.value());
        QWebElement document = m_webPage.mainFrame()->documentElement();
        QWebElementCollection links = document.findAll("li");        
        QRegExp timeExp("(\\d{4}-\\d{2}-\\d{2})");
        QRegExp fairTimeExp("(\\d{4}.\\d{2}.\\d{2}-\\d{2}:\\d{2})");
        QString curTypeStr;
//        QDate oldestDate;
        
//        int index = timeExp.indexIn(links.last().toPlainText());
//        if (index != -1) {
//            oldestDate = QDate::fromString(timeExp.cap(1), "yyyy-MM-dd"); //待修改
//        } else {
//            qDebug() << "this is a bug" << __FILE__ << __LINE__;
//        }
        if (url.toString().contains(cons_jobinfo_id)) {
            curTypeStr = cons_jobinfo_str;
////            oldestJobInfoDate = oldestDate;
//            index = timeExp.indexIn(links.last().toPlainText());
//            if (index != -1) {
//                oldestJobInfoDate = QDate::fromString(timeExp.cap(1), "yyyy-MM-dd");
//            } else {
//                qDebug() << "this is a bug" << __FILE__ << __LINE__;
//            }            
//            qDebug() << "jobinfo date" << oldestJobInfoDate << __FILE__ << __LINE__;
        } else {
            curTypeStr = cons_jobfair_str;
////            oldestJobFairDate = oldestDate;
//            index = fairTimeExp.indexIn(links.last().toPlainText());
//            if (index != -1) {
//                oldestJobFairDate = QDate::fromString(fairTimeExp.cap(1), "yyyy.MM.dd");
//            } else {
//                qDebug() << "this is a bug" << __FILE__ << __LINE__;
//            }            
//            qDebug() << "oldestJobFairDate" << oldestJobFairDate << __FILE__ << __LINE__;
        }

        foreach(QWebElement link, links) {
            QWebElementCollection addrs = link.findAll("a");
            if (addrs.count() > 1) {
                qDebug() << "addrs is more than one" << __FILE__ << __LINE__;
            }
            QWebElement addr = addrs.at(0);
            QStringList attrs = addr.attributeNames();
            QString href;
            QString title = addr.toPlainText();
            QRegExp idExp("newsinfoid=(\\d+)");
            QDateTime time;
            long long id;
            int index;
            
            if (curTypeStr == cons_jobinfo_str) {
                index = timeExp.indexIn(link.toPlainText());
                if (index != -1) {
                    time = QDateTime::fromString(timeExp.cap(1), "yyyy-MM-dd");
                    if (oldestJobInfoTime.isNull() || oldestJobInfoTime > time) {
                        oldestJobInfoTime = time;
                    }
                }
            } else {
                index = fairTimeExp.indexIn(link.toPlainText());
                if (index != -1) {
                    time = QDateTime::fromString(fairTimeExp.cap(1), "yyyy.MM.dd-hh:mm");
                    if (oldestJobFairTime.isNull() || oldestJobFairTime > time) {
                        oldestJobFairTime = time;
                    }
                    title = title.remove(fairTimeExp);
                }
            }
            //链接地址
            foreach(QString attr, attrs) {
                if (QString::compare(attr, "href", Qt::CaseInsensitive) == 0) {
                    href = "http://graduate.cqu.edu.cn/" + addr.attribute(attr);
                    break;
                }
            }
            int idIndex = idExp.indexIn(href);
            if (idIndex == -1) {
                continue;
            } else {
                id = idExp.cap(1).toLongLong();
            }
            
            qDebug() << "id:" << id << "title:" << title << "time" << time << __FILE__ << __LINE__;
            
            if (curTypeStr==cons_jobinfo_str 
                    && (id<=m_lastId || time<=QDateTime::currentDateTime().addDays(0-g_setting->getRecordDays()))) {
                hasMoreJobInfo = false;
                break;
            } else {
                QString sql;                
                QSqlQueryModel queryModel;
                
                sql = QString("select * from JobInfo where Id=%1").arg(id);
                queryModel.setQuery(sql);
                if (queryModel.lastError().isValid()) {
                    qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
                }
                if (queryModel.rowCount() == 0) {
                    m_hasNewJobInfo = true;
                    sql = QString("insert into JobInfo (Id, Title, Addr, Time, IsRead, Type) values(%1, '%2', '%3', '%4', %5, '%6')")
                            .arg(id).arg(title).arg(href).arg(time.toString("yyyy-MM-dd hh:mm:ss")).arg(false).arg(curTypeStr);
                    queryModel.setQuery(sql);
                    if (queryModel.lastError().isValid()) {
                        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
                    }
                } else {
                    qDebug() << id << "exist in JobInfo" << __FILE__ << __LINE__;
                }
            }
        }
    }
    m_webPage.mainFrame()->setHtml("");
//    QSqlQueryModel queryModel;
//    QString sql = "select min(Time) from JobInfo";
//    queryModel.setQuery(sql);
//    if (queryModel.lastError().isValid()) {
//        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
//    }
//    QDate recordOldestDate = queryModel.record(0).value(0).toDate();
    if (hasMoreJobInfo && oldestJobInfoTime < QDateTime::currentDateTime().addDays(0-g_setting->getRecordDays())) {
        hasMoreJobInfo = false;
    }
//    bool hasMoreJobFair = oldestJobFairTime >= QDateTime::currentDateTime().addDays(0-g_setting->getRecordDays());
    if (hasMoreJobFair && oldestJobFairTime.isNull()) {
        hasMoreJobFair = false;
    }
    qDebug() << "has more jobinfo" << hasMoreJobInfo << oldestJobInfoTime 
             << "has more jobfair" << hasMoreJobFair << oldestJobFairTime
             << __FILE__ << __LINE__;
//    if (oldestJobInfoDate >= QDate::currentDate().addDays(0-g_setting->getRecordDays())
//            || oldestJobFairDate >= QDate::currentDate().addDays(0-g_setting->getRecordDays()) 
//            /*&& oldestJobInfoDate >= recordOldestDate*/) {
    if (hasMoreJobInfo || hasMoreJobFair) {
        m_currentPage++;
//        QUrl url(QString("http://graduate.cqu.edu.cn/fourthIndex.do?classId=020601&viewPage=%1").arg(m_currentPage));
//        m_downloadManager.append(url);
        if (hasMoreJobInfo) {
            m_downloadManager.append(QString("http://graduate.cqu.edu.cn/fourthIndex.do?classId=020601&viewPage=%1")
                                     .arg(m_currentPage));
        }
        if (hasMoreJobFair) {
            m_downloadManager.append(QString("http://graduate.cqu.edu.cn/fourthIndex.do?classId=020604&viewPage=%1")
                                     .arg(m_currentPage));
        }
        return;
    } else {
        m_currentPage = 1;
    }
    qDebug() << "slotRenderGraduate" << m_hasNewJobInfo << __FILE__ << __LINE__;
    if (m_hasNewJobInfo) {
        slotUpdateJobInfo();
        slotUpdateJobFair();
        slotReminder(m_hasNewJobInfo);
        m_hasNewJobInfo = false;
    }
}

void MainWindow::slotUpdateJobInfo()
{
    ui->tableWidgetJobInfo->clearContents();
    ui->tableWidgetJobInfo->setRowCount(0);

    openDatabase();    
    QSqlQueryModel queryModel;
    queryModel.setQuery("select * from JobInfo where Type='JobInfo' order by Id desc");
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
    Qt::SortOrder order = ui->tableWidgetJobInfo->horizontalHeader()->sortIndicatorOrder();
    int section = ui->tableWidgetJobInfo->horizontalHeader()->sortIndicatorSection();
    qDebug() << "jobinfo order" << section << order << __FILE__ << __LINE__;
    ui->tableWidgetJobInfo->horizontalHeader()->setSortIndicator(TableJobInfo_Id, Qt::DescendingOrder);
    int count = queryModel.rowCount();
    ui->tableWidgetJobInfo->setRowCount(count);
    for (int row=0; row<count; row++) {
        QSqlRecord record = queryModel.record(row);
        if (row == 0) {
            m_lastId = record.value(JobInfo_Id).toLongLong();
        }
        ui->tableWidgetJobInfo->setItem(row, TableJobInfo_Id, new QTableWidgetItem(
                                            record.value(JobInfo_Id).toString()));
        ui->tableWidgetJobInfo->setItem(row, TableJobInfo_Time, new QTableWidgetItem(
                                            record.value(JobInfo_Time).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        ui->tableWidgetJobInfo->setItem(row, TableJobInfo_Title, new QTableWidgetItem(
                                            record.value(JobInfo_Title).toString()));
        QTableWidgetItem *twItem = new QTableWidgetItem("详细");
        twItem->setData(Qt::UserRole, record.value(JobInfo_Addr));
        twItem->setForeground(QBrush(Qt::red));
        twItem->setTextAlignment(Qt::AlignCenter);
        QFont font(twItem->font());
        font.setUnderline(true);
        twItem->setFont(font);        
        ui->tableWidgetJobInfo->setItem(row, TableJobInfo_View, twItem);
        if (!record.value(JobInfo_IsRead).toBool()) {
            ui->tableWidgetJobInfo->item(row, TableJobFairRemind_Title)->setForeground(QBrush(Qt::blue));
        }
    }      
    ui->tableWidgetJobInfo->horizontalHeader()->setSortIndicator(section, order);    
}

void MainWindow::slotUpdateJobFair()
{
    ui->tableWidgetJobFair->clearContents();
    ui->tableWidgetJobFair->setRowCount(0);

    openDatabase();    
    QSqlQueryModel queryModel;
    queryModel.setQuery("select * from JobInfo where Type='JobFair' order by Id desc");
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
    Qt::SortOrder order = ui->tableWidgetJobFair->horizontalHeader()->sortIndicatorOrder();
    int section = ui->tableWidgetJobFair->horizontalHeader()->sortIndicatorSection();
    ui->tableWidgetJobFair->horizontalHeader()->setSortIndicator(TableJobFair_Id, Qt::DescendingOrder);
//    queryModel.sort(section, order);
    qDebug() << "jobinfo order" << order << section << __FILE__ << __LINE__;
    int count = queryModel.rowCount();
    ui->tableWidgetJobFair->setRowCount(count);
    for (int row=0; row<count; row++) {
        QSqlRecord record = queryModel.record(row);
        if (row == 0) {
            long long lastFairId = record.value(JobInfo_Id).toLongLong();
            if (lastFairId > m_lastId) {
                m_lastId = lastFairId;
            }
        }
        ui->tableWidgetJobFair->setItem(row, TableJobFair_Id, new QTableWidgetItem(
                                            record.value(JobInfo_Id).toString()));
        ui->tableWidgetJobFair->setItem(row, TableJobFair_Time, new QTableWidgetItem(
                                            record.value(JobInfo_Time).toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        ui->tableWidgetJobFair->setItem(row, TableJobFair_Title, new QTableWidgetItem(
                                            record.value(JobInfo_Title).toString()));
        QTableWidgetItem *twItem = new QTableWidgetItem("详细");
        twItem->setData(Qt::UserRole, record.value(JobInfo_Addr));
        twItem->setForeground(QBrush(Qt::red));
        twItem->setTextAlignment(Qt::AlignCenter);
        QFont font(twItem->font());
        font.setUnderline(true);
        twItem->setFont(font);        
        ui->tableWidgetJobFair->setItem(row, TableJobFair_View, twItem);
//        qDebug() << row << twItem->text() << twItem->data(Qt::UserRole) << __FILE__ << __LINE__;
        if (!record.value(JobInfo_IsRead).toBool()) {
            ui->tableWidgetJobFair->item(row, TableJobFairRemind_Title)->setForeground(QBrush(Qt::blue));
        }
    }   
    ui->tableWidgetJobFair->horizontalHeader()->setSortIndicator(section, order);    
}

void MainWindow::slotUpdateJobFairRemind()
{
    ui->tableWidgetJobFairRemind->clearContents();
    ui->tableWidgetJobFairRemind->setRowCount(0);
    
    openDatabase();
    QSqlQueryModel queryModel;
    queryModel.setQuery("select * from JobFairRemind order by Time asc");
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }
    int count = queryModel.rowCount();
    ui->tableWidgetJobFairRemind->setRowCount(count);
    for (int row=0; row<count; row++) {
        QSqlRecord record = queryModel.record(row);
        ui->tableWidgetJobFairRemind->setItem(row, TableJobFairRemind_Id, new QTableWidgetItem(
                                            record.value(JobFairRemind_Id).toString()));
        ui->tableWidgetJobFairRemind->setItem(row, TableJobFairRemind_Time, new QTableWidgetItem(
                                            record.value(JobFairRemind_Time).toDate().toString("yyyy-MM-dd")));
        ui->tableWidgetJobFairRemind->setItem(row, TableJobFairRemind_Title, new QTableWidgetItem(
                                            record.value(JobFairRemind_Title).toString()));
        ui->tableWidgetJobFairRemind->setItem(row, TableJobFairRemind_Note, new QTableWidgetItem(
                                            record.value(JobFairRemind_Note).toString()));
        ui->tableWidgetJobFairRemind->item(row, TableJobFairRemind_Note)->setToolTip("备注：\n" +
                    record.value(JobFairRemind_Note).toString());
    }
}

void MainWindow::slotReminder(bool hasNewJobInfo)
{
    QRect screenRect;
    QPoint pos;

    if (m_dRemind == NULL) {
        m_dRemind = new DialogRemind;
        connect(m_dRemind, SIGNAL(signView()), this, SLOT(showNormal()));
    }
    m_dRemind->init(hasNewJobInfo);
    //show要放在move之前，不然对话框边框的尺寸获取不到
    pos = QPoint(screenRect.width() - m_dRemind->frameSize().width(),
                 screenRect.height() - m_dRemind->frameSize().height());
    m_dRemind->move(pos);
    m_dRemind->activateWindow();
    m_dRemind->showNormal();
    screenRect = QApplication::desktop()->availableGeometry();
    pos = QPoint(screenRect.width() - m_dRemind->frameSize().width(),
                 screenRect.height() - m_dRemind->frameSize().height());
    m_dRemind->move(pos);
}

void MainWindow::viewJobFairRemind(int id)
{
    if (m_dJobFairRemind == NULL) {
        m_dJobFairRemind = new DialogJobFairRemind;
        connect(m_dJobFairRemind, SIGNAL(signFairChanged()), this, SLOT(slotUpdateJobFairRemind()));
    }
    m_dJobFairRemind->init(id);
    m_dJobFairRemind->showNormal();    
}

void MainWindow::on_btnView_clicked()
{
    QList<QTableWidgetSelectionRange> ranges = ui->tableWidgetJobFairRemind->selectedRanges();    
    if (ranges.count() > 0) {
        if (ranges.count() > 1) {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
        }
        QTableWidgetSelectionRange range = ranges.at(0);
        if (range.bottomRow() != range.topRow()) {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
        }
        int row = range.bottomRow();
        viewJobFairRemind(ui->tableWidgetJobFairRemind->item(row, TableJobFairRemind_Id)->text().toInt());
    }
}

void MainWindow::on_btnAdd_clicked()
{
    viewJobFairRemind();
}

void MainWindow::on_btnDelete_clicked()
{
    QList<QTableWidgetSelectionRange> ranges = ui->tableWidgetJobFairRemind->selectedRanges();    
    openDatabase();
    foreach (QTableWidgetSelectionRange range, ranges) {
        int bottomRow = range.bottomRow();
        int topRow = range.topRow();
        for (int row=topRow; row<=bottomRow; row++) {
            int id = ui->tableWidgetJobFairRemind->item(row, TableJobFairRemind_Id)->text().toInt();
            QString sql = QString("delete from JobFairRemind where Id=%1").arg(id);
            QSqlQueryModel queryModel;
            queryModel.setQuery(sql);
            if (queryModel.lastError().isValid()) {
                qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
                return;
            }            
        }
    }    
    slotUpdateJobFairRemind();
}

void MainWindow::on_tableWidgetJobInfo_clicked(QModelIndex index)
{
    QTableWidgetItem *item = ui->tableWidgetJobInfo->item(index.row(), index.column());
    QString urlStr;

    urlStr = item->data(Qt::UserRole).toString();
    qDebug() << urlStr << __FILE__ << __LINE__;
    if (index.column()==TableJobInfo_View && !urlStr.isNull()) {
        openDatabase();
        int id = ui->tableWidgetJobInfo->item(index.row(), TableJobInfo_Id)->text().toInt();
        QString sql = QString("update JobInfo set isRead=1 where Id=%1").arg(id);
        QSqlQueryModel queryModel;
        queryModel.setQuery(sql);
        if (queryModel.lastError().isValid()) {
            qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
            return;
        }        
        slotUpdateJobInfo();
        QDesktopServices::openUrl(QUrl::fromEncoded(urlStr.toLocal8Bit()));
    }
}

void MainWindow::on_tableWidgetJobFair_clicked(QModelIndex index)
{
    QTableWidgetItem *item = ui->tableWidgetJobFair->item(index.row(), index.column());
    QString urlStr;

    urlStr = item->data(Qt::UserRole).toString();
    qDebug() << urlStr << item << __FILE__ << __LINE__;
    if (index.column()==TableJobFair_View && !urlStr.isNull()) {
        openDatabase();
        int id = ui->tableWidgetJobFair->item(index.row(), TableJobFair_Id)->text().toInt();
        QString sql = QString("update JobInfo set isRead=1 where Id=%1").arg(id);
        QSqlQueryModel queryModel;
        queryModel.setQuery(sql);
        if (queryModel.lastError().isValid()) {
            qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
            return;
        }        
        slotUpdateJobFair();
        qDebug() << ui->tableWidgetJobFair->item(index.row(), index.column()) << __FILE__ << __LINE__;
        QDesktopServices::openUrl(QUrl::fromEncoded(urlStr.toLocal8Bit()));
    }
}

void MainWindow::on_tableWidgetJobFairRemind_doubleClicked(QModelIndex index)
{
    if (index.isValid()) {
        int row = index.row();
        int id = ui->tableWidgetJobFairRemind->item(row, TableJobFairRemind_Id)->text().toInt();
        viewJobFairRemind(id);
    }
}    

void MainWindow::on_actionSet_triggered()
{
    DialogSet *dSet = new DialogSet;
    dSet->exec();
}

void MainWindow::on_btnSetAllJobInfoRead_clicked()
{
    openDatabase();
    QString sql = QString("update JobInfo set isRead=1 where Type='JobInfo'");
    QSqlQueryModel queryModel;
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }            
    slotUpdateJobInfo();
}

void MainWindow::on_btnSetAllJobFairRead_clicked()
{
    openDatabase();
    QString sql = QString("update JobInfo set isRead=1 where Type='JobFair'");
    QSqlQueryModel queryModel;
    queryModel.setQuery(sql);
    if (queryModel.lastError().isValid()) {
        qDebug() << queryModel.lastError() << __FILE__ << __LINE__;
        return;
    }            
    slotUpdateJobFair();
}

void MainWindow::on_btnMoreJobInfo_clicked()
{
    QDesktopServices::openUrl(QUrl("http://graduate.cqu.edu.cn/fourthIndex.do?classId=020601"));    
}

void MainWindow::on_btnMoreJobFair_clicked()
{
    QDesktopServices::openUrl(QUrl("http://graduate.cqu.edu.cn/fourthIndex.do?classId=020604"));    
}

void MainWindow::on_actionUpdate_triggered()
{
    m_timer.start();
    slotAutoCheck();
    QTime t;
    t.start();
    while (t.elapsed() < 200) {
        QCoreApplication::processEvents();
    }
    qDebug() << "actionUpdate" << __FILE__ << __LINE__;
    if (m_dRemind->isHidden()) {
        slotReminder(false);
    }
}

void MainWindow::on_pushButton_clicked()
{
    m_downloadManager.append(QUrl("http://192.168.100.210//red.rar"));    
}
