#include <cstdlib>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

void gravar_audio(void);
void alterar_status(bool, bool, bool);
void configurar_gpio(uint8_t);
void configurar_adc(uint8_t, uint8_t);
void configurar_dma(void);
void enviar_amostras_microfone_serial(void);
void remover_componente_dc(void);
void configurar_classificador(void);
int classificar_audio(size_t, size_t, float *);
void classificar_letra(void);
void gerar_letra_aleatoria(void);

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
#define NUM_LETRAS 2

typedef struct
{
    uint8_t id;
    char descricao[2];
} Vogal;

typedef struct
{
    Vogal letra;
    float probabilidade;
} Classificador;

Classificador letra_classificada = {{0, ""}, -1};
Vogal letra_gerada = {0, ""};
Vogal vogais[5] = {{0, "A"}, {1, "E"}, {2, "I"}, {3, "O"}, {4, "U"}};

Classificador letras_exemplo[2] = {{{0, "A"}, 0.3}, {{1, "B"}, 0.7}};

uint dma_canal;
dma_channel_config dma_cfg;
signal_t sinal;
ei_impulse_result_t resultado;

volatile int16_t adc_buffer[NUMERO_AMOSTRAS_ADC];

bool estado_botao_A = false;
bool estado_anterior_botao_A = false;
bool estado_botao_B = false;
bool estado_anterior_botao_B = false;

int main()
{
    stdio_init_all();
    configurar_gpio(PINO_LED_AZUL);
    configurar_gpio(PINO_LED_VERMELHO);
    configurar_gpio(PINO_LED_VERDE);
    configurar_adc(PINO_MICROFONE, CANAL_MICROFONE);
    configurar_dma();
    configurar_classificador();

    while (true)
    {

        estado_botao_A = !gpio_get(PINO_BOTAO_A);
        estado_botao_B = !gpio_get(PINO_BOTAO_B);

        if (estado_botao_A && estado_botao_A != estado_anterior_botao_A)
        {
            gravar_audio();
            gerar_letra_aleatoria();
            classificar_letra();
            printf("Gerada - > Id: %d | Letra: %s\n", letra_gerada.id, letra_gerada.descricao);
            printf("Classificada -> Id: %d | Letra: %s | Probabilidade: %.2f\n", letra_classificada.letra.id, letra_classificada.letra.descricao, letra_classificada.probabilidade);
        }
        else if (estado_botao_B && estado_botao_B != estado_anterior_botao_B)
        {
            gravar_audio();
            enviar_amostras_microfone_serial();
        }

        estado_anterior_botao_A = estado_botao_A;
        estado_anterior_botao_B = estado_botao_B;

        sleep_ms(100);
    }
}

void gerar_letra_aleatoria()
{
    uint8_t id_aleatorio = rand() % NUM_LETRAS;
    letra_gerada.id = vogais[id_aleatorio].id;
    strcpy(letra_gerada.descricao, vogais[id_aleatorio].descricao);
}
void classificar_letra()
{
    int res = run_classifier(&sinal, &resultado, false);

    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
    {
        printf("%s: %.2f\n", resultado.classification[i].label, resultado.classification[i].value);
        if (letra_classificada.probabilidade < resultado.classification[i].value)
        {
            letra_classificada.letra.id = i;
            strcpy(letra_classificada.letra.descricao, resultado.classification[i].label);
            letra_classificada.probabilidade = resultado.classification[i].value;
        }
    }
}

void configurar_classificador()
{
    sinal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    sinal.get_data = classificar_audio;
    printf("Classificador configurado.\n");
}

int classificar_audio(size_t offset, size_t quantidade_amostras, float *classificados)
{
    for (size_t i = 0; i < quantidade_amostras; i++)
    {
        classificados[i] = adc_buffer[i]; // Substituir por leitura real do ADC
    }
    return 0;
}

void enviar_amostras_microfone_serial()
{

    for (uint32_t i = 0; i < NUMERO_AMOSTRAS_ADC; i++)
    {
        printf("%d\n", adc_buffer[i]);
    }
}

void gravar_audio()
{
    adc_fifo_drain();
    adc_run(false);
    alterar_status(true, false, false);
    sleep_ms(2000);
    dma_channel_configure(dma_canal, &dma_cfg,
                          adc_buffer,
                          &(adc_hw->fifo),
                          NUMERO_AMOSTRAS_ADC,
                          true);
    alterar_status(false, true, false);
    sleep_ms(500);
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_canal);
    adc_run(false);
    remover_componente_dc();
    sleep_ms(500);
    alterar_status(false, false, false);
}

void remover_componente_dc()
{
    for (int i = 0; i < NUMERO_AMOSTRAS_ADC; i++)
    {
        adc_buffer[i] = (int16_t)(adc_buffer[i] - OFFSET_DC_ADC);
    }
}

void alterar_status(bool vermelho, bool verde, bool azul)
{
    gpio_put(PINO_LED_VERMELHO, vermelho);
    gpio_put(PINO_LED_VERDE, verde);
    gpio_put(PINO_LED_AZUL, azul);
}

void configurar_adc(uint8_t pino, uint8_t canal)
{
    uint32_t divisor = FREQUENCIA_PADRAO_ADC / (FREQUENCIA_DESEJADA_ADC);
    adc_init();
    adc_gpio_init(pino);
    adc_set_clkdiv(divisor);
    adc_select_input(canal);
    adc_fifo_setup(true, true, 1, false, false);
    printf("ADC configurado.\n");
}
void configurar_gpio(uint8_t porta)
{
    gpio_init(porta);
    gpio_set_dir(porta, GPIO_OUT);
    gpio_put(porta, false);

    gpio_init(PINO_BOTAO_A);
    gpio_pull_up(PINO_BOTAO_A);
    gpio_set_dir(PINO_BOTAO_A, GPIO_IN);

    gpio_init(PINO_BOTAO_B);
    gpio_pull_up(PINO_BOTAO_B);
    gpio_set_dir(PINO_BOTAO_B, GPIO_IN);
    printf("GPIO configurado.\n");
}

void configurar_dma()
{
    dma_canal = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_canal);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
    printf("DMA configurado.\n");
}