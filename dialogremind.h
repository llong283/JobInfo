#ifndef DIALOGREMIND_H
#define DIALOGREMIND_H

#include <QDialog>
#include <QDesktopServices>
#include <QUrl>
#include "config.h"

namespace Ui {
    class DialogRemind;
}

class DialogRemind : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRemind(QWidget *parent = 0);
    ~DialogRemind();

    void init(bool hasNewJobInfo);
    void readSettings();
    
signals:
    void signView();
    
private slots:
    void on_btnView_clicked();
    
private:
    Ui::DialogRemind *ui;
    
    bool openDatabase();
    void updateJobFair();
};

#endif // DIALOGREMIND_H
