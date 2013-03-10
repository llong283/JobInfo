#ifndef DialogJobFairRemind_H
#define DialogJobFairRemind_H

#include <QDialog>
#include "config.h"

namespace Ui {
    class DialogJobFairRemind;
}

class DialogJobFairRemind : public QDialog
{
    Q_OBJECT

public:
    explicit DialogJobFairRemind(QWidget *parent = 0);
    ~DialogJobFairRemind();
    void init(int id=-1);
    
signals:
    void signFairChanged();
    
private slots:
    void on_btnOK_clicked();
    
    void on_btnCancel_clicked();
    
private:
    Ui::DialogJobFairRemind *ui;
    int m_id;
    
    bool openDatabase();
};

#endif // DialogJobFairRemind_H
