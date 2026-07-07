#include "audio.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>

Audio::Audio() {
    frequencia_atual = 0.0;
    amostra_atual = 0.0;
    forma_onda_atual = 0;
    tempo_restante_ms = 0;

    // Configura o formato de áudio padrão da SDL2
    especificacao_desejada.freq = 44100;         // Taxa de amostragem de 44.1kHz
    especificacao_desejada.format = AUDIO_S16SYS; // Inteiro de 16 bits com sinal nativo
    especificacao_desejada.channels = 1;         // Som Mono
    especificacao_desejada.samples = 2048;       // Tamanho do buffer
    especificacao_desejada.callback = callback_audio;
    especificacao_desejada.userdata = this;      // Passa a instância da classe para o callback

    // Inicializa o subsistema de áudio da SDL
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Erro ao inicializar audio SDL: " << SDL_GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }

    // Abre o dispositivo de áudio padrão do computador
    dispositivo_id = SDL_OpenAudioDevice(NULL, 0, &especificacao_desejada, NULL, 0);
    if (dispositivo_id == 0) {
        std::cerr << "Erro ao abrir dispositivo de audio: " << SDL_GetError() << "\n";
        std::exit(EXIT_FAILURE);
    }
}

Audio::~Audio() {
    SDL_CloseAudioDevice(dispositivo_id);
}

void Audio::emitir_som(uint32_t freq, uint32_t ms, uint8_t forma_onda) {
    // Se a frequência for 0 ou tempo 0, pausa o som imediatamente
    if (freq == 0 || ms == 0) {
        SDL_PauseAudioDevice(dispositivo_id, 1);
        tempo_restante_ms = 0;
        return;
    }

    // Atualiza os parâmetros de síntese física da onda
    frequencia_atual = static_cast<double>(freq);
    tempo_restante_ms = ms;
    forma_onda_atual = forma_onda;

    // Liga o dispositivo de áudio (reprodução não-bloqueante ativa)
    SDL_PauseAudioDevice(dispositivo_id, 0);
}

void Audio::atualizar_tempo(uint32_t ms_passados) {
    if (tempo_restante_ms > 0) {
        if (ms_passados >= tempo_restante_ms) {
            tempo_restante_ms = 0;
            SDL_PauseAudioDevice(dispositivo_id, 1); // Desliga o som ao acabar o tempo
        } else {
            tempo_restante_ms -= ms_passados;
        }
    }
}

void Audio::callback_audio(void* dados_usuario, uint8_t* stream, int comprimento) {
    Audio* audio = reinterpret_cast<Audio*>(dados_usuario);
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int tamanho_buffer = comprimento / 2; // Convertendo bytes para amostras de 16-bits

    const double taxa_amostragem = 44100.0;
    const int16_t amplitude_maxima = 8000; // Define um volume confortável

    for (int i = 0; i < tamanho_buffer; i++) {
        double t = audio->amostra_atual / taxa_amostragem;
        double valor_onda = 0.0;

        // Gera as formas de onda de acordo com a Seção 1.3.5
        switch (audio->forma_onda_atual) {
            case 0x00: // SINE (Onda Senoidal pura)
                valor_onda = std::sin(2.0 * M_PI * audio->frequencia_atual * t);
                break;

            case 0x01: // SQUARE (Onda Quadrada)
                valor_onda = (std::sin(2.0 * M_PI * audio->frequencia_atual * t) >= 0.0) ? 1.0 : -1.0;
                break;

            case 0x02: // TRIANGLE (Onda Triangular)
                valor_onda = 2.0 * std::abs(2.0 * (t * audio->frequencia_atual - std::floor(t * audio->frequencia_atual + 0.5))) - 1.0;
                break;

            case 0x03: // NOISE (Ruído branco aleatório)
                valor_onda = (static_cast<double>(std::rand()) / RAND_MAX) * 2.0 - 1.0;
                break;
        }

        buffer[i] = static_cast<int16_t>(valor_onda * amplitude_maxima);
        audio->amostra_atual += 1.0;
    }
}