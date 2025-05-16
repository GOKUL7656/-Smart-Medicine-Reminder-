/*  main.c  —  Smart Medicine Reminder (via ESP-IDF, no Arduino) */

#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

/*------------------------------------------------------------------*/
/* GPIO assignments                                                 */
#define LED_GPIO     GPIO_NUM_2      // on-board LED (active HIGH)
#define BUZZER_GPIO  GPIO_NUM_14     // passive buzzer (active HIGH)
#define BTN_GPIO     GPIO_NUM_4      // push-button to confirm dose

/* Reminder timing (simulation friendly) */
#define REMINDER_PERIOD_MS   15000   // time between dose reminders
#define ACK_WINDOW_MS        10000   // user must press button within this window

/*------------------------------------------------------------------*/
static volatile bool alert_active = false;      // “take medicine” alert is sounding
static volatile uint64_t alert_start_ms = 0;    // when current alert began

/* Return elapsed ms since boot */
static inline uint64_t millis(void)
{
    return (uint64_t) (esp_timer_get_time() / 1000ULL);
}

/*------------------------------------------------------------------*/
/* GPIO helper                                                      */
static void gpio_set_level_fast(gpio_num_t pin, int level)
{
    gpio_set_level(pin, level);   // thin wrapper; keeps code readable
}

/*------------------------------------------------------------------*/
/* Button ISR: clears alert when user presses the button            */
static void IRAM_ATTR button_isr(void *arg)
{
    if (!alert_active) return;               // Ignore presses outside alert
    alert_active = false;                    // Stop alert
    gpio_set_level_fast(LED_GPIO, 0);
    gpio_set_level_fast(BUZZER_GPIO, 0);
}

/*------------------------------------------------------------------*/
/* Task: generates a reminder every REMINDER_PERIOD_MS              */
static void reminder_task(void *arg)
{
    uint64_t last_alarm_ms = millis();

    while (true) {
        uint64_t now = millis();

        /* Time for next reminder? */
        if (!alert_active && (now - last_alarm_ms) >= REMINDER_PERIOD_MS) {
            alert_active   = true;
            alert_start_ms = now;
            last_alarm_ms  = now;
            printf("\n[REMINDER] Time to take your medicine!\n");

            /* Start buzzer + LED */
            gpio_set_level_fast(LED_GPIO, 1);
            gpio_set_level_fast(BUZZER_GPIO, 1);
        }

        /* If alert is active, but user hasn't acknowledged within window */
        if (alert_active && (now - alert_start_ms) >= ACK_WINDOW_MS) {
            alert_active = false;                     // stop alert anyway
            gpio_set_level_fast(LED_GPIO, 0);
            gpio_set_level_fast(BUZZER_GPIO, 0);
            printf("[WARNING] Dose was MISSED!\n");
        }

        vTaskDelay(pdMS_TO_TICKS(100));               // 100 ms scheduler tick
    }
}

/*------------------------------------------------------------------*/
/* Application entry                                                */
void app_main(void)
{
    /* Configure GPIOs */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO) | (1ULL << BUZZER_GPIO),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    /* Button as input with pull-up + interrupt on falling edge */
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BTN_GPIO),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE
    };
    gpio_config(&btn_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN_GPIO, button_isr, NULL);

    printf("Smart Medicine Reminder started — press button on GPIO 4 to confirm dose.\n");
    xTaskCreate(reminder_task, "reminder_task", 2048, NULL, 5, NULL);
}
