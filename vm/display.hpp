#pragma once
#include "memory.hpp"
#include <SDL2/SDL.h>
#include <string>

// Dimensões nativas do Fantasy Computer
const int LARGURA_NATIVA = 320;
const int ALTURA_NATIVA  = 240;

class Display {
private:
    SDL_Window* janela;
    SDL_Renderer* renderizador;
    SDL_Texture* textura;
    int fator_escala; // Define o multiplicador de tamanho (--scale)

public:
    Display(int escala);
    ~Display();

    // Atualiza a textura SDL copiando os dados brutos da memória de vídeo da VM
    void renderizar(Memory& memoria);
    
    // Processa o encerramento da janela (clicar no X)
    bool processar_eventos_fechamento();
};