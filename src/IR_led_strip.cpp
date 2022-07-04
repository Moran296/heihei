#include "IR_led_strip.h"
#include "esparrag_log.h"
#include <cstring>

using namespace IR_LED_STRIP_FSM;

IRLedStrip::IRLedStrip(IrSender& sender, Blinker& blinker) : FsmTask(TASK_STACK, TASK_PRIORITY, TASK_NAME),
                                           m_sender(sender),
                                           m_blinker(blinker)
{
    Start();
}


EVENT_COMMAND::EVENT_COMMAND(const char* button_name) {

    for (auto button : LED_STRIP_BUTTON_ARRAY) {
        if (strncmp(button.first, button_name, 10) == 0) {
            m_button = button;
            break;
        }
    }
}


IRLedStrip::return_state_t IRLedStrip::on_event(STATE_MAIN&, EVENT_COMMAND &event) {
    if (!event.m_button.has_value()) {
        ESPARRAG_LOG_WARNING("invalid button in ir led strip");
    } else {
        ESPARRAG_LOG_WARNING("Led strip send %s", event.m_button.value().first);

        m_blinker.Start(true);
        std::vector<unsigned int> data = generateSignal(event.m_button.value().second);
        m_sender.SetFrequency(38);
        m_sender.Send(data);
    }

    return std::nullopt;

}

std::vector<unsigned int> IRLedStrip::generateSignal(uint32_t code) {
    std::vector<unsigned int> data{};

    constexpr int SHORT_PULSE = 2250 / 4;
    constexpr int LONG_PULSE = SHORT_PULSE * 3;
    
    data.push_back(9000);
    data.push_back(4500);

    uint8_t *bytes = (uint8_t *)&code;

    ESPARRAG_LOG_INFO("bytes: %d %d %d %d", bytes[0], bytes[1], bytes[2], bytes[3]);

    for (int i = 3; i >= 0 ; i--) {
        for (int j = 7; j >= 0; j--) {
            if (bytes[i] & (1 << j)) {
                data.push_back(SHORT_PULSE);
                data.push_back(LONG_PULSE);
            } else {
                data.push_back(SHORT_PULSE);
                data.push_back(SHORT_PULSE);
            }

        }
    }

    //stop bit
    data.push_back(SHORT_PULSE);
    // Repeat
    data.push_back(48000);
    data.push_back(9000);
    data.push_back(2250);
    data.push_back(SHORT_PULSE);

    return data;
}