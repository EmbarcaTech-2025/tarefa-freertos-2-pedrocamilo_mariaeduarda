#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"
#include "hardware/i2c.h"
#include "timers.h"
#include "FreeRTOSConfig.h"
#include "pico/time.h" 
#include <stdlib.h>
#include "hardware/pwm.h"
#include "hardware/clocks.h"

//botões
#define BUTTON_A 5
#define BUTTON_B 6

//matriz de led
#define LED_COUNT 25
#define LED_PIN 7

//buzzer
#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 100
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_G3  196 

uint8_t obstacles[5][5] = {0};

int player_pos = 2;
int perdeu = 0;

struct pixel_t {
  uint8_t G, R, B; 
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; 

npLED_t leds[LED_COUNT];

PIO np_pio;
uint sm;

void npInit(uint pin) {

  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); 
  }

  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); 
}

int getIndex(int x, int y) {
  if (y % 2 == 0) {
      return 24-(y * 5 + x); 
  } else {
      return 24-(y * 5 + (4 - x)); 
  }
}

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); 
    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(pin, 0);
}

void pwm_set_freq_duty(uint slice_num, uint chan, uint32_t freq, int duty) {
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / freq / 4096 + (clock % (freq * 4096) != 0);
    if (divider16 / 16 == 0)
        divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / freq - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16 / 16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * duty / 100);
}

void beep(int freq, int duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint chan = pwm_gpio_to_channel(BUZZER_PIN);

    pwm_set_freq_duty(slice_num, chan, freq, 50);
    pwm_set_enabled(slice_num, true);

    vTaskDelay(duration_ms);

    // Desliga o som
    pwm_set_enabled(slice_num, false);
}

void buttonB(void *params){
  bool button_b_was_pressed = false;
  while(true){
    bool button_b_state = gpio_get(BUTTON_B); 
    if (!button_b_state && !button_b_was_pressed) { 
      if (player_pos < 4) player_pos++;
      button_b_was_pressed = true;
    }
    else if (button_b_state) {
      button_b_was_pressed = false;
    }
    vTaskDelay(100);
  }
}

void buttonA(void *params){
  bool button_a_was_pressed = false;
  while(true){
    bool button_a_state = gpio_get(BUTTON_A); 
    if (!button_a_state && !button_a_was_pressed) { 
      if (player_pos > 0) player_pos--;
      button_a_was_pressed = true;
    }
    else if (button_a_state) {
      button_a_was_pressed = false;
    }
    vTaskDelay(100); 
  }
}

void render_task(void *params) {
  while(true){
    npClear();
    for (int y = 0; y < 5; y++) {
      int count = 0;
      for (int x = 0; x < 5; x++) {
        if (obstacles[y][x]) {
          if(count < 2){
            int pos = getIndex(y, x);
            npSetLED(pos, 8, 0, 0);  
            count ++;
          }
        }
      }
    }
    if (obstacles[player_pos][4]) {
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < LED_COUNT; j++) {
          npSetLED(j, 8, 0, 0);
        }
        npWrite();
        beep(BUZZER_PIN, 500);
        vTaskDelay(400);

        npClear();
        npWrite();
        vTaskDelay(100);
      }
      for (int y = 0; y < 5; y++)
        for (int x = 0; x < 5; x++)
          obstacles[y][x] = 0;
      player_pos = 2;
    }
    int pos = getIndex(player_pos, 4);
    npSetLED(pos,0,8,0);
    npWrite();
    vTaskDelay(30);
  }
}

void obstacle_task(void *params) {
    while (true) {
        for (int y = 4; y > 0; y--) {
            for (int x = 0; x < 5; x++) {
                obstacles[x][y] = obstacles[x][y - 1];
            }
        }

        for (int x = 0; x < 5; x++) {
            obstacles[x][0] = 0;
        }

        int num_obstacles = rand() % 3; 

        for (int i = 0; i < num_obstacles; i++) {
            int pos;
            do {
                pos = rand() % 5; 
            } while (obstacles[pos][0] == 1); 
            
            obstacles[pos][0] = 1;
        }

        vTaskDelay(600);
    }
}

int main() {
  stdio_init_all();
  npInit(LED_PIN);
  npWrite();
  npClear();
  gpio_init(BUTTON_A);
  gpio_set_dir(BUTTON_A, GPIO_IN);
  gpio_pull_up(BUTTON_A);
  gpio_init(BUTTON_B);
  gpio_set_dir(BUTTON_B, GPIO_IN);
  gpio_pull_up(BUTTON_B);
  srand(to_us_since_boot(get_absolute_time()));
  pwm_init_buzzer(BUZZER_PIN);

  xTaskCreate(buttonA, "buttonA", 256, NULL, 1, NULL);
  xTaskCreate(buttonB, "buttonB", 256, NULL, 1, NULL);
  xTaskCreate(render_task, "Render", 512, NULL, 1, NULL);
  xTaskCreate(obstacle_task, "Obstacles", 512, NULL, 1, NULL);
  vTaskStartScheduler();

  while(1){};
}