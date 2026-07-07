; ===================
; Fantasys32 - Snake 
; ===================

.data
; Paleta de Cores
.equ GRAMA, 0xFF1A401A
.equ GRAMA_ESCURA, 0xFF0D2B0D
.equ BORDA, 0xFF4CAF50
.equ COBRA_COR, 0xFF32CD32
.equ MACA, 0xFFD32F2F
.equ TEXTO_COR, 0xFFFFFFFF

txt_score: .string "SCORE: "
txt_best:  .string "HI: "
txt_gover: .string "GAME OVER!"

.text

BOOT:
    ; Zera o High Score apenas quando a VM liga (Endereço 3980)
    MOVL R1, 0
    MOVL R2, 3980
    STORE R1, R2, 0

START:
    MOVL R6, 160             ; Cabeça X
    MOVL R7, 120             ; Cabeça Y
    MOVL R8, 2               ; Direção
    MOVL R9, 200             ; Maçã X
    MOVL R10, 150            ; Maçã Y
    MOVL R11, 15             ; Tamanho da cobra
    MOVL R12, 4000           ; Base RAM do Ring Buffer

    ; Zera Variáveis da Partida Atual
    MOVL R1, 0
    MOVL R2, 3992            ; Score
    STORE R1, R2, 0
    MOVL R2, 3988            ; Grow Counter
    STORE R1, R2, 0
    MOVL R2, 3984            ; Tail Pointer
    STORE R1, R2, 0

    ; Inicia buffer preenchido
    MOVL R13, 0
    ADD R14, R12, R0
INIT_LOOP:
    MOVL R1, 15
    BGE R13, R1, FIM_INIT
    STORE R6, R14, 0
    STORE R7, R14, 1
    MOVL R1, 8
    ADD R14, R14, R1
    INC R13
    JMP INIT_LOOP
FIM_INIT:
    MOVL R1, 112             
    MOVL R2, 3996            ; Head Pointer
    STORE R1, R2, 0

GAME_LOOP:
    ; --- 1. TECLADO ---
    MOVL R2, 1               
    GKEY R1, R2              
    MOVL R4, 1
    BEQ R1, R4, TENTA_DIR
    MOVL R2, 0               
    GKEY R1, R2              
    BEQ R1, R4, TENTA_ESQ
    MOVL R2, 2               
    GKEY R1, R2              
    BEQ R1, R4, TENTA_CIM
    MOVL R2, 3               
    GKEY R1, R2              
    BEQ R1, R4, TENTA_BAI
    JMP VERIFICAR_ESTADO

TENTA_DIR:
    MOVL R1, 0                   ; 0 = ESQUERDA
    BEQ R8, R1, VERIFICAR_ESTADO 
    MOVL R8, 1               
    JMP VERIFICAR_ESTADO
TENTA_ESQ:
    MOVL R1, 1                   ; 1 = DIREITA
    BEQ R8, R1, VERIFICAR_ESTADO 
    MOVL R8, 0               
    JMP VERIFICAR_ESTADO
TENTA_CIM:
    MOVL R1, 4                   ; 4 = BAIXO
    BEQ R8, R1, VERIFICAR_ESTADO 
    MOVL R8, 3
    JMP VERIFICAR_ESTADO
TENTA_BAI:
    MOVL R1, 3                   ; 3 = CIMA
    BEQ R8, R1, VERIFICAR_ESTADO
    MOVL R8, 4

VERIFICAR_ESTADO:
    MOVL R1, 2
    BEQ R8, R1, RENDERIZAR

    ; --- 2. MOVIMENTO FLUIDO (2 px) ---
    MOVL R4, 1
    BEQ R8, R4, MOV_DIR
    BEQ R8, R0, MOV_ESQ
    MOVL R4, 3
    BEQ R8, R4, MOV_CIM
    MOVL R4, 4
    BEQ R8, R4, MOV_BAI
    JMP LIMITES_X

MOV_DIR:
    INC R6
    INC R6
    JMP LIMITES_X
MOV_ESQ:
    DEC R6
    DEC R6
    JMP LIMITES_X
MOV_CIM:
    DEC R7
    DEC R7
    JMP LIMITES_X
MOV_BAI:
    INC R7
    INC R7

    ; --- 3. LIMITES DAS BORDAS ---
LIMITES_X:
    MOVL R1, 2                   
    BLT R6, R1, COLISAO_MORTE
    MOVL R1, 308                 
    BGT R6, R1, COLISAO_MORTE
LIMITES_Y:
    MOVL R1, 32                  
    BLT R7, R1, COLISAO_MORTE
    MOVL R1, 228                 
    BGT R7, R1, COLISAO_MORTE

    ; --- 4. RING BUFFER: ATUALIZA CABEÇA E RABO ---
    MOVL R2, 3996
    LOAD R1, R2, 0           
    MOVL R3, 8
    ADD R1, R1, R3
    MOVL R3, 32000           
    BLT R1, R3, NO_WRAP_HEAD
    MOVL R1, 0
NO_WRAP_HEAD:
    STORE R1, R2, 0          
    ADD R14, R12, R1         
    STORE R6, R14, 0
    STORE R7, R14, 1

    ; Controle de Crescimento Sincronizado
    MOVL R2, 3988
    LOAD R1, R2, 0
    BGT R1, R0, IS_GROWING

    MOVL R2, 3984
    LOAD R13, R2, 0          
    MOVL R3, 8
    ADD R13, R13, R3
    MOVL R3, 32000
    BLT R13, R3, NO_WRAP_TAIL
    MOVL R13, 0
NO_WRAP_TAIL:
    STORE R13, R2, 0
    JMP VERIFICA_CAUDA

IS_GROWING:
    DEC R1
    STORE R1, R2, 0
    INC R11                  

    ; --- 5. COLISÃO MATEMÁTICA ---
VERIFICA_CAUDA:
    MOVL R13, 30
    BGE R13, R11, SISTEMA_COLISAO  
    SUB R13, R11, R13              

    MOVL R2, 3984
    LOAD R14, R2, 0
    ADD R14, R14, R12              
    MOVL R4, 36000                 

LOOP_CAUDA:
    LOAD R1, R14, 0
    LOAD R2, R14, 1

    SUB R3, R6, R1
    BGE R3, R0, CAIXA_X
    SUB R3, R0, R3
CAIXA_X:
    MOVL R5, 8
    BGE R3, R5, PROX_CAUDA

    SUB R3, R7, R2
    BGE R3, R0, CAIXA_Y
    SUB R3, R0, R3
CAIXA_Y:
    MOVL R5, 8
    BGE R3, R5, PROX_CAUDA

    JMP COLISAO_MORTE

PROX_CAUDA:
    MOVL R3, 24              
    ADD R14, R14, R3
    BLT R14, R4, NO_WRAP_CAUDA
    MOVL R3, 32000
    SUB R14, R14, R3
NO_WRAP_CAUDA:
    MOVL R3, 3
    SUB R13, R13, R3
    BGT R13, R0, LOOP_CAUDA

    ; --- 6. COMER A MAÇÃ ---
SISTEMA_COLISAO:
    MOVL R1, 10
    ADD R2, R9, R1
    BGE R6, R2, RENDERIZAR
    ADD R2, R6, R1
    BLE R2, R9, RENDERIZAR
    ADD R2, R10, R1
    BGE R7, R2, RENDERIZAR
    ADD R2, R7, R1
    BLE R2, R10, RENDERIZAR

    MOVL R1, 880
    MOVL R2, 100
    MOVL R3, 1
    PLAY R1, R2, R3

    ; Aumenta o crescimento da cobra
    MOVL R2, 3988
    LOAD R1, R2, 0
    MOVL R3, 10
    ADD R1, R1, R3
    STORE R1, R2, 0

    ; Aumenta a pontuação no placar
    MOVL R2, 3992
    LOAD R1, R2, 0
    MOVL R3, 10
    ADD R1, R1, R3
    STORE R1, R2, 0

NOVA_MACA:
    ; Sorteia X da Maçã
    MOVL R1, 0
    MOVL R2, 29   
    RAND R9, R1, R2
    MOVL R1, 10
    MUL R9, R9, R1

    ; Sorteia Y da Maçã
    MOVL R1, 4    
    MOVL R2, 22   
    RAND R10, R1, R2
    MOVL R1, 10
    MUL R10, R10, R1

    ; --- VERIFICA SE NASCEU EM CIMA DA COBRA ---
    MOVL R2, 3984
    LOAD R14, R2, 0          ; R14 = Ponteiro do Rabo
    ADD R14, R14, R12        ; Soma a base do Ring Buffer
    MOVL R13, 0              ; R13 = Contador de blocos verificados
    MOVL R4, 36000           ; Limite superior da RAM para o Wrap

LOOP_VERIFICA_MACA:
    LOAD R1, R14, 0          ; Lê X do corpo
    LOAD R2, R14, 1          ; Lê Y do corpo

    ; Compara as coordenadas
    BNE R1, R9, SEGMENTO_SEGURO  ; Se o X for diferente, está seguro
    BEQ R2, R10, NOVA_MACA       ; Se o X e o Y forem iguais, COLIDIU! Sorteia de novo!

SEGMENTO_SEGURO:
    MOVL R3, 8
    ADD R14, R14, R3         ; Avança o ponteiro para o próximo bloco da cobra
    BLT R14, R4, NO_WRAP_VM
    MOVL R3, 32000
    SUB R14, R14, R3         ; Faz a volta no Ring Buffer se necessário
NO_WRAP_VM:
    INC R13                  ; Incrementa o contador
    BLT R13, R11, LOOP_VERIFICA_MACA ; Repete até checar o tamanho total da cobra (R11)

    ; --- 7. RENDERIZAÇÃO COMPLETA ---
RENDERIZAR:
    MOVL R1, GRAMA.l
    MOVH R1, GRAMA.h
    CLEAR R1                 

    ; --- Desenha a Grama ---
    MOVL R13, 50
LOOP_GRAMA_Y:
    MOVL R1, 230
    BGE R13, R1, FIM_GRAMA
    MOVL R14, 20
LOOP_GRAMA_X:
    MOVL R1, 320
    BGE R14, R1, PROX_GRAMA_Y
    MOVL R3, 4
    MOVL R4, 2
    MOVL R5, GRAMA_ESCURA.l
    MOVH R5, GRAMA_ESCURA.h
    RECT R14, R13, R3, R4, R5
    MOVL R1, 40
    ADD R14, R14, R1
    JMP LOOP_GRAMA_X
PROX_GRAMA_Y:
    MOVL R1, 40
    ADD R13, R13, R1
    JMP LOOP_GRAMA_Y
FIM_GRAMA:

    ; --- Desenha as Bordas da Arena ---
    MOVL R1, 0
    MOVL R2, 30
    MOVL R3, 320
    MOVL R4, 2
    MOVL R5, BORDA.l
    MOVH R5, BORDA.h
    RECT R1, R2, R3, R4, R5  ; Topo
    MOVL R2, 238
    RECT R1, R2, R3, R4, R5  ; Fundo
    MOVL R2, 30
    MOVL R3, 2
    MOVL R4, 210
    RECT R1, R2, R3, R4, R5  ; Esquerda
    MOVL R1, 318
    RECT R1, R2, R3, R4, R5  ; Direita

    ; --- Maçã ---
    MOVL R3, 2
    ADD R1, R9, R3           
    ADD R2, R10, R0          
    MOVL R3, 6               
    MOVL R4, 10              
    MOVL R5, MACA.l          
    MOVH R5, MACA.h          
    RECT R1, R2, R3, R4, R5
    ADD R1, R9, R0           
    MOVL R3, 2
    ADD R2, R10, R3          
    MOVL R3, 10              
    MOVL R4, 6               
    RECT R1, R2, R3, R4, R5

    ; --- Desenha Cobra ---
    MOVL R2, 3984
    LOAD R14, R2, 0
    ADD R14, R14, R12        
    MOVL R13, 0              
    MOVL R4, 36000           
LOOP_DESENHO:
    LOAD R1, R14, 0
    LOAD R2, R14, 1
    MOVL R3, 10
    MOVL R5, COBRA_COR.l
    MOVH R5, COBRA_COR.h
    RECT R1, R2, R3, R3, R5

    MOVL R3, 8
    ADD R14, R14, R3
    BLT R14, R4, NO_WRAP_REN
    MOVL R3, 32000
    SUB R14, R14, R3
NO_WRAP_REN:
    INC R13
    BLT R13, R11, LOOP_DESENHO

    ; --- Desenha Cabeça ---
    MOVL R3, 10
    MOVL R5, COBRA_COR.l
    MOVH R5, COBRA_COR.h
    RECT R6, R7, R3, R3, R5

    ; --- HUD: Score ---
    MOVL R1, 10                  
    MOVL R2, 10                  
    MOVL R3, txt_score.l
    MOVH R3, txt_score.h
    MOVL R4, TEXTO_COR.l
    MOVH R4, TEXTO_COR.h
    PSTR R1, R2, R3, R4

    MOVL R1, 130                 
    MOVL R2, 10
    MOVL R4, 3992
    LOAD R3, R4, 0
    MOVL R4, TEXTO_COR.l
    MOVH R4, TEXTO_COR.h
    PINT R1, R2, R3, R4          

    ; --- HUD: High Score ---
    MOVL R1, 200                 
    MOVL R2, 10                  
    MOVL R3, txt_best.l
    MOVH R3, txt_best.h
    MOVL R4, TEXTO_COR.l
    MOVH R4, TEXTO_COR.h
    PSTR R1, R2, R3, R4

    MOVL R1, 260                 
    MOVL R2, 10
    MOVL R4, 3980
    LOAD R3, R4, 0
    MOVL R4, TEXTO_COR.l
    MOVH R4, TEXTO_COR.h
    PINT R1, R2, R3, R4          

FIM_DESENHO:
    FRAMENUM R1              
SYNC_LOOP:
    FRAMENUM R2              
    BEQ R1, R2, SYNC_LOOP    
    JMP GAME_LOOP

COLISAO_MORTE:
    ; Verifica Recorde
    MOVL R2, 3992
    LOAD R1, R2, 0           ; R1 = Score Atual
    MOVL R2, 3980
    LOAD R3, R2, 0           ; R3 = High Score
    BLE R1, R3, MORTE_SOM    ; Se Atual <= High, pula
    STORE R1, R2, 0          ; Novo Recorde salvo na RAM!

MORTE_SOM:
    MOVL R1, 150
    MOVL R2, 600
    MOVL R3, 2
    PLAY R1, R2, R3

GAME_OVER:
    MOVL R1, 80                  
    MOVL R2, 100                 
    MOVL R3, txt_gover.l
    MOVH R3, txt_gover.h
    MOVL R4, MACA.l              
    MOVH R4, MACA.h
    PSTR R1, R2, R3, R4
    FRAMENUM R1              
SYNC_MORTE:
    FRAMENUM R2              
    BEQ R1, R2, SYNC_MORTE    

AGUARDA_ESPACO:
    MOVL R2, 4                   
    GKEY R1, R2              
    BEQ R1, R0, AGUARDA_ESPACO   
    JMP START                ; JMP para START mantém o Recorde vivo!
    HALT
