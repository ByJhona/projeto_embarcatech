#ifndef JOGO_H
#define JOGO_H
#include "pico/stdlib.h"
#include "Jogador.h"   


class Jogo
{
public:
    uint8_t rodadas;
    Jogador jogador;

    Jogo(uint8_t rodadas, Professor professor)
        : rodadas(rodadas), rodadas_restantes(rodadas), resposta_certa(0x0), professor(professor)
    {
        this->iniciar_jogo = false;
    }

    void definir_resposta_certa();
};

#endif // JOGO_H
