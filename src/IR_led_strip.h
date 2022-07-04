#ifndef __IR_LED_STRIP_H__
#define __IR_LED_STRIP_H__

#include "fsm_task.h"
#include "blinker.h"
#include "hei_ir_sender.h"
#include "etl/enum_type.h"
#include "esparrag_common.h"
#include <vector>
#include <array>
#include <etl/string.h>


using led_string_button = std::pair<const char*, uint32_t>;
constexpr std::array<led_string_button, 24> LED_STRIP_BUTTON_ARRAY = {{
        {"BPLUS", 0x00F700FF}, {"BMINUS", 0x00F7807F}, {"OFF", 0x00F740BF}, {"ON", 0x00F7C03F},
        {"R", 0x00F720DF}, {"G", 0x00F7A05F}, {"B", 0x00F7609F}, {"W", 0x00F7E01F},
        {"B1", 0x00F710EF}, {"B2", 0x00F7906F}, {"B3", 0x00F750AF}, {"FLASH",  0x00F7D02F},
        {"B4", 0x00F730CF}, {"B5", 0x00F7B04F}, {"B6", 0x00F7708F}, {"STROBE", 0x00F7F00F},
        {"B7", 0x00F708F7}, {"B8", 0x00F78877}, {"B9", 0x00F748B7}, {"FADE", 0x00F7C837},
        {"B10", 0x00F728D7}, {"B11", 0x00F7A857}, {"B12", 0x00F76897}, {"SMOOTH", 0x00F7E81}
}};


namespace IR_LED_STRIP_FSM {

    struct STATE_MAIN {};
    using States = std::variant<STATE_MAIN>;

    struct EVENT_COMMAND {
        EVENT_COMMAND(const char* button_name);
        EVENT_COMMAND() : m_button{} {}
        std::optional<led_string_button> m_button{};
    };

    using Events = std::variant<EVENT_COMMAND>;
}


class IRLedStrip : public FsmTask<IRLedStrip, IR_LED_STRIP_FSM::States, IR_LED_STRIP_FSM::Events>
{
    static constexpr const char* TASK_NAME = "ir_led_strip";
    static constexpr int TASK_STACK = 4096;
    static constexpr int TASK_PRIORITY = 7;

public:
    IRLedStrip(IrSender& sender, Blinker& blinker);

    //use Dispatch(EVENT_COMMAND{const char*}) to send commands

    void on_entry(IR_LED_STRIP_FSM::STATE_MAIN&) {}

    using return_state_t = std::optional<IR_LED_STRIP_FSM::States>;
    return_state_t on_event(IR_LED_STRIP_FSM::STATE_MAIN&, IR_LED_STRIP_FSM::EVENT_COMMAND&);

    template <class STATE, class EVENT>
    return_state_t on_event(STATE&, EVENT&) {
        printf("invalid event\n");
        return std::nullopt;
    }

private:
    std::vector<unsigned int> generateSignal(uint32_t code);
    IrSender& m_sender;
    Blinker& m_blinker;
};



#endif


