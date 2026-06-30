#include "pico_fastpin.h"


pico_OutputPin_t::pico_OutputPin_t() : _pin(255), _slice(0), _chan(0), _is_pwm_active(false), _current_freq(0) {}

void pico_OutputPin_t::setPin(uint8_t pin, uint8_t mode) {
    if (pin != 255) {
        _pin = pin;
        _slice = pwm_gpio_to_slice_num(_pin);
        _chan = pwm_gpio_to_channel(_pin);
        _is_pwm_active = false;
        _current_freq = 0;
        pinMode(_pin, mode);
    } else {
        _pin = 255;
    }
}

void pico_OutputPin_t::writePWM(uint8_t duty_255, uint32_t target_freq, bool invert) {
    if (isValid()) {
        // Ambil nilai frekuensi aman jika config mengirim angka 0 atau terlalu rendah
        if (target_freq < 10) { target_freq = 200; }

        // Jika PWM belum aktif, atau ada perubahan nilai frekuensi dari TunerStudio runtime,
        // inisialisasi ulang hardware register PWM secara dinamis.
        if (!_is_pwm_active || (_current_freq != target_freq)) {
            gpio_set_function(_pin, GPIO_FUNC_PWM);
            
            uint32_t sys_clock = clock_get_hz(clk_sys);
            uint16_t wrap_val = 10000; // Resolusi dasar 0-10000
            pwm_set_wrap(_slice, wrap_val);

            float calculated_div = (float)sys_clock / ((float)target_freq * (wrap_val + 1));
            if (calculated_div < 1.0f) { calculated_div = 1.0f; }
            if (calculated_div > 255.0f) { calculated_div = 255.0f; }

            pwm_set_clkdiv(_slice, calculated_div);
            pwm_set_enabled(_slice, true);
            
            _is_pwm_active = true;
            _current_freq = target_freq;
        }

        // Terapkan Logika Inversi secara instan berdasarkan configPage6
        if (invert) {
            duty_255 = 255 - duty_255;
        }

        // Konversi skala cepat ke resolusi hardware 0-10000 menggunakan fixed-point multiplication
        uint32_t target_level = ((uint32_t)duty_255 * 2573) >> 16;
        pwm_set_chan_level(_slice, _chan, target_level);
    }
}

void pico_OutputPin_t::disablePWM(void) {
    if (isValid() && _is_pwm_active) {
        pwm_set_chan_level(_slice, _chan, 0);
        gpio_set_function(_pin, GPIO_FUNC_SIO);
        digitalWriteFast(_pin, LOW);
        _is_pwm_active = false;
        _current_freq = 0;
    }
}

void pico_InputPin_t::setPin(uint8_t pin, uint8_t mode) {
    if (pin != 255) {
        _pin = pin;
        pinMode(_pin, mode);
    } else {
        _pin = 255;
    }
}
