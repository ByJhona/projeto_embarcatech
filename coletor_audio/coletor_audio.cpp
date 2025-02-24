#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "coletor_audio.h"

uint dma_canal;
dma_channel_config dma_cfg;

volatile int16_t adc_buffer[NUMERO_AMOSTRAS_ADC];

bool estado_botao_A = false;
bool estado_anterior_botao_A = false;

int main()
{
    stdio_init_all();
    configurar_gpio(PINO_LED_AZUL);
    configurar_gpio(PINO_LED_VERMELHO);
    configurar_gpio(PINO_LED_VERDE);
    configurar_adc(PINO_MICROFONE, CANAL_MICROFONE);
    configurar_dma();

    while (true)
    {
        gravar_audio();
        enviar_amostras_microfone_serial();
        sleep_ms(100);
    }
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
    dma_channel_configure(dma_canal, &dma_cfg,
                          adc_buffer,
                          &(adc_hw->fifo),
                          NUMERO_AMOSTRAS_ADC,
                          true);
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_canal);
    adc_run(false);
    alterar_status(false, true, false);
    remover_componente_dc();
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
