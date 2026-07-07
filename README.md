# FantasyS32 - Projeto de Aspectos de Linguagens de Programação

Este repositório contém o jogo/projeto desenvolvido em Assembly para a máquina virtual FantasyS32.

## 🛠️ Desenvolvimento

- **Jogo/Implementação:** Daniel Suzuki Naves e Vitor Hugo Pires De Moraes
- **Ferramental (Pasta `/assembler`):** Desenvolvido e fornecido pelo Professor e Doutor Juliano Henrique Foleis, para disciplina: Aspectos de Linguagens de Programação, como base para a atividade acadêmica.

---

## 🎮 O Projeto: Snake Game (Jogo da Cobrinha)

Este projeto consiste na implementação completa do clássico jogo *Snake* escrito puramente em linguagem **Assembly** para a arquitetura customizada de 32-bits do ecossistema **FantasyS32**. O programa gerencia o laço principal de renderização de vídeo (60 FPS), controle de entrada por teclado, lógica de colisão, crescimento do corpo da cobra, pontuação e geração aleatória de comida através do algoritmo LCG (*Linear Congruential Generator*).

---

## 💻 Ambiente de Execução (Importante)

O desenvolvimento, compilação e teste deste ecossistema foram realizados utilizando o **WSL (Windows Subsystem for Linux)**. 

> [!IMPORTANT]
> Como os binários fornecidos (`vm` e `assembler`) foram compilados para ambientes Linux, quem clonar este repositório no Windows **deve utilizar o terminal do WSL** para conseguir executar os comandos abaixo com sucesso.

### Pré-requisitos do Sistema
A Máquina Virtual utiliza a biblioteca gráfica **SDL2** para a renderização do Framebuffer e gerenciamento de janelas. Caso o ambiente que vai rodar o jogo não possua a biblioteca instalada, execute o seguinte comando no terminal do WSL/Linux:
  ```bash
  sudo apt-get update
  sudo apt-get install libsdl2-dev
  ```

## 🚀 Como Executar o Jogo
Como todos os executáveis necessários já estão disponíveis na raiz e nas pastas deste repositório, você não precisa compilar nada. Basta abrir o terminal no diretório do projeto e iniciar a máquina virtual alimentando-a com o binário do jogo.

Dê permissão de execução para o arquivo da VM (caso necessário):
  ```bash
  chmod +x vm
  ```
Execute o jogo diretamente na Máquina Virtual:
  ```bash
  ./vm --scale 2 ../jogo/snake_game.bin
  ```

## 🛠️ Como Compilar o Código Fonte (Opcional)
Se você fizer alterações no arquivo código fonte (.asm) do jogo e quiser gerar um novo arquivo binário executável (.bin), utilize o **Assembler** fornecido na pasta do projeto seguindo os passos abaixo:

1. Acesse o terminal na pasta do projeto.
2. Certifique-se de que o montador possui permissões de execução:
   ```bash
   chmod +x assembler
3. Execute o comando de montagem especificando o arquivo de entrada e o de saída:
   ```bash
   ./assembler snake.asm snake_game.bin
4. Após gerar o novo .bin, basta rodar o comando da seção anterior utilizando a **./vm**.

## ⌨️ Controles do Jogo
Ao iniciar a VM com o jogo, utilize as seguintes teclas para controlar a cobrinha:
* **W**: Mover para Cima
* **S**: Mover para Baixo
* **A**: Mover para a Esquerda
* **D**: Mover para a Direita
