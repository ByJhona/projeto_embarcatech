#include "main.h"


int main()
{
    stdio_init_all();
    configurar_gpio();
    configurar_adc();
    configurar_dma();
    configurar_classificador();
    configurar_display_pixel();
    desenhar_display();

    while (true)
    {

        estado_botao_A = !gpio_get(PINO_BOTAO_A);
        estado_botao_B = !gpio_get(PINO_BOTAO_B);

        if (estado_botao_A && estado_botao_A != estado_anterior_botao_A)
        {

            gerar_letra_aleatoria();
            limpar_display_pixel();
            desenhar_display();
            sleep_ms(500);
            colorir_resposta_gabarito();
            desenhar_display();
            
            printf("Gerada - > Id: %d | Letra: %s\n", jogo.resposta_gabarito.id, jogo.resposta_gabarito.descricao);
        }
        else if (estado_botao_B && estado_botao_B != estado_anterior_botao_B)
        {
            gravar_audio();
            classificar_letra();
            printf("Classificada -> Id: %d | Letra: %s\n", jogo.resposta_jogador.id, jogo.resposta_jogador.descricao);
            corrigir_jogador();

            limpar_display_pixel();
            desenhar_display();
            sleep_ms(1000);

            colorir_resposta_jogador();
            desenhar_display();
            sleep_ms(5000);
            limpar_display_pixel();
            desenhar_display();

        }

        estado_anterior_botao_A = estado_botao_A;
        estado_anterior_botao_B = estado_botao_B;

        sleep_ms(100);
    }
}

void configurar_display_pixel()
{
    uint offset = pio_add_program(pio0, &ws2818b_program);
    display_pio = pio0;

    sm = pio_claim_unused_sm(display_pio, false);
    if (sm < 0)
    {
        display_pio = pio1;
        sm = pio_claim_unused_sm(display_pio, true);
    }

    iniciar_pio(display_pio, sm, offset, PINO_PIO, 800000.f);

    for (uint i = 0; i < NUM_PIXEL; ++i)
    {
        display[i].R = 0;
        display[i].G = 0;
        display[i].B = 0;
    }
}

void colorir_resposta_gabarito()
{
    for (uint8_t index = 0, i = 23; index <= 2 && i >= 3; index++, i -= 10)
    {
        if (jogo.resposta_gabarito.escrita[index] == 1)
        {
            colorir_pixel(i, 0, 0, 1);
        }
    }

    for (uint8_t index = 3, i = 21; index <= 5 && i >= 1; index++, i -= 10)
    {
        if (jogo.resposta_gabarito.escrita[index] == 1)
        {
            colorir_pixel(i, 0, 0, 1);
        }
    }
}


void colorir_resposta_jogador()
{
    for (uint8_t index = 0, i = 23; index <= 2 && i >= 3; index++, i -= 10)
    {
        if (jogo.resposta_jogador.escrita[index] == 1)
        {
            colorir_pixel(i, 1, 0, 1);
        }
    }

    for (uint8_t index = 3, i = 21; index <= 5 && i >= 1; index++, i -= 10)
    {
        if (jogo.resposta_jogador.escrita[index] == 1)
        {
            colorir_pixel(i, 1, 0, 1);
        }
    }
}


void colorir_pixel(uint8_t indice, uint8_t r, uint8_t g, uint8_t b)
{
    display[indice].R = r;
    display[indice].G = g;
    display[indice].B = b;
}

void limpar_display_pixel()
{
    for (uint i = 0; i < NUM_PIXEL; ++i)
        colorir_pixel(i, 0, 0, 0);
}
void desenhar_display()
{
    for (uint i = 0; i < NUM_PIXEL; ++i)
    {
        pio_sm_put_blocking(display_pio, sm, display[i].G);
        pio_sm_put_blocking(display_pio, sm, display[i].R);
        pio_sm_put_blocking(display_pio, sm, display[i].B);
    }
    sleep_us(100);
}

void gerar_letra_aleatoria()
{
    srand(time(NULL));  // Define a semente uma única vez
    uint8_t id_aleatorio = rand() % NUM_LETRAS;
    jogo.resposta_gabarito.id = id_aleatorio;
    strcpy(jogo.resposta_gabarito.descricao, vogais[id_aleatorio].descricao);
    for (int i = 0; i <= 5; i++)
    {
        jogo.resposta_gabarito.escrita[i] = vogais[id_aleatorio].escrita[i];
    }
}

void corrigir_jogador()
{
    uint8_t result = strcmp(jogo.resposta_jogador.descricao, jogo.resposta_gabarito.descricao);
    if (result == 0)
    {
        jogo.pontos += 1;
        printf("Jogador acertou a letra e ganhou 1 ponto.\n");
    }else{
        printf("Jogador errou!!.\n");
    }
}

void classificar_letra()
{
    Classificador letra_classificada = {{0, ""}, -1};
    int res = run_classifier(&sinal, &resultado, false);

    if (res != 0)
    {
        printf("Erro ao executar o classificador.\n");
        return;
    }

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
    definir_resposta_jogador(letra_classificada.letra.id, letra_classificada.letra.descricao);
}

void definir_resposta_jogador(uint8_t id, const char *descricao)
{
    jogo.resposta_jogador.id = id;
    strcpy(jogo.resposta_jogador.descricao, descricao);
    for (int i = 0; i <= 5; i++)
    {
        jogo.resposta_jogador.escrita[i] = vogais[id].escrita[i];
    }
    printf("Resposta da Inferência: %s\n", descricao);
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
        classificados[i] = adc_buffer[i];
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

void configurar_adc()
{
    uint32_t divisor = FREQUENCIA_PADRAO_ADC / (FREQUENCIA_DESEJADA_ADC);
    adc_init();
    adc_gpio_init(PINO_MICROFONE);
    adc_set_clkdiv(divisor);
    adc_select_input(CANAL_MICROFONE);
    adc_fifo_setup(true, true, 1, false, false);
    printf("ADC configurado.\n");
}

void configurar_gpio()
{
    gpio_init(PINO_LED_VERMELHO);
    gpio_set_dir(PINO_LED_VERMELHO, GPIO_OUT);
    gpio_put(PINO_LED_VERMELHO, false);

    gpio_init(PINO_LED_VERDE);
    gpio_set_dir(PINO_LED_VERDE, GPIO_OUT);
    gpio_put(PINO_LED_VERDE, false);

    gpio_init(PINO_LED_AZUL);
    gpio_set_dir(PINO_LED_AZUL, GPIO_OUT);
    gpio_put(PINO_LED_AZUL, false);

    gpio_init(PINO_BOTAO_A);
    gpio_pull_up(PINO_BOTAO_A);
    gpio_set_dir(PINO_BOTAO_A, GPIO_IN);

    gpio_init(PINO_BOTAO_B);
    gpio_pull_up(PINO_BOTAO_B);
    gpio_set_dir(PINO_BOTAO_B, GPIO_IN);
    printf("GPIO's configurados.\n");
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