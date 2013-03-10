#ifndef CONFIG_H
#define CONFIG_H

#include <QtGui>
#include <QtCore>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlError>

enum JobFairRemind {
    JobFairRemind_Id,
    JobFairRemind_Title,
    JobFairRemind_Time,
    JobFairRemind_Note
};

enum JobInfo {
    JobInfo_Id,
    JobInfo_Title,
    JobInfo_Addr,
    JobInfo_Time,
    JobInfo_IsRead
};

#endif // CONFIG_H
