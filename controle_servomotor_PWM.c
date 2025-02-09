#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO 22
#define LED 12
#define PWM_FREQUENCY 50       // Frequência do PWM (50 Hz)
#define SYSTEM_CLOCK 125000000 // Frequência do clock do sistema (125 MHz)
#define PWM_CLKDIV 64.0f       // Divisor de clock do PWM

// Cálculo do valor de "wrap" baseado no clock e frequência desejada
// PWM_WRAP determina o número máximo que o contador pode atingir antes de reiniciar
#define PWM_WRAP (SYSTEM_CLOCK / (PWM_FREQUENCY * PWM_CLKDIV))

// Definição das larguras de pulso para cada posição do servo (em ticks do PWM)
#define SERVO_0_DEG (uint16_t)(PWM_WRAP * 2.5 / 100)   // Pulso de 0°
#define SERVO_90_DEG (uint16_t)(PWM_WRAP * 7.35 / 100) // Pulso de 90°
#define SERVO_180_DEG (uint16_t)(PWM_WRAP * 12 / 100)  // Pulso de 180°

void set_servo_position(uint slice_num, uint channel, uint16_t pulse_width)
{
    pwm_set_chan_level(slice_num, channel, pulse_width);
}

void set_led_brightness(uint slice_num, uint channel, uint16_t brightness)
{
    pwm_set_chan_level(slice_num, channel, brightness);
}

void configure_pwm(uint gpio)
{
    gpio_set_function(gpio, GPIO_FUNC_PWM);       // Define o pino como função PWM
    uint slice_num = pwm_gpio_to_slice_num(gpio); // Obtém o slice do PWM correspondente ao pino
    uint channel = pwm_gpio_to_channel(gpio);     // Obtém o canal do PWM correspondente ao pino

    pwm_set_clkdiv(slice_num, PWM_CLKDIV);                // Define o divisor de clock
    pwm_set_wrap(slice_num, (uint16_t)PWM_WRAP);          // Define o valor de "wrap"
    pwm_set_chan_level(slice_num, channel, SERVO_90_DEG); // Inicializa o servo na posição 90°
    pwm_set_enabled(slice_num, true);                     // Ativa o PWM
}

int main()
{
    stdio_init_all();
    configure_pwm(SERVO);
    configure_pwm(LED);

    uint slice_num = pwm_gpio_to_slice_num(SERVO);
    uint channel = pwm_gpio_to_channel(SERVO);
    uint slice_num_led = pwm_gpio_to_slice_num(LED);
    uint channel_led = pwm_gpio_to_channel(LED);

    while (1)
    {
        // Posição 180°
        set_servo_position(slice_num, channel, SERVO_180_DEG);
        set_led_brightness(slice_num_led, channel_led, PWM_WRAP);
        sleep_ms(5000);

        // Posição 90°
        set_servo_position(slice_num, channel, SERVO_90_DEG);
        set_led_brightness(slice_num_led, channel_led, PWM_WRAP / 2);
        sleep_ms(5000);

        // Posição 0°
        set_servo_position(slice_num, channel, SERVO_0_DEG);
        set_led_brightness(slice_num_led, channel_led, PWM_WRAP / 4);
        sleep_ms(5000);

        // Transição suave de 0° para 180°
        for (uint16_t pulse = SERVO_0_DEG; pulse <= SERVO_180_DEG; pulse += 5)
        {
            set_servo_position(slice_num, channel, pulse);
            uint16_t brightness = (uint16_t)((pulse - SERVO_0_DEG) * (PWM_WRAP / (SERVO_180_DEG - SERVO_0_DEG)));
            set_led_brightness(slice_num_led, channel_led, brightness);
            sleep_ms(10);
        }

        // Transição suave de 180° para 0°
        for (uint16_t pulse = SERVO_180_DEG; pulse >= SERVO_0_DEG; pulse -= 5)
        {
            set_servo_position(slice_num, channel, pulse);
            uint16_t brightness = (uint16_t)((SERVO_180_DEG - pulse) * (PWM_WRAP / (SERVO_180_DEG - SERVO_0_DEG)));
            set_led_brightness(slice_num_led, channel_led, brightness);
            sleep_ms(10);
        }
    }
}
