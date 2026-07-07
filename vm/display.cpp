#include "display.hpp"
#include <iostream>
#include <cstdlib>

Display::Display(int escala) {
    fator_escala = escala;

    // Inicializa o subsistema de vídeo da SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Calcula o tamanho da janela baseado no fator de escala (Requisito da VM)
    int largura_janela = LARGURA_NATIVA * fator_escala;
    int altura_janela  = ALTURA_NATIVA * fator_escala;

    janela = SDL_CreateWindow(
        "Fantasys32 Virtual Machine",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        largura_janela,
        altura_janela,
        SDL_WINDOW_SHOWN
    );

    if (!janela) {
        std::cerr << "Erro ao criar janela SDL: " << SDL_GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Cria o renderizador acelerado por hardware
    renderizador = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);
    if (!renderizador) {
        std::cerr << "Erro ao criar renderizador: " << SDL_GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Cria a textura no formato estrito ARGB8888 exigido pela especificação (Seção 1.1)
    textura = SDL_CreateTexture(
        renderizador,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        LARGURA_NATIVA,
        ALTURA_NATIVA
    );

    if (!textura) {
        std::cerr << "Erro ao criar textura: " << SDL_GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }
}

Display::~Display() {
    SDL_DestroyTexture(textura);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(janela);
    SDL_Quit();
}

void Display::renderizar(Memory& memoria) {
    // Usamos um vetor temporário para ler palavra por palavra.
    // O método ler_palavra já aplica a correção de Big-Endian nativamente!
    std::vector<uint32_t> pixels(LARGURA_NATIVA * ALTURA_NATIVA);
    for(int i = 0; i < LARGURA_NATIVA * ALTURA_NATIVA; i++) {
        pixels[i] = memoria.ler_palavra(FRAMEBUFFER_INICIO + (i * 4));
    }

    SDL_UpdateTexture(
        textura,
        NULL,
        pixels.data(),
        LARGURA_NATIVA * sizeof(uint32_t)
    );

    SDL_RenderClear(renderizador);
    SDL_RenderCopy(renderizador, textura, NULL, NULL);
    SDL_RenderPresent(renderizador);
}

bool Display::processar_eventos_fechamento() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        // Retorna verdadeiro se o usuário fechar a janela ou apertar ESC
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
            return true;
        }
    }
    return false;
}