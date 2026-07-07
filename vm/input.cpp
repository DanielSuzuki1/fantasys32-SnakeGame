#include "input.hpp"

Input::Input() {
    for (int i = 0; i < 16; i++) {
        estado_teclas[i] = false;
    }
}

SDL_Scancode Input::converter_codigo_para_scancode(uint8_t codigo_tecla) {
    // Tabela de mapeamento estrito da especificação (Seção 1.3.5)
    switch (codigo_tecla) {
        case 0x00: return SDL_SCANCODE_LEFT;      // SETA ESQUERDA
        case 0x01: return SDL_SCANCODE_RIGHT;     // SETA DIREITA
        case 0x02: return SDL_SCANCODE_UP;        // SETA CIMA
        case 0x03: return SDL_SCANCODE_DOWN;      // SETA BAIXO
        case 0x04: return SDL_SCANCODE_SPACE;     // ESPAÇO
        case 0x05: return SDL_SCANCODE_RETURN;    // ENTER
        case 0x06: return SDL_SCANCODE_N;         // N
        case 0x07: return SDL_SCANCODE_M;         // M
        case 0x08: return SDL_SCANCODE_A;         // A
        case 0x09: return SDL_SCANCODE_S;         // S
        case 0x0A: return SDL_SCANCODE_D;         // D
        case 0x0B: return SDL_SCANCODE_W;         // W
        case 0x0C: return SDL_SCANCODE_Q;         // Q
        case 0x0D: return SDL_SCANCODE_E;         // E
        case 0x0E: return SDL_SCANCODE_C;         // C
        case 0x0F: return SDL_SCANCODE_V;         // V
        default:   return SDL_SCANCODE_UNKNOWN;
    }
}

void Input::atualizar_estado() {
    // Captura o estado instantâneo de todas as teclas do sistema operacional
    const Uint8* teclado_sdl = SDL_GetKeyboardState(NULL);

    // Varre e atualiza o estado das nossas 16 teclas virtuais de forma não-bloqueante
    for (uint8_t i = 0; i < 16; i++) {
        SDL_Scancode sc = converter_codigo_para_scancode(i);
        if (sc != SDL_SCANCODE_UNKNOWN) {
            estado_teclas[i] = teclado_sdl[sc];
        }
    }
}

uint32_t Input::verificar_tecla(uint8_t codigo_tecla) {
    if (codigo_tecla >= 16) return 0;
    
    // Retorna 1 se pressionada, 0 caso contrário (Requisito GKEY)
    return estado_teclas[codigo_tecla] ? 1 : 0;
}