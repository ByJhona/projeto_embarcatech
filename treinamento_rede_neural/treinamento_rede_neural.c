#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define NOME_WIFI "EmbarcaTeche"
#define SENHA_WIFI "jhonatan"

#define LED_ALERTA 12
#define LED_OK 11
#define LED_ERRO 13

volatile uint8_t porta_callback;

void configurar_wifi(void);
void configurar_status(uint8_t);
void alterar_status(uint8_t);
int64_t desativar_status_callback(alarm_id_t id, void *porta);

int main()
{
    stdio_init_all();
    configurar_status(LED_ALERTA);
    configurar_status(LED_ERRO);
    configurar_status(LED_OK);
    configurar_wifi();

    while (true)
    {
        cyw43_arch_poll();
        printf("Hello, world!\n");
        sleep_ms(100);
    }
}

void configurar_status(uint8_t porta)
{
    gpio_init(porta);
    gpio_set_dir(porta, GPIO_OUT);
    gpio_put(porta, false);
}

void alterar_status(uint8_t porta)
{
    gpio_put(porta, true);

    uint8_t *porta_ptr = malloc(sizeof(uint8_t));
    if (porta_ptr != NULL)
    {
        *porta_ptr = porta;
        add_alarm_in_ms(1000, desativar_status_callback, porta_ptr, true);
    }
}

int64_t desativar_status_callback(alarm_id_t id, void *porta)
{
    uint8_t portaInt = *((uint8_t *)porta);
    gpio_put(portaInt, false);
    free(porta);
    return 0;
}

void configurar_wifi()
{
    if (cyw43_arch_init() != 0)
    {
        printf("Falha ao iniciar wifi.\n");
        alterar_status(LED_ERRO);
    }

    cyw43_arch_enable_sta_mode();
    alterar_status(LED_ALERTA);

    printf("Conectando ao wifi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(NOME_WIFI, SENHA_WIFI, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Conexão do wifi falhou.\n");
        alterar_status(LED_ERRO);
    }
    else
    {
        printf("Conectado..\n");
        alterar_status(LED_OK);
        uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP: %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }
}
