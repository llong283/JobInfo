#ifndef SETTING_H
#define SETTING_H

#include <QObject>
#include "config.h"

#define g_setting Setting::instance()

class Setting : public QObject
{
    Q_OBJECT
public:
    static Setting *instance();   
    void updateSetting();    
    inline bool getIsCloseMin() {
        return m_isCloseMin;
    }
    inline bool getIsStartMin() {
        return m_isStartMin;
    }
    inline int getRecordDays() {
        return m_recordDays;
    }
    inline int getAheadDays() {
        return m_aheadDays;
    }
    inline int getIntervalHours() {
        return m_intervalHours;
    }
    
signals:
    void signSettingChanged(bool, bool, bool);

private:
    Setting(QObject *parent = 0);
    static Setting *setting;
    QString m_settingPath;
    bool m_isCloseMin;
    bool m_isStartMin;    
    int m_recordDays;
    int m_aheadDays;
    int m_intervalHours;
};

#endif // SETTING_H
