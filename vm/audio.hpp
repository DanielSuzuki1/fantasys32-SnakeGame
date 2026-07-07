#pragma once
#include <SDL2/SDL.h>
#include <stdint.h>

class Audio {
private:
    SDL_AudioSpec especificacao_desejada;
    SDL_AudioDeviceID dispositivo_id;
    
    // Variáveis de controle da onda senoidal/quadrada/triangular
    double frequencia_atual;
    double amostra_atual;
    uint8_t forma_onda_atual;
    uint32_t tempo_restante_ms;

    // Função de callback estática que a SDL2 chama para preencher o buffer de som
    static void callback_audio(void* dados_usuario, uint8_t* stream, int comprimento);

public:
    Audio();
    ~Audio();

    // Executa a instrução PLAY de forma não-bloqueante
    void emitir_som(uint32_t freq, uint32_t ms, uint8_t forma_onda);
    
    // Atualiza o temporizador do som a cada frame (chamado no loop principal)
    void atualizar_tempo(uint32_t ms_passados);
};