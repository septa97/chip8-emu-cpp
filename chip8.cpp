#include <cstdio>
#include <cstdlib>
#define FONTSET_SIZE 80
#define GFX_SIZE 2048
#define KEYPAD_SIZE 16
#define MEMORY_SIZE 4096

class Chip8 {
    public:
        unsigned char gfx[GFX_SIZE];

    void initiliaze(const char *file_path) {
        pc = 0x200; // program counter starts at 0x200
        opcode = 0;
        I = 0;
        pc = 0;

        // clear the display
        for (int i = 0; i < GFX_SIZE; i++) {
            gfx[i] = 0;
        }

        // clear the stack, keypad, and V registers
        for (int i = 0; i < KEYPAD_SIZE; i++) {
            stack[i] = 0;
            key[i] = 0;
            V[i] = 0;
        }

        // load fontset into memory
        for (int i = 0; i < FONTSET_SIZE; i++) {
            memory[i] = fontset[i];
        }

        // reset timers
        delay_timer = 0;
        sound_timer = 0;

        printf("Loading ROM %s...\n", file_path);

        // Open ROM file
        FILE *rom_fp= fopen(file_path, "rb"); // read as bytes
        if (rom_fp == NULL) {
            printf("Failed to open ROM.\n");
            return;
        }

        // Get file size
        fseek(rom_fp, 0, SEEK_END); // set the position indicator to the end of the stream
        long rom_size = ftell(rom_fp); // return the number of bytes from the beginning of the file up to the current position indicator (that's why the position indicator is set using the `fseek` function call above)
        rewind(rom_fp); // set the position indicator to the beginning of the stream

        // Allocate memory to store ROM
        char *rom_buffer = (char *) malloc(sizeof(char) * rom_size);
        if (rom_buffer == NULL) {
            printf("Failed to allocate memory for ROM.\n");
            return;
        }

        // store the data from the stream `rom_fp` to the block of memory `rom_buffer`
        size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom_fp);
        if (result != rom_size) {
            printf("Failed to read ROM.\n");
            return;
        }

        // Copy buffer to memory
        if ((MEMORY_SIZE-512) > rom_size) {
            for (int i = 0; i < rom_size; i++) {
                memory[i+512] = rom_buffer[i]; // TODO: do I need to typecast?
            }
        } else {
            printf("ROM too large to fit in memory.\n");
            return;
        }

        // Clean up
        fclose(rom_fp);
        free(rom_buffer);
    }

    private:
        bool drawFlag;
        unsigned short opcode;
        unsigned char memory[MEMORY_SIZE];
        unsigned char V[16]; // CPU registers
        unsigned short I; // Index register
        unsigned short pc; // program counter
        unsigned char delay_timer;
        unsigned char sound_timer;
        unsigned short stack[16];
        unsigned short sp; // stack pointer
        unsigned char key[KEYPAD_SIZE]; // keypad
        static unsigned char fontset[FONTSET_SIZE];

    void emulateCycle() {
        // fetch opcode
        opcode = memory[pc] << 8 | memory[pc+1];

        // decode and execute opcode
        switch (opcode & 0xF000) {
            case 0x0000:
                switch (opcode & 0x0FFF) {
                    // 00E0 (display): Clears the screen.
                    case 0x00E0:
                        for (int i = 0; i < GFX_SIZE; i++) {
                            gfx[i] = 0;
                        }
                        drawFlag = true;
                        pc += 2;
                        break;
                    // 00EE (flow): Returns from a subroutine.
                    case 0x00EE:
                        sp--;
                        pc = stack[sp];
                        pc += 2;
                        break;
                    // 0NNN (call): Calls RCA 1802 program at address NNN. Not necessary for most ROMs.
                    
                    // default
                    default:
                        printf("Unknown opcode.\n");
                }
                break;
            // 1NNN (flow): Jumps to address NNN.
            case 0x1000:
                pc = opcode & 0x0FFF;
                break;
            // 2NNN (flow): Calls subroutine at NNN.
            case 0x2000:
                stack[sp] = pc;
                sp++;
                pc = opcode & 0x0FFF;
                break;
            // 3XNN (cond): Skips the next instruction if VX equals NN.
            case 0x3000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short NN = opcode & 0x00FF;

                if (V[X] == NN) {
                    pc += 4;
                } else {
                    pc += 2;
                }
                break;
            }
            // 4XNN (cond): Skips the next instruction if VX is not equal to NN.
            case 0x4000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short NN = opcode & 0x00FF;

                if (V[X] == NN) {
                    pc += 4;
                } else {
                    pc += 2;
                }
                break;
            }
            // 5XY0 (cond): Skips the next instruction if VX is equal to VY.
            case 0x5000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short Y = (opcode & 0x00F0) >> 4;

                if (V[X] == V[Y]) {
                    pc += 4;
                } else {
                    pc += 2;
                }
                break;
            }
            // 6XNN (const): Sets VX to NN.
            case 0x6000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short NN = opcode & 0x00FF;

                V[X] = NN;
                pc += 2;
                break;
            }
            // 7XNN (const): Adds NN to VX.
            case 0x7000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short NN = opcode & 0x00FF;

                V[X] += NN;
                pc += 2;
                break;
            }
            case 0x8000:
                switch (opcode & 0xF00F) {
                    // 8XY0 (assign): Sets VX to the value of VY.
                    case 0x8000: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        unsigned short Y = (opcode & 0x00F0) >> 4;

                        V[X] = V[Y];
                        pc += 2;
                        break;
                    }
                    // 8XY1 (bitOp): Sets VX to VX bitwise or VY.
                    case 0x8001: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        unsigned short Y = (opcode & 0x00F0) >> 4;

                        V[X] |= V[Y];
                        pc += 2;
                        break;
                    }
                    // 8XY2 (bitOp): Sets VX to VX bitwise and VY.
                    case 0x8002: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        unsigned short Y = (opcode & 0x00F0) >> 4;

                        V[X] &= V[Y];
                        pc += 2;
                        break;
                    }
                    // 8XY3 (bitOp): Sets VX to VX xor VY.
                    case 0x8003: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        unsigned short Y = (opcode & 0x00F0) >> 4;

                        V[X] ^= V[Y];
                        pc += 2;
                        break;
                    }
                    // 8XY4 (math): Adds VY to VX. VF is set to 1 when there's a carry, 0 when there's none.
                    case 0x8004: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        unsigned short Y = (opcode & 0x00F0) >> 4;

                        // TODO: check if this is correct
                        if (V[X] + V[Y] > 0xFF) {
                            V[0xF] = 1;
                        } else {
                            V[0xF] = 0;
                        }

                        V[X] += V[Y];
                        pc += 2;
                        break;
                    }
                    // 8XY5 (math): VY is subtracted from VX. VF is set to 0 when there's a borrow, 1 when there's none.
                    case 0x8005: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        unsigned short Y = (opcode & 0x00F0) >> 4;

                        // TODO: check if this is correct
                        if (V[X] - V[Y] < 0) {
                            V[0xF] = 0;
                        } else {
                            V[0xF] = 1;
                        }

                        V[X] -= V[Y];
                        pc += 2;
                        break;
                    }
                    // 8XY6 (bitOp): Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    // TODO: tf is the purpose of Y here?
                    case 0x8006: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        V[0xF] = V[X] & 1;
                        V[X] >>= 1;

                        pc += 2;
                        break;
                    }
                    // 8XY7 (math): Sets VX to VY - VX. VF is set to 0 when there's a borrow, 1 when there's none.
                    case 0x8007: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        unsigned short Y = (opcode & 0x00F0) >> 4;

                        if (V[Y] - V[X] < 0) {
                            V[0xF] = 0;
                        } else {
                            V[0xF] = 1;
                        }
                        
                        V[X] = V[Y] - V[X];
                        pc += 2;
                        break;
                    }
                    // 8XYE (bitOp): Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    // TODO: tf is the purpose of Y here?
                    case 0x800E: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        V[0xF] = V[X] >> 7;
                        V[X] <<= 1;

                        pc += 2;
                        break;
                    }
                    default:
                        printf("Unknown opcode.\n");
                }
            // 9XY0 (cond): Skips the next instruction if VX is not equal to VY.
            case 0x9000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short Y = (opcode & 0x00F0) >> 4;

                if (V[X] != V[Y]) {
                    pc += 4;
                } else {
                    pc += 2;
                }
                break;
            }
            // ANNN (MEM): Sets I to the address NNN.
            case 0xA000:
                I = opcode & 0x0FFF;
                pc += 2;
                break;
            // BNNN (flow): Jumps to the address NNN plus V0.
            case 0xB000:
                pc = (opcode & 0x0FFF) + V[0];
                break;
            // CXNN (rand): Sets VX to the result of a `bitwise and` operation on a random number (typically, from 0 to 255) and NN.
            case 0xC000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short NN = opcode & 0x00FF;
                unsigned char num = rand() % 0xFF;

                V[X] = num & NN;
                pc += 2;
                break;
            }
            // DXYN (disp): Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
            //              Each row of 8 pixels is read as bit-coded starting from memory location I;
            //              I value doesn't change after the execution of this instruction.
            //              As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
            //              and to 0 if that doesn't happen.
            case 0xD000: {
                unsigned short X = (opcode & 0x0F00) >> 8;
                unsigned short Y = (opcode & 0x00F0) >> 4;
                unsigned short N = opcode & 0x000F;
                unsigned short pixel;

                V[0xF] = 0;

                for (int y = 0; y < N; y++) {
                    pixel = memory[I+y];

                    for (int x = 0; x < 8; x++) {
                        // TODO: understand this
                        if ((pixel & (0x80 >> x)) != 0) {
                            if (gfx[V[X] + x + ((V[Y] + y) * 64)] == 1) {
                                V[0xF] = 1;
                            }

                            gfx[V[X] + x + ((V[Y] + y) * 64)] ^= 1;
                        }
                    }
                }

                drawFlag = true;
                pc += 2;
                break;
            }
            case 0xE000:
                switch (opcode & 0xF0FF) {
                    // EX9E (keyOp): Skips the next instruction if the key stored in VX is pressed.
                    case 0xE09E: {
                        unsigned short X = (opcode & 0x0F00) >> 8;

                        if (key[V[X]] != 0) {
                            pc += 4;
                        } else {
                            pc += 2;
                        }
                        break;
                    }
                    // EXA1 (keyOp): Skips the next instruction if the key stored in VX is not pressed.
                    case 0xE0A1: {
                        unsigned short X = (opcode & 0x0F00) >> 8;

                        if (key[V[X]] == 0) {
                            pc += 4;
                        } else {
                            pc += 2;
                        }
                        break;
                    }
                }
                break;
            case 0xF000:
                switch (opcode & 0xF0FF) {
                    // FX07 (timer): Sets VX to the value of the delay timer
                    case 0xF007: {
                        unsigned short X = (opcode & 0x0F00) >> 8;

                        V[X] = delay_timer;
                        pc += 2;
                        break;
                    }
                    // FX0A (keyOp): A key press is awaited, and then stored in VX.
                    //               (Blocking operation. All instruction halted until next key event.)
                    case 0xF00A: {
                        bool keyPressed = false;

                        for (int i = 0; i < KEYPAD_SIZE; i++) {
                            if (key[i] != 0) {
                                unsigned short X = (opcode & 0x0F00) >> 8;
                                V[X] = i;
                                keyPressed = true;
                            }
                        }

                        if (keyPressed) {
                            pc += 2;
                        }
                        break;
                    }
                    // FX15 (timer): Sets the delay timer to VX.
                    case 0xF015: {
                        unsigned short X = (opcode & 0x0F00) >> 8;

                        delay_timer = V[X];
                        pc += 2;
                        break;
                    }
                    // FX18 (sound): Sets the sound timer to VX.
                    case 0xF018: {
                        unsigned short X = (opcode & 0x0F00) >> 8;

                        sound_timer = V[X];
                        pc += 2;
                        break;
                    }
                    // FX1E (MEM): Adds VX to I. VF is set to 1 when there is a range overflow (I + VX > 0xFFF), and to 0 when there isn't
                    case 0xF01E: {
                        unsigned short X = (opcode & 0x0F00) >> 8;

                        if (I + V[X] > 0xFFF) {
                            V[0xF] = 1;
                        } else {
                            V[0xF] = 0;
                        }

                        I += V[X];
                        pc += 2;
                        break;
                    }
                    // FX33 (MEM): Stores the binary-coded decimal (BCD) representation of VX,
                    //              with the most significant of three digits at the address in I,
                    //              the middle digit at I+1,
                    //              and the least significant digit at I+2.
                    case 0xF033: {
                        unsigned short X = (opcode & 0x0F00) >> 8;
                        memory[I] = V[X] / 100;
                        memory[I+1] = (V[X] / 10) % 10;
                        memory[I+2] = V[X] % 10;
                        break;
                    }
                }
                break;

            // default
            default:
                printf("Unknown opcode.\n");
        }

        // update timers
        if (delay_timer > 0) {
            delay_timer--;
        }

        if (sound_timer > 0) {
            if (sound_timer == 1) {
                printf("BEEP!\n");
            }

            sound_timer--;
        }
    }
};

unsigned char Chip8::fontset[FONTSET_SIZE] = 
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
