#include <cstdio>
#include <cstdlib>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

void disassemble(uint8_t *buffer, uint16_t pc) {
    uint8_t *code = &buffer[pc];
    uint8_t first_nibble = code[0] >> 4;

    printf("%04x %02x %02x: ", pc, code[0], code[1]);
    switch (first_nibble) {
        case 0x0:
            switch ((code[0] << 8) | code[1]) {
                case 0x00E0: printf("%-10s", "CLS"); break;
                case 0x00EE: printf("%-10s", "RTS"); break;
                default: printf("Only needed if emulating the RCA 1802 processor"); break;
            }
            break;
        case 0x1: printf("%-10s $%01x%02x", "JUMP", code[0]&0xF, code[1]); break;
        case 0x2: printf("%-10s $%01x%02x", "CALL", code[0]&0xF, code[1]); break;
        case 0x3: printf("%-10s V%01x, #$%02x", "SKIP.EQ", code[0]&0xF, code[1]); break;
        case 0x4: printf("%-10s V%01x, #$%02x", "SKIP.NE", code[0]&0xF, code[1]); break;
        case 0x5: printf("%-10s V%01x, V%01x", "SKIP.EQ", code[0]&0xF, code[1]>>4); break;
        case 0x6: printf("%-10s V%01x #$%02x", "MVI", code[0]&0xF, code[1]); break;
        case 0x7: printf("%-10s V%01x #$%02x", "ADD", code[0]&0xF, code[1]); break;
        case 0x8:
            switch (code[1] & 0xF) {
                case 0x00: printf("%-10s V%01x, V%01x", "MOV", code[0]&0xF, code[1]>>4); break;
                case 0x01: printf("%-10s V%01x, V%01x", "OR", code[0]&0xF, code[1]>>4); break;
                case 0x02: printf("%-10s V%01x, V%01x", "AND", code[0]&0xF, code[1]>>4); break;
                case 0x03: printf("%-10s V%01x, V%01x", "XOR", code[0]&0xF, code[1]>>4); break;
                case 0x04: printf("%-10s V%01x, V%01x", "ADD.", code[0]&0xF, code[1]>>4); break;
                case 0x05: printf("%-10s V%01x, V%01x", "SUB.", code[0]&0xF, code[1]>>4); break;
                case 0x06: printf("%-10s V%01x", "SHR.", code[0]&0xF); break;
                case 0x07: printf("%-10s V%01x, V%01x", "SUBB.", code[0]&0xF, code[1]>>4); break;
                case 0x0E: printf("%-10s V%01x", "SHL.", code[0]&0xF); break;
                default: printf("Unknown 8 code 0x8%01x%02x", code[0]&0xF, code[1]);
            }
            break;
        case 0x9: printf("%-10s V%01x, V%01x", "SKIP.NE", code[0]&0xF, code[1]>>4); break;
        case 0xA: printf("%-10s I, #$%01x%02x", "MVI", code[0]&0xF, code[1]); break;
        case 0xB: printf("%-10s $%01x%02x(V0)", "JUMP", code[0]&0xF, code[1]); break;
        case 0xC: printf("%-10s V%01x, #$%02x", "RNDMSK", code[0]&0xF, code[1]); break;
        case 0xD: printf("%-10s V%01x, V%01x, #$%01x", "SPRITE", code[0]&0xF, code[1]>>4, code[1]&0xF); break;
        case 0xE:
            switch (code[1]) {
                case 0x9E: printf("%-10s V%01x", "SKIP.KEY", code[0]&0xF); break;
                case 0xA1: printf("%-10s V%01x", "SKIP.NOKEY", code[0]&0xF); break;
                default: printf("Unknown E code 0xe%01x%02x", code[0]&0xF, code[1]);
            }
            break;
        case 0xF:
            switch (code[1]) {
                case 0x07: printf("%-10s V%01x, DELAY", "MOV", code[0]&0xF); break;
                case 0x0A: printf("%-10s V%01x", "WAITKEY", code[0]&0xF); break;
                case 0x15: printf("%-10s DELAY, V%01x", "MOV", code[0]&0xF); break;
                case 0x18: printf("%-10s SOUND, V%01x", "MOV", code[0]&0xF); break;
                case 0x1E: printf("%-10s I, V%01x", "ADD.", code[0]&0xF); break;
                case 0x29: printf("%-10s V%01x", "SPRITECHAR", code[0]&0xF); break;
                case 0x33: printf("%-10s V%01x", "MOVBCD", code[0]&0xF); break;
                case 0x55: printf("%-10s (I), V0-V%01x", "MOVM", code[0]&0xF); break;
                case 0x65: printf("%-10s V0-V%01x, (I)", "MOVM", code[0]&0xF); break;
                default: printf("Unknown F code 0xf%01x%02x", code[0]&0xF, code[1]);
            }
            break;
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
    uint8_t *buffer = (uint8_t *)malloc(rom_size + 0x200);
    fread(buffer + 0x200, rom_size, 1, fp);
    fclose(fp);

    uint16_t pc = 0x200;
    while (pc < rom_size+0x200) {
        disassemble(buffer, pc);
        pc += 2;
        printf("\n");
    }

    return 0;
}
