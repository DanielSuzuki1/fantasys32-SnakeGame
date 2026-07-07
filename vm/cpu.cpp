#include "cpu.hpp"
#include "input.hpp"
#include "audio.hpp"
#include <iostream>
#include <cstdlib>

/**
 * @brief Construtor da Unidade Central de Processamento (CPU).
 */
CPU::CPU(uint32_t pc_inicial, bool no_syscall) {
    for (int i = 0; i < 16; i++) regs[i] = 0;
    regs[SP] = PILHA_FIM;      
    regs[PC] = pc_inicial;    
    seed_lcg = 0;              
    frames = 0;                
    halt = false;              
    opt_no_syscall = no_syscall; 
}

void CPU::push(Memory& memoria, uint32_t valor) {
    if (regs[SP] - 4 < (int32_t)PILHA_INICIO) {
        std::cerr << "Erro de Execucao: Stack Overflow detectado no PC 0x" << std::hex << regs[PC] << "\n";
        std::exit(EXIT_FAILURE);
    }
    regs[SP] -= 4; 
    memoria.escrever_palavra(regs[SP], valor); 
}

uint32_t CPU::pop(Memory& memoria) {
    if (regs[SP] + 4 > (int32_t)PILHA_FIM) {
        std::cerr << "Erro de Execucao: Stack Underflow detectado no PC 0x" << std::hex << regs[PC] << "\n";
        std::exit(EXIT_FAILURE);
    }
    uint32_t valor = memoria.ler_palavra(regs[SP]); 
    regs[SP] += 4; 
    return valor;
}

void CPU::executar_instrucao_completa(Memory& memoria, Input& input, Audio& audio) {
    regs[0] = 0; 

    // === FASE 1: FETCH ===
    uint32_t instr = memoria.ler_palavra(regs[PC]);
    regs[PC] += 4;

    // === FASE 2: DECODE ===
    uint8_t opcode = (instr >> 26) & 0x3F; 

    // === FASE 3: EXECUTE ===
    switch (opcode) {
        
        // --- 1.3.1 ARITMÉTICA E LÓGICA ---
        case 0x00: { // ADD rd, rs, rt
            uint8_t reg_rd = (instr >> 22) & 0x0F; 
            uint8_t reg_rs = (instr >> 18) & 0x0F; 
            uint8_t reg_rt = (instr >> 14) & 0x0F; 
            regs[reg_rd] = regs[reg_rs] + regs[reg_rt];
            break;
        }
            
        case 0x01: { // SUB rd, rs, rt
            uint8_t reg_rd = (instr >> 22) & 0x0F; 
            uint8_t reg_rs = (instr >> 18) & 0x0F; 
            uint8_t reg_rt = (instr >> 14) & 0x0F; 
            regs[reg_rd] = regs[reg_rs] - regs[reg_rt];
            break;
        }
            
        case 0x02: { // MUL rd, rs, rt
            uint8_t reg_rd = (instr >> 22) & 0x0F; 
            uint8_t reg_rs = (instr >> 18) & 0x0F; 
            uint8_t reg_rt = (instr >> 14) & 0x0F; 
            regs[reg_rd] = regs[reg_rs] * regs[reg_rt];
            break;
        }
            
        case 0x03: { // DIV rd, rs, rt
            uint8_t reg_rd = (instr >> 22) & 0x0F; 
            uint8_t reg_rs = (instr >> 18) & 0x0F; 
            uint8_t reg_rt = (instr >> 14) & 0x0F; 
            if (regs[reg_rt] == 0) { exit(EXIT_FAILURE); }
            regs[reg_rd] = regs[reg_rs] / regs[reg_rt];
            break;
        }
            
        case 0x04: { // MOD rd, rs, rt
            uint8_t reg_rd = (instr >> 22) & 0x0F; 
            uint8_t reg_rs = (instr >> 18) & 0x0F; 
            uint8_t reg_rt = (instr >> 14) & 0x0F; 
            if (regs[reg_rt] == 0) { exit(EXIT_FAILURE); }
            regs[reg_rd] = regs[reg_rs] % regs[reg_rt];
            break;
        }

        // Lógica Booleana e Shifts
        case 0x05: { // AND
            uint8_t rd = (instr >> 22) & 0x0F, rs = (instr >> 18) & 0x0F, rt = (instr >> 14) & 0x0F;
            regs[rd] = regs[rs] & regs[rt]; break;
        }
        case 0x06: { // OR
            uint8_t rd = (instr >> 22) & 0x0F, rs = (instr >> 18) & 0x0F, rt = (instr >> 14) & 0x0F;
            regs[rd] = regs[rs] | regs[rt]; break;
        }
        case 0x07: { // XOR
            uint8_t rd = (instr >> 22) & 0x0F, rs = (instr >> 18) & 0x0F, rt = (instr >> 14) & 0x0F;
            regs[rd] = regs[rs] ^ regs[rt]; break;
        }
        case 0x08: { // SHL
            uint8_t rd = (instr >> 22) & 0x0F, rs = (instr >> 18) & 0x0F, rt = (instr >> 14) & 0x0F;
            regs[rd] = regs[rs] << (regs[rt] & 0x1F); break;
        }
        case 0x09: { // SHR
            uint8_t rd = (instr >> 22) & 0x0F, rs = (instr >> 18) & 0x0F, rt = (instr >> 14) & 0x0F;
            regs[rd] = regs[rs] >> (regs[rt] & 0x1F); break;
        }
        case 0x0A: { // ROL
            uint8_t rd = (instr >> 22) & 0x0F, rs = (instr >> 18) & 0x0F, rt = (instr >> 14) & 0x0F;
            regs[rd] = (regs[rs] << (regs[rt] & 0x1F)) | (regs[rs] >> (32 - (regs[rt] & 0x1F))); break;
        }
        case 0x0B: { // ROR
            uint8_t rd = (instr >> 22) & 0x0F, rs = (instr >> 18) & 0x0F, rt = (instr >> 14) & 0x0F;
            regs[rd] = (regs[rs] >> (regs[rt] & 0x1F)) | (regs[rs] << (32 - (regs[rt] & 0x1F))); break;
        }

        case 0x0C: { // ADDI rt, rs, imm18
            uint8_t reg_rt = (instr >> 22) & 0x0F; // 1º Arg: Destino
            uint8_t reg_rs = (instr >> 18) & 0x0F; // 2º Arg: Origem
            uint32_t imm18 = instr & 0x3FFFF;
            int32_t imm18_sinal = (imm18 & 0x20000) ? (int32_t)(imm18 | 0xFFFC0000) : (int32_t)imm18;
            regs[reg_rt] = regs[reg_rs] + imm18_sinal;
            break;
        }

        // --- 1.3.2 MOVIMENTAÇÃO E MEMÓRIA ---
        case 0x0D: { // MOVL rt, imm16
            // Extração padrão: rt está nos bits [21..18] e o valor nos bits [15..0]
            uint8_t reg_rt = (instr >> 18) & 0x0F; 
            uint16_t valor_imm = instr & 0xFFFF;

            regs[reg_rt] = valor_imm;
            break;
        }
            
        case 0x0E: { // MOVH rt, imm16
            uint8_t reg_rt = (instr >> 18) & 0x0F;
            uint32_t valor_imm = instr & 0xFFFF;

            regs[reg_rt] = regs[reg_rt] | (valor_imm << 16);
            break;
        }
            
        case 0x0F: { // LOAD rt, rs, imm18
            uint8_t reg_rt = (instr >> 22) & 0x0F; // 1º Arg: Destino (Valor lido)
            uint8_t reg_rs = (instr >> 18) & 0x0F; // 2º Arg: Base (Ponteiro)
            uint32_t imm18 = instr & 0x3FFFF;
            int32_t imm18_sinal = (imm18 & 0x20000) ? (int32_t)(imm18 | 0xFFFC0000) : (int32_t)imm18;
            regs[reg_rt] = memoria.ler_palavra(regs[reg_rs] + (imm18_sinal * 4));
            break;
        }
        
        case 0x10: { // STORE rt, rs, imm18
            uint8_t reg_rt = (instr >> 22) & 0x0F; // 1º Arg: Valor a ser salvo
            uint8_t reg_rs = (instr >> 18) & 0x0F; // 2º Arg: Base (Ponteiro)
            uint32_t imm18 = instr & 0x3FFFF;
            int32_t imm18_sinal = (imm18 & 0x20000) ? (int32_t)(imm18 | 0xFFFC0000) : (int32_t)imm18;
            memoria.escrever_palavra(regs[reg_rs] + (imm18_sinal * 4), regs[reg_rt]);
            break;
        }

        // --- 1.3.3 CONTROLE DE FLUXO ---
        case 0x11: { // BEQ rs, rt, offset16
            uint8_t b_rs = (instr >> 22) & 0x0F;
            uint8_t b_rt = (instr >> 18) & 0x0F;
            int16_t offset16 = static_cast<int16_t>(instr & 0xFFFF);

            if (regs[b_rs] == regs[b_rt]) {
                regs[PC] = regs[PC] + (offset16 * 4);
            }
            break;
        }
            
        case 0x12: { // BNE rs, rt, offset16
            uint8_t b_rs = (instr >> 22) & 0x0F;
            uint8_t b_rt = (instr >> 18) & 0x0F;
            int16_t offset16 = static_cast<int16_t>(instr & 0xFFFF);

            if (regs[b_rs] != regs[b_rt]) {
                regs[PC] = regs[PC] + (offset16 * 4);
            }
            break;
        }
            
        case 0x13: { // BLT rs, rt
            uint8_t b_rs = (instr >> 22) & 0x0F;
            uint8_t b_rt = (instr >> 18) & 0x0F;
            int16_t offset16 = static_cast<int16_t>(instr & 0xFFFF);
            if (static_cast<int32_t>(regs[b_rs]) < static_cast<int32_t>(regs[b_rt])) {
                regs[PC] = regs[PC] + (offset16 * 4);
            }
            break;
        }
            
        case 0x14: { // BGT rs, rt
            uint8_t b_rs = (instr >> 22) & 0x0F;
            uint8_t b_rt = (instr >> 18) & 0x0F;
            int16_t offset16 = static_cast<int16_t>(instr & 0xFFFF);
            if (static_cast<int32_t>(regs[b_rs]) > static_cast<int32_t>(regs[b_rt])) {
                regs[PC] = regs[PC] + (offset16 * 4);
            }
            break;
        }
            
        case 0x15: { // BLE rs, rt
            uint8_t b_rs = (instr >> 22) & 0x0F;
            uint8_t b_rt = (instr >> 18) & 0x0F;
            int16_t offset16 = static_cast<int16_t>(instr & 0xFFFF);
            if (static_cast<int32_t>(regs[b_rs]) <= static_cast<int32_t>(regs[b_rt])) {
                regs[PC] = regs[PC] + (offset16 * 4);
            }
            break;
        }
            
        case 0x16: { // BGE rs, rt
            uint8_t b_rs = (instr >> 22) & 0x0F;
            uint8_t b_rt = (instr >> 18) & 0x0F;
            int16_t offset16 = static_cast<int16_t>(instr & 0xFFFF);
            if (static_cast<int32_t>(regs[b_rs]) >= static_cast<int32_t>(regs[b_rt])) {
                regs[PC] = regs[PC] + (offset16 * 4);
            }
            break;
        }
            
        case 0x17: { // JMP addr26
            // Extração padrão do endereço absoluto de 26 bits
            uint32_t address = instr & 0x3FFFFFF;
            regs[PC] = address * 4;
            break;
        }
            
        case 0x18: { // CALL addr26
            push(memoria, regs[PC]); 
            regs[PC] = (instr & 0x3FFFFFF) * 4;   
            break;
        }

        // --- 1.3.4 UNÁRIAS E PILHA ---
        case 0x19: // POP ra
            regs[ (instr >> 22) & 0x0F ] = pop(memoria); 
            break; 
            
        case 0x1A: // PUSH ra
            push(memoria, regs[ (instr >> 22) & 0x0F ]); 
            break; 
            
        case 0x1B: // INC ra
            regs[ (instr >> 22) & 0x0F ]++; 
            break; 
            
        case 0x1C: // DEC ra
            regs[ (instr >> 22) & 0x0F ]--; 
            break; 
            
        case 0x1D: // NOT ra
            regs[ (instr >> 22) & 0x0F ] = ~regs[ (instr >> 22) & 0x0F ]; 
            break; 
            
        case 0x1E: // RET
            regs[PC] = pop(memoria); 
            break;


        // --- 1.3.5 REQUISITOS DE SISTEMA ---
        case 0x21: { // CLEAR ra
            uint8_t reg_ra = (instr >> 22) & 0x0F;
            uint32_t cor = regs[reg_ra];
            for (uint32_t addr = FRAMEBUFFER_INICIO; addr <= FRAMEBUFFER_FIM; addr += 4) {
                memoria.escrever_palavra(addr, cor);
            }
            break;
        }

        case 0x1F: { // RECT ra, rb, rc, rd, re
            uint8_t reg_ra  = (instr >> 22) & 0x0F; 
            uint8_t reg_rb  = (instr >> 18) & 0x0F; 
            uint8_t reg_rc  = (instr >> 14) & 0x0F; 
            uint8_t reg_rd  = (instr >> 10) & 0x0F; 
            uint8_t reg_re  = (instr >> 6)  & 0x0F; 

            int32_t x = static_cast<int32_t>(regs[reg_ra]);  
            int32_t y = static_cast<int32_t>(regs[reg_rb]);  
            int32_t w = static_cast<int32_t>(regs[reg_rc]);  
            int32_t h = static_cast<int32_t>(regs[reg_rd]); 
            uint32_t cor = regs[reg_re];

            for (int32_t j = y; j < y + h; j++) {
                for (int32_t i = x; i < x + w; i++) {
                    if (i >= 0 && i < 320 && j >= 0 && j < 240) {
                        memoria.escrever_palavra(FRAMEBUFFER_INICIO + ((j * 320 + i) * 4), cor);
                    }
                }
            }
            break;
        }

        case 0x20: { // DSPRITE ra, rb, rc, rd, re
            uint8_t ra = (instr >> 22) & 0x0F, rb = (instr >> 18) & 0x0F;
            uint8_t rc = (instr >> 14) & 0x0F, rd = (instr >> 10) & 0x0F, re = (instr >> 6) & 0x0F;
            int32_t x = regs[ra], y = regs[rb], w = regs[rc], h = regs[rd];
            uint32_t addr = regs[re];
            for (int32_t j = 0; j < h; j++) {
                for (int32_t i = 0; i < w; i++) {
                    uint32_t cor = memoria.ler_palavra(addr);
                    if (cor != 0x00000000 && (x+i) >= 0 && (x+i) < 320 && (y+j) >= 0 && (y+j) < 240) {
                        memoria.escrever_palavra(FRAMEBUFFER_INICIO + (((y+j) * 320 + (x+i)) * 4), cor);
                    }
                    addr += 4;
                }
            }
            break;
        }

        case 0x22: { // GKEY ra, rb
            uint8_t reg_ra = (instr >> 22) & 0x0F; 
            uint8_t reg_rb = (instr >> 18) & 0x0F; 
            uint32_t id_tecla = regs[reg_rb];

            SDL_PumpEvents();
            const uint8_t* estado_teclado = SDL_GetKeyboardState(NULL);
            uint32_t resultado = 0;

            // Mapeamento Oficial das 16 Teclas da Especificação
            switch(id_tecla) {
                case 0:  if (estado_teclado[SDL_SCANCODE_LEFT]) resultado = 1; break;
                case 1:  if (estado_teclado[SDL_SCANCODE_RIGHT]) resultado = 1; break;
                case 2:  if (estado_teclado[SDL_SCANCODE_UP]) resultado = 1; break;
                case 3:  if (estado_teclado[SDL_SCANCODE_DOWN]) resultado = 1; break;
                case 4:  if (estado_teclado[SDL_SCANCODE_SPACE]) resultado = 1; break;
                case 5:  if (estado_teclado[SDL_SCANCODE_RETURN]) resultado = 1; break;
                case 6:  if (estado_teclado[SDL_SCANCODE_N]) resultado = 1; break;
                case 7:  if (estado_teclado[SDL_SCANCODE_M]) resultado = 1; break;
                case 8:  if (estado_teclado[SDL_SCANCODE_A]) resultado = 1; break;
                case 9:  if (estado_teclado[SDL_SCANCODE_S]) resultado = 1; break;
                case 10: if (estado_teclado[SDL_SCANCODE_D]) resultado = 1; break;
                case 11: if (estado_teclado[SDL_SCANCODE_W]) resultado = 1; break;
                case 12: if (estado_teclado[SDL_SCANCODE_Q]) resultado = 1; break;
                case 13: if (estado_teclado[SDL_SCANCODE_E]) resultado = 1; break;
                case 14: if (estado_teclado[SDL_SCANCODE_C]) resultado = 1; break;
                case 15: if (estado_teclado[SDL_SCANCODE_V]) resultado = 1; break;
            }

            regs[reg_ra] = resultado;
            break;
        }

        case 0x23: { // PLAY ra, rb, rc
            uint8_t reg_ra = (instr >> 22) & 0x0F; 
            uint8_t reg_rb = (instr >> 18) & 0x0F; 
            uint8_t reg_rc = (instr >> 14) & 0x0F; 
            // Executa o som de forma não-bloqueante chamando o subsistema de áudio
            audio.emitir_som(regs[reg_ra], regs[reg_rb], regs[reg_rc]);
            break;
        }

        case 0x24: { // SLEEP ra
            uint8_t ra = (instr >> 22) & 0x0F;
            uint32_t delay_ms = regs[ra];
            uint32_t start_time = SDL_GetTicks();
            while (SDL_GetTicks() - start_time < delay_ms) {
                SDL_PumpEvents(); // Mantém a janela e o SO responsivos
                SDL_Delay(1);     // Evita uso de 100% da CPU no loop
            }
            break;
        }

        case 0x25: // PSTR ra, rb, rc, rd
        
        case 0x26: { // PINT ra, rb, rc, rd
            // Mini-Fonte 8x8 (Tabela ASCII do caractere 32 ao 95: Espaço até '_')
            static const uint8_t font8x8[64][8] = {
                {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, {0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00},
                {0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00}, {0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00},
                {0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00}, {0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00},
                {0x38,0x6C,0x6C,0x38,0x6D,0x66,0x3B,0x00}, {0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00},
                {0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00}, {0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
                {0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00}, {0x00,0x18,0x18,0x7E,0x18,0x18,0x00,0x00},
                {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30}, {0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
                {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}, {0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00},
                {0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, {0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
                {0x3C,0x66,0x06,0x3C,0x60,0x66,0x7E,0x00}, {0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
                {0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00}, {0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
                {0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C,0x00}, {0x7E,0x66,0x0C,0x18,0x30,0x30,0x30,0x00},
                {0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, {0x3C,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00},
                {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30},
                {0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00}, {0x00,0x00,0x7E,0x00,0x7E,0x00,0x00,0x00},
                {0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00}, {0x3C,0x66,0x0C,0x18,0x18,0x00,0x18,0x00},
                {0x3C,0x66,0x6E,0x6E,0x60,0x66,0x3C,0x00}, {0x3C,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
                {0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00}, {0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},
                {0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00}, {0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00},
                {0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00}, {0x3C,0x66,0x60,0x6E,0x66,0x66,0x3E,0x00},
                {0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00}, {0x3E,0x18,0x18,0x18,0x18,0x18,0x3E,0x00},
                {0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00}, {0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},
                {0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00}, {0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
                {0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00}, {0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
                {0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, {0x3C,0x66,0x66,0x66,0x6A,0x6C,0x36,0x00},
                {0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00}, {0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00},
                {0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, {0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
                {0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00}, {0x63,0x63,0x6B,0x7F,0x77,0x63,0x63,0x00},
                {0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00}, {0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00},
                {0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00}, {0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00},
                {0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x00}, {0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00},
                {0x18,0x3C,0x66,0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00}
            };

            uint8_t reg_ra = (instr >> 22) & 0x0F; 
            uint8_t reg_rb = (instr >> 18) & 0x0F; 
            uint8_t reg_rc = (instr >> 14) & 0x0F; 
            uint8_t reg_rd = (instr >> 10) & 0x0F; 

            int32_t start_x = static_cast<int32_t>(regs[reg_ra]);
            int32_t start_y = static_cast<int32_t>(regs[reg_rb]);
            uint32_t cor = regs[reg_rd];
            
            std::string texto_final = "";

            if (opcode == 0x25) { // Lógica do PSTR (Ler da RAM)
                uint32_t addr = regs[reg_rc];
                bool fim = false;
                while (!fim) {
                    uint32_t palavra = memoria.ler_palavra(addr);
                    for (int b = 3; b >= 0; b--) { // Big-Endian: O 1º caractere está nos bits mais altos
                        char c = (palavra >> (b * 8)) & 0xFF;
                        if (c == '\0') { fim = true; break; }
                        texto_final += c;
                    }
                    addr += 4;
                }
            } else { // Lógica do PINT (Converter Inteiro para String)
                texto_final = std::to_string(static_cast<int32_t>(regs[reg_rc]));
            }

            // --- Motor de Renderização 16x16 ---
            for (char c : texto_final) {
                // Converte para maiúscula se for letra minúscula (a fonte só tem maiúsculas)
                if (c >= 'a' && c <= 'z') c -= 32; 

                int char_idx = c - 32; // Offset da tabela ASCII
                if (char_idx >= 0 && char_idx < 64) {
                    for (int row = 0; row < 8; row++) {
                        uint8_t linha_pixels = font8x8[char_idx][row];
                        for (int col = 0; col < 8; col++) {
                            if (linha_pixels & (1 << (7 - col))) {
                                // Dobra o pixel 8x8 para 16x16 (Escala 2x)
                                for (int dy = 0; dy < 2; dy++) {
                                    for (int dx = 0; dx < 2; dx++) {
                                        int px = start_x + (col * 2) + dx;
                                        int py = start_y + (row * 2) + dy;
                                        if (px >= 0 && px < 320 && py >= 0 && py < 240) {
                                            memoria.escrever_palavra(FRAMEBUFFER_INICIO + ((py * 320 + px) * 4), cor);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                start_x += 16; // Avança 16 pixels para a direita para a próxima letra
            }
            break;
        }

        case 0x27: { // SYSCALL ra, rb, rc, rd, re
            if (!opt_no_syscall) {
                uint8_t ra = (instr >> 22) & 0x0F, rb = (instr >> 18) & 0x0F;
                if (regs[ra] == 1) std::cout << "[SYSCALL] DEBUG: " << regs[rb] << "\n";
            }
            break;
        }

        case 0x28: { // SRAND ra
            uint8_t ra = (instr >> 22) & 0x0F;
            seed_lcg = regs[ra];
            break;
        }

        case 0x29: { // RAND ra, rb, rc
            // Isola os registradores do formato de 3 argumentos
            uint8_t reg_ra = (instr >> 22) & 0x0F; 
            uint8_t reg_rb = (instr >> 18) & 0x0F; 
            uint8_t reg_rc = (instr >> 14) & 0x0F; 

            // Algoritmo LCG (Linear Congruential Generator)
            seed_lcg = (1103515245 * seed_lcg + 12345) & 0x7FFFFFFF;
            int32_t min = regs[reg_rb]; 
            int32_t max = regs[reg_rc]; 
            
            if (max >= min) {
                regs[reg_ra] = min + (seed_lcg % (max - min + 1));
            } else {
                regs[reg_ra] = 0;
            }
            break;
        }

        case 0x2A: { // FRAMENUM ra
            uint8_t reg_ra = (instr >> 22) & 0x0F; 
            regs[reg_ra] = frames; 
            break;
        }

        case 0x2B: halt = true; break; // HALT
    
    }
    
    regs[0] = 0; 
}