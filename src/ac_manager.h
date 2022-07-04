#ifndef AC_MANAGER_H 
#define AC_MANAGER_H

#include "fsm_task.h"
#include "IRelectra.h"
#include "hei_ir_sender.h"
#include "cJSON.h"
#include "esparrag_gpio.h"
#include "blinker.h"

struct ACSettings {
    IRElectraMode mode;
    IRElectraFan fan; 
    int temperature;
};

namespace AC_FSM {

struct STATE_MAIN {};

using States = std::variant<STATE_MAIN>;


struct EVENT_TOGGLE {};

struct EVENT_COMMAND {
    EVENT_COMMAND(cJSON* payload);

    std::optional<IRElectraMode> mode{};
    std::optional<IRElectraFan> fan{}; 
    std::optional<int> temperature{};
};

using Events = std::variant<EVENT_TOGGLE, EVENT_COMMAND>;


} // namespace


class ACManager : public FsmTask<ACManager, AC_FSM::States, AC_FSM::Events> {

    static constexpr const char* TASK_NAME = "ac_manager";
    static constexpr int TASK_STACK = 8192;
    static constexpr int TASK_PRIORITY = 7;

    public:

    ACManager(IrSender& sender, Blinker& blinker);

    void on_entry(AC_FSM::STATE_MAIN&);

    using return_state_t = std::optional<AC_FSM::States>;

    return_state_t on_event(AC_FSM::STATE_MAIN&, AC_FSM::EVENT_TOGGLE&);
    return_state_t on_event(AC_FSM::STATE_MAIN&, AC_FSM::EVENT_COMMAND&);

    template <class STATE, class EVENT>
    return_state_t on_event(STATE&, EVENT&) {
        printf("invalid event\n");
        return std::nullopt;
    }

    private:

    void command(bool toggle);
    void updateSettings(AC_FSM::EVENT_COMMAND&);

    IrSender m_sender();
    IRelectra m_electra;
    Blinker& m_blinker;
    ACSettings m_settings {
        .mode = IRElectraModeCool,
        .fan = IRElectraFanAuto,
        .temperature = 23
    };
};


#endif