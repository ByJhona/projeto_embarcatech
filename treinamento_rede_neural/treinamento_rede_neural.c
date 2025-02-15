#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "kiss_fft.h"

#define NOME_WIFI "EmbarcaTech"
#define SENHA_WIFI "jhonatan"

#define LED_ALERTA_PINO 12
#define LED_OK_PINO 11
#define LED_ERRO_PINO 13
#define MICROFONE_PINO 28
#define MICROFONE_CANAL 2
#define BOTAO_A 5

#define AMOSTRAS_ADC 24000 // 8khz * 3 segundos
#define FREQ_ADC 8000      // 48Mhz / 8k amostras
#define FREQ_PADRAO_ADC 48000000

uint16_t adc_buffer[AMOSTRAS_ADC];
uint dma_canal;
dma_channel_config dma_cfg;

void configurar_wifi(void);
void configurar_gpio(uint8_t);
void alterar_status(bool, bool, bool);
void configurar_adc(void);
void amostrar_audio(void);
void configurar_dma(void);

int main()
{
    stdio_init_all();
    configurar_gpio(LED_ALERTA_PINO);
    configurar_gpio(LED_ERRO_PINO);
    configurar_gpio(LED_OK_PINO);
    // configurar_wifi();
    configurar_adc();
    configurar_dma();

    while (true)
    {
        //  cyw43_arch_poll();
        bool clk_bnt_a = !gpio_get(BOTAO_A);

        if (clk_bnt_a)
        {
            amostrar_audio();
        }

        sleep_ms(1000);
    }
}

void configurar_dma()
{
    dma_canal = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_canal);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);

    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}

void amostrar_audio()
{
    adc_fifo_drain();
    adc_run(false);
    dma_channel_configure(dma_canal, &dma_cfg,
                          adc_buffer,
                          &(adc_hw->fifo),
                          AMOSTRAS_ADC,
                          true);
    alterar_status(true, false, false);
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_canal);
    adc_run(false);
    alterar_status(false, true, false);
}
void configurar_adc()
{
    uint32_t divisor = FREQ_PADRAO_ADC / (FREQ_ADC);
    adc_init();
    adc_gpio_init(MICROFONE_PINO);
    adc_set_clkdiv(divisor);
    adc_select_input(MICROFONE_CANAL);
    adc_fifo_setup(true, true, 1, false, false);
}
void configurar_gpio(uint8_t porta)
{
    gpio_init(porta);
    gpio_set_dir(porta, GPIO_OUT);
    gpio_put(porta, false);
    gpio_init(BOTAO_A);
    gpio_pull_up(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
}

void alterar_status(bool vermelho, bool verde, bool azul)
{
    gpio_put(LED_ERRO_PINO, vermelho);
    gpio_put(LED_OK_PINO, verde);
    gpio_put(LED_ALERTA_PINO, azul);
}

void configurar_wifi()
{
    if (cyw43_arch_init() != 0)
    {
        printf("Falha ao iniciar wifi.\n");
        alterar_status(true, false, false);
    }

    cyw43_arch_enable_sta_mode();
    alterar_status(false, true, true);

    printf("Conectando ao wifi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(NOME_WIFI, SENHA_WIFI, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Conexão do wifi falhou.\n");
        alterar_status(true, false, false);
    }
    else
    {
        printf("Conectado..\n");
        alterar_status(false, true, false);
        uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP: %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }
}
