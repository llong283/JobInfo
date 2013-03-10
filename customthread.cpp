#include "customthread.h"

CustomThread::CustomThread(QObject *parent) :
    QThread(parent)
{
}

CustomThread::CustomThread(QObject *parent, QMap<QDate, QString> fairDateMap) :
    QThread(parent), m_fairDateMap(fairDateMap)
{
    m_isLoading = false;
    m_pWebPage = new QWebPage();
    connect(m_pWebPage, SIGNAL(loadFinished(bool)), this, SLOT(slotRender()));
    QMap<QDate, QString>::iterator iter;
    for (iter = m_fairDateMap.begin(); iter != m_fairDateMap.end(); iter++) {
        qDebug() << iter.key() << ":" << iter.value() << __FILE__ << __LINE__;
        while (m_isLoading) {
            sleep(10);
        }
        m_pWebPage->mainFrame()->load(QUrl(iter.value()));
        m_isLoading = true;
    }
}

void CustomThread::run()
{
}

void CustomThread::setFairDateMap(QMap<QDate, QString> fairDateMap)
{
    m_fairDateMap = fairDateMap;
}

void CustomThread::slotRenderHome()
{
    QWebElement document = m_pWebPage->mainFrame()->documentElement();
    QString htmlStr = m_pWebPage->mainFrame()->toHtml();
    QRegExp regExpMonth("(\\d{4})年(\\d{1,2})月");
    int index = regExpMonth.indexIn(htmlStr);
    if (index != -1) {
        m_fairMonth = QDate(regExpMonth.cap(1).toInt(), regExpMonth.cap(2).toInt(), 1);
    } else {
        qDebug() << "m_fairMonth is not initialized" << __FILE__ << __LINE__;
    }
    examineChildElements(document);

    qDebug() << m_fairDateMap << __FILE__ << __LINE__;
    m_isLoading = false;
}

void CustomThread::slotRender()
{
    QWebElement document = m_pWebPage->mainFrame()->documentElement();
    qDebug() << document.toOuterXml() << __FILE__ << __LINE__;
    QWebElementCollection paraCollection = document.findAll("p.MsoNormal");
    QString paraHtmlStr;
    QRegExp regExpTime("(\\d{1,2}):(\\d\\d)"); //时间正则表达式
    QRegExp regExpSite(">(\\w+)<");  //地点正则表达式

    foreach(QWebElement paragraph, paraCollection) {
        int index;

        paraHtmlStr = paragraph.toOuterXml();
        index = regExpTime.indexIn(paraHtmlStr);
        if (index != -1 && !paraHtmlStr.contains("title", Qt::CaseInsensitive)) { //有可能title里有时间，所以不能包含title
            //获得招聘会的时间
            m_fairTime.setHMS(regExpTime.cap(1).toInt(), regExpTime.cap(2).toInt(), 0);
        } else if (paraHtmlStr.contains("href=", Qt::CaseInsensitive)) {
            QWebElementCollection linkCollection = paragraph.findAll("a");
            //可能会有多个链接，所以取有标题的链接
            foreach(QWebElement link, linkCollection) {
                //获得链接地址
                QStringList attrList = link.attributeNames();
                foreach(QString attr, attrList) {
                    if (QString::compare(attr, "href", Qt::CaseInsensitive) == 0) {
                        m_fairHref = link.attribute(attr);
                        break;
                    }
                }
                //获得链接标题
                m_fairTitle = link.findFirst("u").toInnerXml();
                if (m_fairTitle.isEmpty()) {
                    m_fairTitle = link.toInnerXml();
                }
                if (!m_fairTitle.isEmpty()) {
                    break;
                } else {
                    //                    qDebug() << link.toOuterXml() << __FILE__ << __LINE__;
                }
            }
        } else {
            //获取招聘会的地点
            if (m_fairHref.isEmpty() || m_fairTitle.isEmpty()) {
                continue;
            } else {
                //                index = 0;
                //                while ((index = regExpSite.indexIn(paraHtmlStr, index)) != -1) {
                //                    m_fairSite += regExpSite.cap(1);
                //                    index += regExpSite.matchedLength();
                //                }
                QWebElementCollection spanCollection = paragraph.findAll("span");
                foreach(QWebElement span, spanCollection) {
                    //                    m_fairSite += span.toInnerXml();
                    if (!m_fairSite.contains(span.toPlainText())) {
                        m_fairSite += span.toPlainText();
                    }
                }

                qDebug() << m_fairTime << m_fairTitle << m_fairHref << m_fairSite << __FILE__ << __LINE__;

                //清空
                m_fairHref.clear();
                m_fairSite.clear();
                m_fairSite.clear();
            }
        }
    }
    m_isLoading = false;
}

void CustomThread::examineChildElements(const QWebElement &parentElement)
{
    QWebElement element = parentElement.firstChild();
    QRegExp regExp("\\d{4}-\\d{1,2}-\\d{1,2} \\d{1,2}:\\d{1,2}:\\d{1,2}");
    QString newsTitle;
    QString newsTime;
    QString newsHref;
    QString title;
    QStringList attrList;

    while (!element.isNull()) {
        attrList = element.attributeNames();
        foreach(QString attr, attrList) {
            if (QString::compare(attr, "title", Qt::CaseInsensitive) == 0) {
                title = element.attribute(attr);
            } else if (QString::compare(attr, "href", Qt::CaseInsensitive) == 0) {
                newsHref = element.attribute(attr);
            }
        }

        int pos = regExp.indexIn(title);
        if (pos != -1) {
            QString time;
            QStringList dateTimeList;
            QStringList dateList;
            QStringList timeList;

            time = title.mid(pos, regExp.matchedLength());
            dateTimeList = time.split(" ");
            dateList = dateTimeList.at(0).split("-");
            timeList = dateTimeList.at(1).split(":");
            newsTime.sprintf("%d-%02d-%02d %02d:%02d:%02d",
                             dateList.at(0).toInt(), dateList.at(1).toInt(), dateList.at(2).toInt(),
                             timeList.at(0).toInt(), timeList.at(1).toInt(), timeList.at(2).toInt());
        }

        if (!title.isEmpty()) {
            if (title.left(3) == "点击数") {
                newsTitle = element.toInnerXml().trimmed();
            } else {
                pos = title.indexOf("点击数");
                if (pos != -1) {
                    newsTitle = title.left(pos).trimmed();
                }
            }
        } else if (!newsHref.isEmpty() && newsHref.contains("招聘会日历2", Qt::CaseInsensitive)) {
            //提取招聘会日历信息，保存至map
            newsTitle = element.toInnerXml().trimmed();
            if (newsTitle.contains("<")) {
                QRegExp regExpDay(">(\\d+)<");
                QDate fairDate;
                int index;
                int day;
                int daysInterval;

                index = regExpDay.indexIn(newsTitle);
                if (index != -1) {
                    day = regExpDay.cap(1).toInt();
                }
                newsTitle.clear();
                fairDate = QDate(m_fairMonth.year(), m_fairMonth.month(), day);
                daysInterval = QDate::currentDate().daysTo(fairDate);
                //保存招聘会日历未来一周的链接地址
                if (daysInterval >= 0 && daysInterval <= 7) {
                    m_fairDateMap.insert(fairDate, newsHref);
                }
                if (day == m_fairMonth.daysInMonth()) {
                    m_fairMonth = m_fairMonth.addMonths(1);
                }
            }
        }

//        if (!newsTitle.isEmpty()) {
//            qDebug() << newsTime << newsTitle << newsHref << __FILE__ << __LINE__;
//        }

//        if (!newsTime.isEmpty()) {
//            QDateTime dateTime = QDateTime::fromString(newsTime, "yyyy-MM-dd hh:mm:ss");
//            if (dateTime > m_lastCheckTime) {
//                int row = ui->tableWidget->rowCount();
//                ui->tableWidget->insertRow(row);
//                ui->tableWidget->setItem(row, 0, new QTableWidgetItem(newsTime));
//                ui->tableWidget->setItem(row, 1, new QTableWidgetItem(newsTitle));
//                ui->tableWidget->setItem(row, 2, new QTableWidgetItem("详细"));
//                ui->tableWidget->item(row, 2)->setData(Qt::UserRole, m_webPage.mainFrame()->url().toString() + newsHref);
//            }
//        }

        examineChildElements(element);
        element = element.nextSibling();
    }
}
