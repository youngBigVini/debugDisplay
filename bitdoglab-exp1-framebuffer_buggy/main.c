#include <stdio.h>

#include <string.h>

#include "pico/stdlib.h"

#include "hardware/i2c.h"

#include "hardware/adc.h"



// Definições do Display OLED (BitDogLab)

#define I2C_PORT i2c1

#define PIN_SDA 2

#define PIN_SCL 3

#define SSD1306_ADDR 0x3C



#define SSD1306_WIDTH 128

#define SSD1306_HEIGHT 64

#define BUFFER_SIZE ((SSD1306_WIDTH * SSD1306_HEIGHT) / 8)



// Definições do Joystick (BitDogLab)

#define ADC_JOYSTICK_X 1 // GPIO 27

#define ADC_JOYSTICK_Y 0 // GPIO 26

#define SQUARE_SIZE 8



void ssd1306_send_cmd(uint8_t cmd) {

    uint8_t buf[2] = {0x00, cmd};

    i2c_write_blocking(I2C_PORT, SSD1306_ADDR, buf, 2, false);

}



void ssd1306_init() {

    uint8_t init_cmds[] = {

        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,

        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,

        0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF

    };

    for (size_t i = 0; i < sizeof(init_cmds); i++) {

        ssd1306_send_cmd(init_cmds[i]);

    }

}



void ssd1306_render(uint8_t *buffer) {

    ssd1306_send_cmd(0x21);

    ssd1306_send_cmd(0);

    ssd1306_send_cmd(SSD1306_WIDTH - 1);



    ssd1306_send_cmd(0x22);

    ssd1306_send_cmd(0);

    ssd1306_send_cmd((SSD1306_HEIGHT / 8) - 1);



    uint8_t payload[1 + BUFFER_SIZE];

    payload[0] = 0x40;

    memcpy(&payload[1], buffer, BUFFER_SIZE);



    i2c_write_blocking(I2C_PORT, SSD1306_ADDR, payload, sizeof(payload), false);

}



// Desenha um único pixel com validação de limites

void draw_pixel(uint8_t *buffer, int x, int y, bool on) {

   //if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) return;

    int index = x + (y / 8) * SSD1306_WIDTH;

    if (on) {

        buffer[index] |= (1 << (y % 8));

    } else {

        buffer[index] &= ~(1 << (y % 8));

    }

}



// Desenha o quadrado de 8x8 pixels

void draw_square(uint8_t *buffer, int x, int y, int size) {

    for (int i = 0; i < size; i++) {

        for (int j = 0; j < size; j++) {

            draw_pixel(buffer, x + i, y + j, true);

        }

    }

}



int main() {

    stdio_init_all();



    // Inicializa I2C1 a 400kHz

    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);

    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(PIN_SDA);

    gpio_pull_up(PIN_SCL);



    // Inicializa o ADC para leitura dos eixos do Joystick

    adc_init();

    adc_gpio_init(26);

    adc_gpio_init(27);



    ssd1306_init();



    uint8_t frame_buffer[BUFFER_SIZE];



    while (true) {

        // Leitura analógica (0 a 4095)

        adc_select_input(ADC_JOYSTICK_X);

        uint16_t raw_x = adc_read();



        adc_select_input(ADC_JOYSTICK_Y);

        uint16_t raw_y = adc_read();



        // Mapeamento dos valores do ADC para os limites seguros da tela

        //int pos_x = ((4095 - raw_x) * (SSD1306_WIDTH - SQUARE_SIZE)) / 4095;

        //int pos_y = ((raw_y) * (SSD1306_HEIGHT - SQUARE_SIZE)) / 4095;

        // Agora as coordenadas vão até 128 e 64 (estourando o tamanho da tela)

        int pos_x = ((4095-raw_x) * SSD1306_WIDTH) / 4095;

        int pos_y = ((raw_y) * SSD1306_HEIGHT) / 4095;

        // Limpa a tela

        memset(frame_buffer, 0x00, BUFFER_SIZE);



        // Desenha o quadrado na posição calculada

        draw_square(frame_buffer, pos_x, pos_y, SQUARE_SIZE);



        // Transmite o quadro para o OLED

        ssd1306_render(frame_buffer);



        sleep_ms(20); // ~50 FPS

    }

} 