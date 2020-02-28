#include <cstdio>
#include <cstdlib>

void disassemble(unsigned char *buffer, unsigned short pc) { // TODO: research on uint8_t
    unsigned char *code = &buffer[pc];
    unsigned char first_nibble = code[0] >> 4;

    printf("%04x %02x %02x\n", pc, code[0], code[1]);
    switch (first_nibble) {
        case 0x0: printf("0 is not implemented yet."); break;
        case 0x1: printf("1 is not implemented yet."); break;
        case 0x2: printf("2 is not implemented yet."); break;
        case 0x3: printf("3 is not implemented yet."); break;
        case 0x4: printf("4 is not implemented yet."); break;
        case 0x5: printf("5 is not implemented yet."); break;
        case 0x6: {
            unsigned char X = code[0] & 0x0F;
            printf("%-10s V%01x, #$%02x", "MVI", X, code[1]);
            break;
        }
        case 0x7: printf("7 is not implemented yet."); break;
        case 0x8: printf("8 is not implemented yet."); break;
        case 0x9: printf("9 is not implemented yet."); break;
        case 0xA: {
            unsigned char addressI = code[0] & 0x0F;
            printf("%-10s I, #$%01x%02x", "MVI", addressI, code[1]);
            break;
        }
        case 0xB: printf("B is not implemented yet."); break;
        case 0xC: printf("C is not implemented yet."); break;
        case 0xD: printf("D is not implemented yet."); break;
        case 0xE: printf("E is not implemented yet."); break;
        case 0xF: printf("F is not implemented yet."); break;
    }
}

int main(int argc, char *argv[]) {
    FILE *fp = fopen(argv[1], "rb");

    if (fp == NULL) {
        printf("Failed to open ROM.\n");
        return 1;
    }

    fseek(fp, 0, SEEK_END); // pointer to the end of stream
    long rom_size = ftell(fp); // get the number of bytes
    fseek(fp, 0, SEEK_SET); // pointer to the start of stream

    printf("rom size: %ld bytes.\n", rom_size);
    unsigned char *buffer = (unsigned char *)malloc(rom_size + 0x200);
    fread(buffer + 0x200, rom_size, 1, fp);
    fclose(fp);

    unsigned short pc = 0x200;
    while (pc < rom_size+0x200) {
        disassemble(buffer, pc);
        pc += 2;
        printf("\n");
    }

    return 0;
}
