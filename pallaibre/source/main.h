#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include <string.h>
#include "ws2818b.pio.h"
#include <time.h>


#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

typedef struct
{
    uint8_t id;
    char descricao[2];
    uint8_t escrita[6];
} Vogal;

typedef struct
{
    Vogal letra;
    float probabilidade;
} Classificador;

typedef struct
{
    char nome_jogador[21];
    uint16_t pontos;
    Vogal resposta_jogador;
    Vogal resposta_gabarito;
} Jogo;

typedef struct
{
    uint8_t R, G, B;
} Pixel;

void gravar_audio(void);
void alterar_status(bool, bool, bool);
void configurar_gpio(void);
void configurar_adc(void);
void configurar_dma(void);
void enviar_amostras_microfone_serial(void);
void remover_componente_dc(void);
void configurar_classificador(void);
int classificar_audio(size_t, size_t, float *);
void classificar_letra(void);
void gerar_letra_aleatoria(void);
void corrigir_jogador(void);
void definir_resposta_jogador(uint8_t, const char *);
void configurar_display_pixel(void);
void colorir_pixel(uint8_t, uint8_t, uint8_t, uint8_t);
void limpar_display_pixel();
void desenhar_display();
void colorir_resposta_gabarito(void);
void colorir_resposta_jogador(void);

#define PINO_LED_AZUL 12
#define PINO_LED_VERDE 11
#define PINO_LED_VERMELHO 13
#define PINO_MICROFONE 28
#define CANAL_MICROFONE 2
#define PINO_BOTAO_A 5
#define PINO_BOTAO_B 6
#define NUMERO_AMOSTRAS_ADC 16000
#define FREQUENCIA_DESEJADA_ADC 16000
#define FREQUENCIA_PADRAO_ADC 48000000
#define OFFSET_DC_ADC 2048
#define NUM_LETRAS 5
#define NUM_PIXEL 25
#define PINO_PIO 7

uint dma_canal;
dma_channel_config dma_cfg;
signal_t sinal;
ei_impulse_result_t resultado;

volatile int16_t adc_buffer[NUMERO_AMOSTRAS_ADC];

bool estado_botao_A = false;
bool estado_anterior_botao_A = false;
bool estado_botao_B = false;
bool estado_anterior_botao_B = false;

Vogal vogais[5] = {{0, "A", {1, 0, 0, 0, 0, 0}},
                   {1, "E", {1, 0, 0, 0, 1, 0}},
                   {2, "I", {0, 1, 0, 1, 0, 0}},
                   {3, "O", {1, 0, 1, 0, 1, 0}},
                   {4, "U", {1, 0, 1, 0, 0, 1}}};

Jogo jogo = {"Jhonatan",
             0,
             {0, {'\0'}, {0, 0, 0, 0, 0, 0}},
             {0, {'\0'}, {0, 0, 0, 0, 0, 0}}};

Pixel display[NUM_PIXEL];
PIO display_pio;
uint sm;
