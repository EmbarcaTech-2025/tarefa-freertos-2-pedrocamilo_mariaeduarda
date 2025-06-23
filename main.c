#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"

#define BUTTON_A 5
#define BUTTON_B 6

#define LED_COUNT 25
#define LED_PIN 7

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
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return 24-(y * 5 + x); // Linha par (esquerda para direita).
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
    }
  }

void led_task() {
  const uint LED_PIN = 11;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  while (true) {
    gpio_put(LED_PIN, 1);
    vTaskDelay(100);
    gpio_put(LED_PIN, 0);
    vTaskDelay(100);
  }
}

void buttonB(){
  bool button_b_was_pressed = false;
  while(true){
    bool button_b_state = gpio_get(BUTTON_B); 
    if (!button_b_state && !button_b_was_pressed) { 
      int matriz2[5][5][3] = {
                    {{0, 0, 0}, {0, 0, 0}, {4, 1, 0}, {4, 1, 0}, {0, 0, 0}},
        {{0, 0, 0}, {8, 0, 0}, {4, 1, 0}, {8, 0, 0}, {0, 0, 0}},
        {{8, 0, 0}, {8, 0, 0}, {8, 0, 0}, {8, 0, 0}, {8, 0, 0}},
        {{8, 0, 0}, {8, 0, 0}, {8, 0, 0}, {8, 0, 0}, {8, 0, 0}},
        {{0, 0, 0}, {8, 0, 0}, {8, 0, 0}, {8, 0, 0}, {0, 0, 0}}
                };
            
      for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz2[coluna][linha][0], matriz2[coluna][linha][1], matriz2[coluna][linha][2]);
        }
      }
            
      npWrite();
      vTaskDelay(pdMS_TO_TICKS(300)); 
      npClear();
    }
    else if (button_b_state) {
      button_b_was_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void buttonA(){
  bool button_a_was_pressed = false;
  while(true){
    bool button_a_state = gpio_get(BUTTON_A); 
    if (!button_a_state && !button_a_was_pressed) { 
      int matriz2[5][5][3] = {
                  {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
          {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
          {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
          {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
          {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
              };
            
      for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
          int posicao = getIndex(linha, coluna);
          npSetLED(posicao, matriz2[coluna][linha][0], matriz2[coluna][linha][1], matriz2[coluna][linha][2]);
        }
      }
            
      npWrite();
      vTaskDelay(pdMS_TO_TICKS(300)); 
      npClear();
    }
    else if (button_b_state) {
      button_b_was_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
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

  xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
  xTaskCreate(buttonA, "buttonA", 256, NULL, 1, NULL);
  xTaskCreate(buttonB, "buttonB", 256, NULL, 1, NULL);
  vTaskStartScheduler();

  while(1){};
}