#ifndef HEI_IR_SENDER_H__
#define HEI_IR_SENDER_H__

#include "esparrag_pwm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <vector>

class IrSender {
public:
    IrSender(int pin, int channel, int carrier_freq_khz);

    void Send(std::vector<unsigned int>& data);
    void SetFrequency(int freq_khz);
    void Space(uint64_t us);
    void Mark(uint64_t us);
    void Stop();

private:
    Pwm m_pwm;
    SemaphoreHandle_t m_mutex;
};


#endif
