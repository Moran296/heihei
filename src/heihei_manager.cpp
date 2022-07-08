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
    instance = this;
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
    m_ota.Init();
}

void HeiheiManager::Run() {

    esp_log_set_vprintf(heihei_print);
    initConnectivity();

    for(;;) {
        ESPARRAG_LOG_WARNING("heihei manager running....");
        m_ac_led.Set(m_wifi.IsInState<WifiFSM::STATE_Connected>());
        vTaskDelay(Seconds(20).toTicks());
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

bool isEmptyLog(const char* log) {
    return  log[0] == '\0' ||
            strncmp(log, "", 1) == 0 ||
            strncmp(log, "\n", 1) == 0 ||
            strncmp(log, "\r", 1) == 0;
}

void HeiheiManager::sendMqttLog(const char* format, va_list arg) {
    static char sendBuffer[140]{};

    bool isConnected = 
            m_wifi.IsInState<WifiFSM::STATE_Connected>() &&
            m_mqtt.IsInState<MqttFSM::STATE_CONNECTED>();

    if (!isConnected) {
        return;
    }

    int len = vsnprintf(sendBuffer, sizeof(sendBuffer), format, arg);

    if (len < 0 || isEmptyLog(sendBuffer)) {
        return;
    }

    const char* severity{};
    switch(format[7]) {
        case 'E':
            severity = "error";
            break;
        case 'W':
            severity = "warning";
            break;
        case 'I':
            severity = "info";
            break;
        default:
            return;
    }

    const char* payload = strchr(sendBuffer, ')') + 2;

    cJSON* j_log = cJSON_CreateObject();
    cJSON_AddStringToObject(j_log, "device", DEVICE_NAME);
    cJSON_AddStringToObject(j_log, "log", payload);
    cJSON_AddStringToObject(j_log, "level", severity);

    m_mqtt.Publish("/log", j_log);
}

int HeiheiManager::heihei_print(const char* format, va_list arg) {
    int len = vprintf(format, arg);

    HeiheiManager& self = GetInstance();
    self.sendMqttLog(format, arg);
    return len;
}



