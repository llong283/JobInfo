#include <QtGui/QApplication>
#include <QtCore>
#include "mainwindow.h"
//#include <QtTest>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *tc = QTextCodec::codecForName("gb2312");
    QTextCodec::setCodecForCStrings(tc);
    QTextCodec::setCodecForLocale(tc);
    QTextCodec::setCodecForTr(tc);

    // ȷ��ֻ����һ��
    QSystemSemaphore sema("JAMKey",1,QSystemSemaphore::Open);
    sema.acquire();// ���ٽ������������ڴ�   SharedMemory
    QSharedMemory mem("JobInfo");// ȫ�ֶ�����
    if (!mem.create(1))// ���ȫ�ֶ����Դ������˳�
    {
        QMessageBox::warning(0, QObject::tr("����"), QObject::tr("�����Ѿ�����."));
        sema.release();// ����� Unix ϵͳ�����Զ��ͷš�
        return 0;
    }
    sema.release();// �ٽ���

    MainWindow w;
//    w.show();
    return a.exec();
}
