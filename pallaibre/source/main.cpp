#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

#define LED_AZUL_PINO 12
#define LED_VERDE_PINO 11
#define LED_VERMELHO_PINO 13
#define MICROFONE_PINO 28
#define MICROFONE_CANAL 2
#define BOTAO_A 5

#define AMOSTRAS_ADC 16000
#define FREQ_ADC 16000
#define FREQ_PADRAO_ADC 48000000

volatile int16_t adc_buffer[AMOSTRAS_ADC];
uint dma_canal;
dma_channel_config dma_cfg;

void configurar_gpio(uint8_t);
void alterar_status(bool, bool, bool);
void configurar_adc(void);
void amostrar_audio(void);
void configurar_dma(void);
void remover_componente_dc(void);
float converter_adc_volts(int16_t);
void enviar_amostras_microfone_serial(void);
int amostrar_audio_ei(size_t, size_t, float *);

volatile int limite_inferior = 0;
volatile int limite_superior = 200;
signal_t sinal;
ei_impulse_result_t resultado;
int contador = 0;

int main()
{
    stdio_init_all();
    configurar_gpio(LED_AZUL_PINO);
    configurar_gpio(LED_VERMELHO_PINO);
    configurar_gpio(LED_VERDE_PINO);
    configurar_adc();
    configurar_dma();

    sinal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    sinal.get_data = amostrar_audio_ei; // Passa a função que coleta áudio
    printf("Tudo configurado");

    while (true)
    {
        if (!gpio_get(BOTAO_A))
        { // Se o botão for pressionado (nível baixo)
            amostrar_audio();
            printf("Resultado da analise: %d \n", contador);
            contador++;
            int res = run_classifier(&sinal, &resultado, false);
            printf("Resultado: %d\n", res);

            // Mostrar a saída do modelo
            for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
            {
                printf("%s: %.2f\n", resultado.classification[i].label, resultado.classification[i].value);
            }
        }

        sleep_ms(100);
    }
}

// Implementação da captura de áudio
int amostrar_audio_ei(size_t offset, size_t length, float *out_ptr)
{
    for (size_t i = 0; i < length; i++)
    {
        out_ptr[i] = adc_buffer[i]; // Substituir por leitura real do ADC
    }
    return 0;
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

void amostrar_audio()
{
    printf("Aguarde 1 segundo para captura de audio...\n");
    sleep_ms(1000);
    adc_fifo_drain();
    adc_run(false);
    alterar_status(true, false, false);
    printf("Já\n");

    dma_channel_configure(dma_canal, &dma_cfg,
                          adc_buffer,
                          &(adc_hw->fifo),
                          AMOSTRAS_ADC,
                          true);
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_canal);
    adc_run(false);
    alterar_status(false, true, false);
    remover_componente_dc();
}

void remover_componente_dc()
{

    for (int i = 0; i < AMOSTRAS_ADC; i++)
    {
        adc_buffer[i] = (int16_t)(adc_buffer[i] - 2046);
    }
}

void configurar_adc()
{
    uint32_t divisor = FREQ_PADRAO_ADC / (FREQ_ADC);
    adc_init();
    adc_gpio_init(MICROFONE_PINO);
    adc_set_clkdiv(divisor);
    adc_select_input(MICROFONE_CANAL);
    adc_fifo_setup(true, true, 1, false, false);
    printf("ADC configurado.\n");
}
void configurar_gpio(uint8_t porta)
{
    gpio_init(porta);
    gpio_set_dir(porta, GPIO_OUT);
    gpio_put(porta, false);
    gpio_init(BOTAO_A);
    gpio_pull_up(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    printf("GPIO configurado.\n");
}

void alterar_status(bool vermelho, bool verde, bool azul)
{
    gpio_put(LED_VERMELHO_PINO, vermelho);
    gpio_put(LED_VERDE_PINO, verde);
    gpio_put(LED_AZUL_PINO, azul);
}
