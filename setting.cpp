#include "setting.h"

Setting *Setting::setting = NULL;

Setting::Setting(QObject *parent) :
    QObject(parent)
{
    m_settingPath = QApplication::applicationDirPath() + "/JobInfo/setting.ini";
    updateSetting();
}

Setting *Setting::instance()
{
    if (setting == NULL) {
        setting = new Setting;
    }
    return setting;
}

void Setting::updateSetting()
{  
    QSettings setting(m_settingPath, QSettings::IniFormat);
    m_isStartMin = setting.value("startmin", true).toBool();
    m_isCloseMin = setting.value("closemin", true).toBool();
    int recordDays = setting.value("recorddays", 3).toInt();
    int aheadDays = setting.value("aheaddays", 3).toInt();
    int intervalHours = setting.value("intervalhours", 3).toInt();
    bool isRecordChanged = recordDays!=m_recordDays;
    bool isAheadChanged = aheadDays!=m_aheadDays;
    bool isIntervalChanged = intervalHours!=m_intervalHours;
    
    m_recordDays = recordDays;
    m_aheadDays = aheadDays;
    m_intervalHours = intervalHours;
    
    emit signSettingChanged(isRecordChanged, isAheadChanged, isIntervalChanged);    
}
