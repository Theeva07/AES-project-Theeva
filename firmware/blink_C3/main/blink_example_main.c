#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

// -----------------------------------------------------
// CONFIG: Change pins here if your wiring is different
// -----------------------------------------------------

// XIAO ESP32-C3 mapping reminder:
// D0=GPIO2, D1=GPIO3, D2=GPIO4, D3=GPIO5, D4=GPIO6, D5=GPIO7,
// D6=GPIO8, D7=GPIO9, D8=GPIO18, D9=GPIO19, D10=GPIO10

#define LED_PIN        4    // D2  -> LED
#define BUZZER_PIN     5    // D3  -> Buzzer
#define BUTTON_PIN     18   // D8  -> Push button (to GND, use internal pull-up)

// Reminder duration options (ms)
static const uint32_t reminder_options_ms[] = {
    10000,   // 10 s
    30000,   // 30 s
    60000,   // 60 s
    0        // OFF
};
#define NUM_REMINDER_OPTIONS  (sizeof(reminder_options_ms)/sizeof(reminder_options_ms[0]))

// Button timing (ms)
#define BUTTON_POLL_MS        20
#define LONG_PRESS_THRESHOLD  1200    // >1.2 s = long press

static const char *TAG = "AES_REMINDER";

// -----------------------------------------------------
// OLED support (currently stubbed so always compiles)
// If you later wire the OLED and fix I2C includes, set to 1
// -----------------------------------------------------
#define USE_OLED   0

static void oled_show_message(const char *line1, const char *line2)
{
#if USE_OLED
    // TODO: implement I2C + SSD1306 here when ready.
    // For now we just log to console even if USE_OLED==1.
#endif
    ESP_LOGI(TAG, "OLED: %s | %s", line1 ? line1 : "", line2 ? line2 : "");
}

// -----------------------------------------------------
// "Cloud" simulation
// -----------------------------------------------------
static void cloud_push_event(const char *msg)
{
    ESP_LOGI("CLOUD", "[SIMULATED CLOUD] %s", msg);
}

// -----------------------------------------------------
// Shared state
// -----------------------------------------------------

typedef enum {
    BUTTON_EVENT_SHORT,
    BUTTON_EVENT_LONG
} button_event_type_t;

typedef struct {
    button_event_type_t type;
} button_event_t;

static QueueHandle_t button_event_queue = NULL;

// current index in reminder_options_ms
static int current_option_index = 0;

// esp_timer handle
static esp_timer_handle_t reminder_timer = NULL;

// flag to indicate reminder alert active
static volatile bool alert_active = false;

// -----------------------------------------------------
// GPIO helpers
// -----------------------------------------------------
static void gpio_init_all(void)
{
    // LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    // Buzzer
    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_PIN, 0);

    // Button with internal pull-up
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
}

// -----------------------------------------------------
// Reminder timer callback (runs in timer task context)
// -----------------------------------------------------
static void reminder_timer_callback(void *arg)
{
    // When timer expires, activate alert
    alert_active = true;

    char msg[64];
    snprintf(msg, sizeof(msg), "Reminder alert fired (%u ms)",
             (unsigned)reminder_options_ms[current_option_index]);
    cloud_push_event(msg);

    oled_show_message("Reminder ALERT", "Press & hold to cancel");
    ESP_LOGI(TAG, "Reminder timer expired -> alert_active = true");
}

// -----------------------------------------------------
// Button task: debounced button + short/long press
// -----------------------------------------------------
static void button_task(void *pvParameters)
{
    (void)pvParameters;

    bool last_level = gpio_get_level(BUTTON_PIN); // 1 when not pressed (pull-up)
    int press_ticks = 0;
    bool pressed = false;

    while (1) {
        bool level = gpio_get_level(BUTTON_PIN);

        if (!pressed) {
            // waiting for press
            if (last_level == 1 && level == 0) {
                // falling edge -> button pressed
                pressed = true;
                press_ticks = 0;
            }
        } else {
            // button currently pressed
            if (level == 0) {
                // still held
                press_ticks += BUTTON_POLL_MS;
            } else {
                // released -> determine short vs long
                button_event_t evt;
                if (press_ticks >= LONG_PRESS_THRESHOLD) {
                    evt.type = BUTTON_EVENT_LONG;
                } else {
                    evt.type = BUTTON_EVENT_SHORT;
                }
                xQueueSend(button_event_queue, &evt, 0);
                pressed = false;
            }
        }

        last_level = level;
        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

// -----------------------------------------------------
// Alert task: drives LED + buzzer while alert_active=true
// -----------------------------------------------------
static void alert_task(void *pvParameters)
{
    (void)pvParameters;

    while (1) {
        if (alert_active) {
            // Simple beep pattern: LED + buzzer ON 200 ms, OFF 200 ms
            gpio_set_level(LED_PIN, 1);
            gpio_set_level(BUZZER_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(200));

            gpio_set_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            // LED stays on continuously while alert is active
        } else {
            // Ensure everything OFF
            gpio_set_level(LED_PIN, 0);
            gpio_set_level(BUZZER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// -----------------------------------------------------
// Control task: handles button events, modes, timer
// -----------------------------------------------------
static void control_task(void *pvParameters)
{
    (void)pvParameters;

    // Initial OLED + logs
    oled_show_message("AES Reminder", "Short press to set");
    ESP_LOGI(TAG, "Control task started. Initial mode = 10 s reminder.");

    while (1) {
        button_event_t evt;
        if (xQueueReceive(button_event_queue, &evt, portMAX_DELAY)) {
            if (evt.type == BUTTON_EVENT_SHORT) {
                // Cycle reminder options
                current_option_index =
                    (current_option_index + 1) % NUM_REMINDER_OPTIONS;
                uint32_t dur = reminder_options_ms[current_option_index];

                if (dur == 0) {
                    // OFF mode
                    if (esp_timer_is_active(reminder_timer)) {
                        esp_timer_stop(reminder_timer);
                    }
                    alert_active = false;
                    oled_show_message("Reminder OFF", "Short press to set");
                    ESP_LOGI(TAG, "Reminder turned OFF.");
                } else {
                    // Start / restart timer with new duration
                    if (esp_timer_is_active(reminder_timer)) {
                        esp_timer_stop(reminder_timer);
                    }
                    esp_timer_start_once(reminder_timer, dur * 1000ULL); // ms -> us

                    char line2[32];
                    snprintf(line2, sizeof(line2), "Trigger in %u s",
                             (unsigned)(dur / 1000));
                    oled_show_message("Reminder SET", line2);
                    ESP_LOGI(TAG, "Reminder set for %u ms", (unsigned)dur);
                }
            } else if (evt.type == BUTTON_EVENT_LONG) {
                // Long press -> cancel everything
                if (esp_timer_is_active(reminder_timer)) {
                    esp_timer_stop(reminder_timer);
                }
                alert_active = false;
                oled_show_message("Reminder CANCEL", "Short press to set again");
                ESP_LOGI(TAG, "Reminder cancelled by long press.");
            }
        }
    }
}

// -----------------------------------------------------
// app_main
// -----------------------------------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "AES Project - Smart Reminder System starting...");

    gpio_init_all();

    // Create queue
    button_event_queue = xQueueCreate(8, sizeof(button_event_t));
    if (button_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create button_event_queue");
        return;
    }

    // Create reminder timer (one-shot)
    const esp_timer_create_args_t timer_args = {
        .callback = &reminder_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "reminder_timer"
    };
    esp_err_t err = esp_timer_create(&timer_args, &reminder_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create failed: %s", esp_err_to_name(err));
        return;
    }

    // Create tasks
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    xTaskCreate(alert_task,  "alert_task",  2048, NULL, 4, NULL);
    xTaskCreate(control_task,"control_task",3072, NULL, 6, NULL);

    ESP_LOGI(TAG, "System initialised. "
                  "Short press = cycle reminder, Long press = cancel.");
}

    
