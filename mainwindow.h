#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QtWebKit>
#include <QtGui>
#include "config.h"

#include "downloadmanager.h"
#include "dialogJobFairRemind.h"
#include "dialogremind.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
public slots:
    void setVisible(bool visible);
    
protected:
    void showEvent(QShowEvent *se);
    void resizeEvent(QResizeEvent *re);
    void closeEvent(QCloseEvent *ce);

private:
    Ui::MainWindow *ui;

    //members
    QTimer m_timer;
    QWebPage m_webPage;
    QWebPage m_webFairPage;
    QDateTime m_lastCheckTime;
    QString m_mainUrl;
    DownloadManager m_downloadManager;
    long long m_lastId;
    int m_currentPage;
    QTranslator m_tr;
    bool m_hasNewJobInfo;    
        
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *m_restoreAction;         //还原
    QAction *m_minAction;             //最小化
    QAction *m_quitAction;            //退出

    DialogJobFairRemind *m_dJobFairRemind;
    DialogRemind *m_dRemind;
    
    //methods
    void initWidget();
    bool openDatabase();
    void createDatabase();
    void writeSettings();
    void viewJobFairRemind(int id=-1);
    void processOldJobInfo();
    void updateTableJobInfoWidth();

private slots:
    void slotAutoCheck();
    void slotRenderGraduate(const QMap<QUrl, QString> &downloadList);
    void slotUpdateJobInfo();
    void slotUpdateJobFair();
    void slotUpdateJobFairRemind();
    void slotReminder(bool hasNewJobInfo);
    void slotProcessSettingChanged(bool isRecordChanged, bool isAheadChanged, bool isIntervalChanged);
    void slotTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void slotOpen();
    
    void on_tableWidgetJobInfo_clicked(QModelIndex index);
    void on_tableWidgetJobFair_clicked(QModelIndex index);
    void on_btnView_clicked();
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_tableWidgetJobFairRemind_doubleClicked(QModelIndex index);
    void on_actionSet_triggered();
    void on_btnSetAllJobInfoRead_clicked();
    void on_btnSetAllJobFairRead_clicked();
    void on_btnMoreJobInfo_clicked();
    void on_btnMoreJobFair_clicked();
    void on_actionUpdate_triggered();
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
