#ifndef HEIHEI_MANAGER_H
#define HEIHEI_MANAGER_H

#include "esparrag_wifi.h"
#include "esparrag_mqtt.h"
#include "ac_manager.h"
#include "IR_led_strip.h"
#include "esparrag_fota.h"

class HeiheiManager 
{
public:
    static HeiheiManager& GetInstance();

    HeiheiManager();
    void Run();



private:
    static HeiheiManager* instance;

    void initConnectivity();
    void subscribe_to_mqtt();
    void sendMqttLog(const char* format, va_list arg);
    static int heihei_print(const char* format, va_list arg);

    Wifi m_wifi;
    MqttClient m_mqtt;
    IrSender m_sender;
    GPO m_ac_led;
    Blinker m_blinker;
    ACManager m_ac;
    IRLedStrip m_strip;
    DirectOta m_ota;

};



#endif // HEIHEI_MANAGER_H