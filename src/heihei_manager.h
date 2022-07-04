#ifndef HEIHEI_MANAGER_H
#define HEIHEI_MANAGER_H

#include "esparrag_wifi.h"
#include "esparrag_mqtt.h"
#include "ac_manager.h"
#include "IR_led_strip.h"

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

    Wifi m_wifi;
    MqttClient m_mqtt;
    IrSender m_sender;
    GPO m_ac_led;
    Blinker m_blinker;
    ACManager m_ac;
    IRLedStrip m_strip;
};



#endif // HEIHEI_MANAGER_H