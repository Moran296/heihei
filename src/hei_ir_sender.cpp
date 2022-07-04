#include "hei_ir_sender.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "lock.h"



IrSender::IrSender(int pin, int channel, int carrier_freq_khz) : 
m_pwm((gpio_num_t)pin, LEDC_LOW_SPEED_MODE, LEDC_TIMER_8_BIT, LEDC_TIMER_0, carrier_freq_khz * 1000, (ledc_channel_t)channel) {

    m_mutex = xSemaphoreCreateMutex();
    assert(m_mutex != NULL);
}

void IrSender::SetFrequency(int freq_khz) {
    Lock lock(m_mutex);
    m_pwm.SetFrequency(freq_khz * 1000);
}

void IrSender::Space(uint64_t us) {
    m_pwm.SetDutyCycle(0);
    ets_delay_us(us);
}

void IrSender::Mark(uint64_t us) {
    m_pwm.SetDutyCycle(0xff / 2);
    ets_delay_us(us);
}

void IrSender::Stop() {
    m_pwm.SetDutyCycle(0);
}

void IrSender::Send(std::vector<unsigned int>& data) {

    Lock lock(m_mutex);
    for (int i = 0; i < data.size(); i++) {
        if (i % 2 == 0) {
            Mark(data[i]);
        } else {
            Space(data[i]);
        }
    }

    Stop();
}











