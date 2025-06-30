
#ifndef CHIP8_H
#define CHIP8_H

class chip8
{
    // These are public cause I want my main file to use this
    public:
        chip8();
        ~chip8();

        void emulateCycle();
        void loadRom(const char* filename);
        void init();

        // Ideally i can also use uint8_t to represent a byte
        unsigned char memory[4096];
        unsigned char graphics[64 * 32];
        bool drawFlag;
        unsigned char key[16];

        unsigned char sound_timer;
        unsigned char delay_timer;

    private:
        // Opcodes are 2 bytes, that's why they are used for the opcodes
        unsigned short opcode;
        // The registers
        unsigned char reg[16];

        unsigned short instruction_register;
        unsigned short program_counter;

        // 16 levels of a stack with 16 bits each aka 2 bytes
        unsigned short stack[16];
        unsigned short stack_pointer;
};

#endif // CHIP8_H

