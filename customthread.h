#ifndef CUSTOMTHREAD_H
#define CUSTOMTHREAD_H

#include <QThread>
#include <Qtcore>
#include <QtWebKit>

class CustomThread : public QThread
{
    Q_OBJECT
public:
    explicit CustomThread(QObject *parent = 0);
    CustomThread(QObject *parent = 0, QMap<QDate, QString> fairDateMap = QMap<QDate, QString>());

    void setFairDateMap(QMap<QDate, QString> fairDateMap);

private:
    QMap<QDate, QString> m_fairDateMap;
    QWebPage *m_pWebPage;
    QDate m_fairMonth;
    QTime m_fairTime;
    QString m_fairTitle;
    QString m_fairHref;
    QString m_fairSite;
    bool m_isLoading;

    void examineChildElements(const QWebElement &parentElement);

protected:
    void run();

signals:

private slots:
    void slotRender();
    void slotRenderHome();
};

#endif // CUSTOMTHREAD_H
