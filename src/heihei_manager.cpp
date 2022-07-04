#include "heihei_manager.h"
#include "esparrag_time_units.h"
#include "esparrag_log.h"
#include "esparrag_mdns.h"
#include "IR_led_strip.h"
#include "secret.h"

HeiheiManager* HeiheiManager::instance{};


HeiheiManager& HeiheiManager::GetInstance() {
    return *instance;
}


HeiheiManager::HeiheiManager() : m_sender(IR_PIN, 0, 38),
                                 m_ac_led(AC_LED_PIN),
                                 m_blinker(m_ac_led, 80, 80, 6),
                                 m_ac(m_sender, m_blinker),
                                 m_strip(m_sender, m_blinker) 
{
    m_mqtt.Init();
    m_wifi.Init();
    configASSERT(Mdns::Init());
}

void HeiheiManager::initConnectivity() {
    m_wifi.Connect(SSID, PASS);
    while(!m_wifi.IsInState<WifiFSM::STATE_Connected>()) {

        vTaskDelay(Seconds(2).toTicks());
        ESPARRAG_LOG_INFO("connecting to wifi.......");
    } 

    EsparragResult brokerIP = Mdns::FindBroker();
    if (brokerIP.IsError()) {
        ESPARRAG_LOG_ERROR("couldn't find broker");
        PRINT_ERROR_RES(brokerIP);
    } else {
        m_mqtt.TryConnect(brokerIP.ok_or_assert());
    }

    while(!m_mqtt.IsInState<MqttFSM::STATE_CONNECTED>()) {
        vTaskDelay(Seconds(2).toTicks());
        ESPARRAG_LOG_INFO("connecting to mqtt.......");
    }

    subscribe_to_mqtt();
}

void HeiheiManager::Run() {

    initConnectivity();

    for(;;) {
        m_ac_led.Set(m_wifi.IsInState<WifiFSM::STATE_Connected>());
        vTaskDelay(Seconds(60).toTicks());
    }
}

void HeiheiManager::subscribe_to_mqtt() {
    m_mqtt.On("/ac/command", [this](auto topic, cJSON* payload) {
        ESPARRAG_LOG_INFO("heihei got mqtt - command for ac");
        m_ac.Dispatch(AC_FSM::EVENT_COMMAND{payload});
    });
    m_mqtt.On("/ac/toggle", [this](auto topic, cJSON* payload) {
        ESPARRAG_LOG_INFO("heihei got mqtt - toggle for ac");
        m_ac.Dispatch(AC_FSM::EVENT_TOGGLE{});
    });
    m_mqtt.On("/strip/command", [this](auto topic, cJSON* payload) {
        ESPARRAG_LOG_INFO("heihei got mqtt - command for strip");

        cJSON* j_button = cJSON_GetObjectItem(payload, "button");
        if (j_button && j_button->type == cJSON_String) {
            m_strip.Dispatch(IR_LED_STRIP_FSM::EVENT_COMMAND{j_button->valuestring});
        } else {
            ESPARRAG_LOG_ERROR("heihei got mqtt - command for strip - no button");
        }
    });
}





