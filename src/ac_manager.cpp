#include "ac_manager.h"    
#include <cstring>
#include "esparrag_log.h"

using namespace AC_FSM;



ACManager::ACManager(IrSender& sender, Blinker& blinker) : FsmTask(TASK_STACK, TASK_PRIORITY, TASK_NAME),
                         m_electra(sender),
                         m_blinker(blinker)
{
    Start();
}

void ACManager::on_entry(STATE_MAIN&) { }

using return_state_t = std::optional<AC_FSM::States>;

return_state_t ACManager::on_event(STATE_MAIN&, EVENT_TOGGLE&) {
    command(true);

    return std::nullopt;
}

return_state_t ACManager::on_event(STATE_MAIN&, EVENT_COMMAND &event) {
    updateSettings(event);
    command(false);

    return std::nullopt;
}


void ACManager::updateSettings(EVENT_COMMAND&event) {
    if (event.mode) {
        ESPARRAG_LOG_INFO("SETTING MODE TO %d", *event.mode);
        m_settings.mode = *event.mode;
    }
    if (event.fan) {
        ESPARRAG_LOG_INFO("SETTING FAN TO %d", *event.fan);
        m_settings.fan = *event.fan;
    }
    if (event.temperature) {
        ESPARRAG_LOG_INFO("SETTING TEMPERATURE TO %d", *event.temperature);
        m_settings.temperature = *event.temperature;
    }
}

void ACManager::command(bool toggle) {
    if (toggle) 
        ESPARRAG_LOG_INFO("Toggling AC");

    m_blinker.Start(true);
    m_electra.sendElectra(toggle, m_settings.mode, m_settings.fan, m_settings.temperature, false, false);
}

// ============================= OTHERS ========================================

std::optional<IRElectraMode> stringToMode(const char* str) {
    if (strcmp(str, "cool") == 0) {
        return IRElectraModeCool;
    } else if (strcmp(str, "heat") == 0) {
        return IRElectraModeHeat;
    } else if (strcmp(str, "auto") == 0) {
        return IRElectraModeAuto;
    } else if (strcmp(str, "dry") == 0) {
        return IRElectraModeDry;
    } else if (strcmp(str, "fan") == 0) {
        return IRElectraModeFan;
    } else {
        return std::nullopt;
    }
}

std::optional<IRElectraFan> stringToFan(const char* str) {
    if (strcmp(str, "auto") == 0) {
        return IRElectraFanAuto;
    } else if (strcmp(str, "low") == 0) {
        return IRElectraFanLow;
    } else if (strcmp(str, "medium") == 0) {
        return IRElectraFanMedium;
    } else if (strcmp(str, "high") == 0) {
        return IRElectraFanHigh;
    } else {
        return std::nullopt;
    }
}


EVENT_COMMAND::EVENT_COMMAND(cJSON* payload) {

    cJSON* j_mode = cJSON_GetObjectItem(payload, "mode");
    cJSON* j_temperature = cJSON_GetObjectItem(payload, "temperature");
    cJSON* j_fan = cJSON_GetObjectItem(payload, "fan");

    if (j_mode && j_mode->type == cJSON_String) {
        mode = stringToMode(cJSON_GetStringValue(j_mode));
    }

    if (j_temperature && j_temperature->type == cJSON_Number) {
        temperature = j_temperature->valueint;
    }

    if (j_fan && j_fan->type == cJSON_String) {
        fan = stringToFan(cJSON_GetStringValue(j_fan));
    }
}