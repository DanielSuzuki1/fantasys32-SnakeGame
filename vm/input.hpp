#pragma once
#include <SDL2/SDL.h>
#include <stdint.h>

class Input {
private:
    // Array interno mapeando o estado físico das 16 teclas mapeadas (Seção 1.3.5)
    bool estado_teclas[16];

    // Converte o código interno da VM para o Scancode físico da SDL2
    SDL_Scancode converter_codigo_para_scancode(uint8_t codigo_tecla);

public:
    Input();
    
    // Atualiza a tabela interna lendo o estado atual do teclado da SDL
    void atualizar_estado();

    // Responde diretamente à instrução GKEY da CPU
    uint32_t verificar_tecla(uint8_t codigo_tecla);
};