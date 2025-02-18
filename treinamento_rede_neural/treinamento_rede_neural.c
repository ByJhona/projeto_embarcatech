#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "cJSON.h"
#include "time.h"

#define NOME_WIFI "EmbarcaTech"
#define SENHA_WIFI "jhonatan"

#define LED_ALERTA_PINO 12
#define LED_OK_PINO 11
#define LED_ERRO_PINO 13
#define MICROFONE_PINO 28
#define MICROFONE_CANAL 2
#define BOTAO_A 5

#define AMOSTRAS_ADC 16000 // 8khz * 2 segundos
#define FREQ_ADC 8000      // 48Mhz / 8k amostras
#define FREQ_PADRAO_ADC 48000000
#define VALOR_MAX_12_BITS 4095
#define VOLT_3_3 3.3
#define ADC_CONV_FACTOR (3.3f / 4095.0f)
#define ENDERECO_IP "192.168.231.149"
#define PORTA_TCP 8080

// Configurações do keep-alive
#define KEEP_ALIVE_IDLE_TIME 5  // Tempo de inatividade em segundos
#define KEEP_ALIVE_INTERVAL 2   // Intervalo entre pacotes keep-alive em segundos
#define KEEP_ALIVE_MAX_PROBES 3 // Número máximo de pacotes keep-alive

int16_t adc_buffer[AMOSTRAS_ADC];
uint dma_canal;
dma_channel_config dma_cfg;
static struct tcp_pcb *tcp_cliente;
volatile int8_t numero_pacotes = 16;

void configurar_wifi(void);
void configurar_gpio(uint8_t);
void alterar_status(bool, bool, bool);
void configurar_adc(void);
void amostrar_audio(void);
void configurar_dma(void);
void remover_componente_dc(void);
float converter_adc_volts(int16_t);
void conectar_servidor(void);
err_t enviar_amostras_servidor(void *, struct tcp_pcb *, err_t);
err_t enviar_dados_callback(void *, struct tcp_pcb *, u16_t);
err_t contar_pacotes(void *, struct tcp_pcb *, u16_t);
static err_t tcp_enviado_callback(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t teste1(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t teste2(void *arg, struct tcp_pcb *tpcb, u16_t len);

void criar_json();

volatile int limite_inferior = 0;
volatile int limite_superior = 200;

int main()
{
    stdio_init_all();
    configurar_gpio(LED_ALERTA_PINO);
    configurar_gpio(LED_ERRO_PINO);
    configurar_gpio(LED_OK_PINO);
    configurar_wifi();
    configurar_adc();
    configurar_dma();

    while (true)
    {
        cyw43_arch_poll();
        bool clk_bnt_a = !gpio_get(BOTAO_A);

        if (clk_bnt_a)
        {
            amostrar_audio();
            conectar_servidor();
            remover_componente_dc();
            // criar_json();
        }

        sleep_ms(1000);
    }
}

void conectar_servidor()
{
    alterar_status(false, true, false);

    tcp_cliente = tcp_new();

    ip_addr_t ip_servidor;

    ipaddr_aton(ENDERECO_IP, &ip_servidor);

    tcp_connect(tcp_cliente, &ip_servidor, PORTA_TCP, enviar_amostras_servidor);
}



err_t enviar_amostras_servidor(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    alterar_status(false, false, true);

    if (err == ERR_OK)
    {
        cJSON *json_obj = cJSON_CreateObject();
        uint64_t identificador_tempo = to_ms_since_boot(get_absolute_time());

        cJSON_AddNumberToObject(json_obj, "identificador", identificador_tempo);
        cJSON *amostras = cJSON_CreateArray();
        for (uint16_t i = 0; i < 1000; i++)
        {
            cJSON_AddItemToArray(amostras, cJSON_CreateNumber(adc_buffer[i]));
        }
        cJSON_AddItemToObject(json_obj, "amostras", amostras);

        char *string = cJSON_PrintUnformatted(json_obj); // `PrintUnformatted` evita quebra de linha
        int tamanho = strlen(string);

        char *http_request = (char *)malloc(tamanho + 200); // Aloca memória dinamicamente
        if (!http_request)
        {
            cJSON_Delete(json_obj);
            free(string);
            return ERR_MEM;
        }

        snprintf(http_request, tamanho + 200,
                 "POST /enviar_amostras HTTP/1.1\r\n"
                 "Host: 192.168.231.149:8080\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: keep-alive\r\n"
                 "\r\n"
                 "%s",
                 tamanho, string);

        err_t erro = tcp_write(tpcb, http_request, strlen(http_request), TCP_WRITE_FLAG_COPY);
        if (erro == ERR_OK)
        {
            tcp_output(tpcb);
        }
        else if (erro == ERR_MEM)
        {
            // Repetir tentativa após um curto atraso
            int tentativas = 5;
            while (tentativas-- > 0 && erro == ERR_MEM)
            {
                sleep_ms(100);
                erro = tcp_write(tpcb, http_request, strlen(http_request), TCP_WRITE_FLAG_COPY);
                if (erro == ERR_OK)
                    tcp_output(tpcb);
            }
        }

        // tcp_sent(tpcb, tcp_enviado_callback); // Define o callback quando os dados forem enviados
        free(http_request);
        free(string);
        cJSON_Delete(json_obj);

        return erro;
    }

    alterar_status(false, false, false);
    return ERR_TIMEOUT;
}

err_t enviar_dados_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    if (numero_pacotes >= 0)
    {
        alterar_status(false, false, true);

        uint64_t identificador_tempo = to_ms_since_boot(get_absolute_time());
        uint16_t i;
        uint16_t j;
        uint16_t limite = 0;

        cJSON *json_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_obj, "identificador", identificador_tempo);

        cJSON *amostras = cJSON_CreateArray();

        for (i = 0; i < 1000; i++)
        {
            cJSON_AddItemToArray(amostras, cJSON_CreateNumber(adc_buffer[i]));
        }

        cJSON_AddItemToObject(json_obj, "amostras", amostras);

        char *string = cJSON_Print(json_obj);

        char http_request[16000];

        snprintf(http_request, sizeof(http_request),
                 "POST /enviar_amostras HTTP/1.1\r\n"
                 "Host: 192.168.107.149:8080\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %d\r\n"
                 "\r\n"
                 "%s",
                 strlen(string), string);

        err_t err = tcp_write(tpcb, http_request, strlen(http_request), TCP_WRITE_FLAG_COPY);

        int8_t tentativas = 100;
        while (tentativas >= 0 && err != ERR_MEM)
        {
            alterar_status(true, false, false);
            sleep_ms(100);
            err = tcp_write(tpcb, http_request, strlen(http_request), TCP_WRITE_FLAG_COPY);
            tentativas--;
        }

        free(string);
        cJSON_Delete(json_obj);

        numero_pacotes--;
    }
    else
    {
        alterar_status(false, false, false);
        numero_pacotes = 16;
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
}

void remover_componente_dc()
{
    float soma = 0.f;

    for (int i = 0; i < AMOSTRAS_ADC; i++)
    {
        soma += adc_buffer[i];
    }

    float medida = soma / AMOSTRAS_ADC;

    for (int i = 0; i < AMOSTRAS_ADC; i++)
    {
        adc_buffer[i] = (int16_t)adc_buffer[i] - 2047;
    }
}

float converter_adc_volts(int16_t valor)
{
    return valor * ADC_CONV_FACTOR;
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
    if (cyw43_arch_wifi_connect_timeout_ms(NOME_WIFI, SENHA_WIFI, CYW43_AUTH_WPA2_AES_PSK, 60000))
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
