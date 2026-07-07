# Arquivo Makefile para compilação modular da VM do Fantasys32
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# Vincula as flags de linkagem dinâmica da biblioteca SDL2 (Gráficos, Teclado e Áudio)
LIBS = -lSDL2

# Inclusão obrigatória de 'audio.cpp' na lista de arquivos de origem do compilador
SRCS = main.cpp memory.cpp cpu.cpp display.cpp input.cpp audio.cpp
TARGET = vm

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)