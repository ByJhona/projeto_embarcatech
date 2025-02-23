#ifndef COLETOR_AUDIO_H  
#define COLETOR_AUDIO_H

#define PINO_LED_AZUL 12
#define PINO_LED_VERDE 11
#define PINO_LED_VERMELHO 13
#define PINO_MICROFONE 28
#define CANAL_MICROFONE 2
#define PINO_BOTAO_A 5
#define NUMERO_AMOSTRAS_ADC 16000
#define FREQUENCIA_DESEJADA_ADC 16000
#define FREQUENCIA_PADRAO_ADC 48000000
#define OFFSET_DC_ADC 2048

void gravar_audio(void);
void alterar_status(bool, bool, bool);
void configurar_gpio(uint8_t);
void configurar_adc(uint8_t, uint8_t);
void configurar_dma(void);
void enviar_amostras_microfone_serial(void);
void remover_componente_dc(void);

#endif 
