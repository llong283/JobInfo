#ifndef DIALOGSET_H
#define DIALOGSET_H

#include <QDialog>
#include "config.h"

namespace Ui {
    class DialogSet;
}

class DialogSet : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSet(QWidget *parent = 0);
    ~DialogSet();

private slots:
    void on_btnOK_clicked();
    
    void on_btnCancel_clicked();
    
private:
    Ui::DialogSet *ui;   
    QString m_appPath;
    QString m_settingPath;
    QIntValidator m_intValid;
    
    void readSettings();
    bool writeSettings();
};

#endif // DIALOGSET_H
