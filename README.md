
# Tarefa: Roteiro de FreeRTOS #2 - EmbarcaTech 2025

Autor: **Maria Eduarda Araujo Pereira e Pedro Tulio Curvelo Camilo**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Junho de 2025

---

# Jogo de Desviar para Raspberry Pi Pico com FreeRTOS

Um jogo simples no estilo "dodge" (desviar) implementado em C para o Raspberry Pi Pico. O projeto utiliza uma matriz de LED 5x5 para exibir o jogo, um display OLED para informações de nível, e o sistema operacional de tempo real FreeRTOS para gerenciar as tarefas de forma concorrente.

## Demonstração

![Demonstração do Jogo]([URL_DO_SEU_VIDEO_OU_GIF_AQUI](https://youtube.com/shorts/7n6nybHZgOw?feature=share))

## Funcionalidades

- **Jogabilidade Dinâmica:** Controle um pixel verde na base da matriz e desvie dos obstáculos vermelhos que caem.
- **Dificuldade Progressiva:** A velocidade com que os obstáculos caem aumenta conforme o jogador avança de nível.
- **Display de Status:** Um display OLED SSD1306 mostra o nível de dificuldade atual.
- **Feedback Sonoro:** O jogo conta com uma música de fundo e um som característico ao perder.
- **Multitarefa com FreeRTOS:** O projeto é estruturado em várias tarefas que rodam simultaneamente:
    - Controle dos botões
    - Geração de obstáculos
    - Renderização do jogo na matriz de LED
    - Reprodução da música de fundo

## Hardware Necessário

- 1x Raspberry Pi Pico
- 1x Matriz de LED 5x5 (25 pixels WS2812B) ou similar
- 1x Display OLED 0.96" I2C (SSD1306)
- 1x Buzzer passivo
- 2x Botões de pressão (Push Buttons)
- 1x Protoboard e fios para conexão

## Pinagem (Pinout)

| Componente              | Pino na Raspberry Pi Pico |
| :---------------------- | :------------------------ |
| **Matriz de LED (Sinal)** | GPIO 7                    |
| **Botão A (Esquerda)** | GPIO 5                    |
| **Botão B (Direita)** | GPIO 6                    |
| **Buzzer** | GPIO 21                   |
| **Display OLED (SDA)** | GPIO 14                   |
| **Display OLED (SCL)** | GPIO 15                   |

## Como Compilar e Rodar

Este projeto utiliza o SDK do Raspberry Pi Pico e o CMake.

1.  **Clone o repositório:**
    ```bash
    git clone [https://github.com/seu-usuario/seu-repositorio.git](https://github.com/seu-usuario/seu-repositorio.git)
    ```
2.  **Configure o ambiente:** Certifique-se de que o caminho para o `PICO_SDK_PATH` está configurado corretamente no seu ambiente.

3.  **Compile o projeto:**
    ```bash
    cd seu-repositorio
    mkdir build
    cd build
    cmake ..
    ninja
    ```
    *(ou `make` no lugar de `ninja`)*

4.  **Flashe o firmware:** Com o Pico em modo BOOTSEL, copie o arquivo `.uf2` gerado na pasta `build` para o dispositivo.
    ```bash
    cp main.uf2 /media/seu-usuario/RPI-RP2
    ```

---

## 📜 Licença
GNU GPL-3.0.
