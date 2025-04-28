# Reconhecimento de Vogais e Ensino de Braille com RP2040

## 📚 Descrição

Este projeto utiliza a placa **BitDogLab**, equipada com um microfone embutido, para capturar áudio em tempo real. As amostras de áudio foram utilizadas para treinar uma rede neural no **Edge Impulse**.

Após o treinamento, a rede foi exportada como uma **biblioteca otimizada** para ser executada **offline** diretamente na RP2040. A placa, usando essa biblioteca, reconhece **vogais (A, E, I, O, U)** e exibe a letra correspondente em **Braille** utilizando um **display de LEDs**.

## 💠 Tecnologias Utilizadas

- **Placa:** RP2040 BitDogLab
- **Microfone:** Embutido na placa
- **Framework de IA:** [Edge Impulse](https://edgeimpulse.com/)
- **Linguagem:** C++
- **Display:** Matriz de LEDs

## 🌟 Objetivos

- Capturar áudio usando o microfone da RP2040.
- Processar e enviar as amostras para o Edge Impulse.
- Treinar uma rede neural para classificação de vogais.
- Exportar o modelo como biblioteca para uso offline.
- Exibir a letra reconhecida em formato Braille num display de LEDs.

## 🧐 Como Funciona

1. **Captura de Áudio:** O microfone embutido grava as entradas de voz.
2. **Treinamento Online:** As amostras de áudio são enviadas ao Edge Impulse, onde a rede é treinada.
3. **Exportação do Modelo:** Após o treinamento, a IA é exportada como uma biblioteca C++ para a placa.
4. **Execução Offline:** O RP2040 roda o modelo localmente, sem necessidade de conexão com a internet.
5. **Exibição em Braille:** A vogal reconhecida é exibida em Braille no display de LEDs.

## 🔋 Requisitos

- Placa RP2040 BitDogLab.
- Conta no [Edge Impulse](https://edgeimpulse.com/).
- Ambiente de desenvolvimento configurado (Arduino IDE, Thonny, etc.).
- Display de LEDs compatível.

## 🚀 Instruções

1. Clone este repositório.
2. Conecte sua RP2040 ao computador.
3. Capture amostras de voz através do firmware de coleta.
4. Treine o modelo no Edge Impulse.
5. Exporte o modelo para biblioteca C++.
6. Integre o modelo ao firmware da placa.
7. Implemente a exibição das vogais reconhecidas em Braille.

## 💬 Ideias Futuras

- Expandir para reconhecimento de palavras completas.
- Integrar feedback sonoro junto com o Braille.
- Transformar em um kit didático para escolas inclusivas.
- Criar display com motores solenoides para mais acessibilidade.

---

# 🎉 Feito com paixão por Jhonatan Borges de Souza 💖

