#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

unsigned char chip8_fontset[80] =
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

chip8::chip8()
{
}

chip8::~chip8()
{
}

void chip8::init()
{
    program_counter = 0x200;
    instruction_register = 0;
    opcode = 0;
    stack_pointer = 0;

    // clear graphics
    for (int i = 0; i < 64 * 32; ++i)
    {

        graphics[i] = 0;
    }

    for (int i = 0; i < 16; ++i)
    {
        reg[i] = 0;
    }

    for (int i = 0; i < 16; ++i)
    {
        key[i] = 0;
    }

    for (int i = 0; i < 16; ++i)
    {
        stack[i] = 0;
    }

    for (int i = 0; i < sizeof(chip8_fontset); ++i)
    {
        memory[0x50 + i] = chip8_fontset[i];
    }

    delay_timer = 0;
    sound_timer = 0;

    drawFlag = true;
}

void chip8::emulateCycle()
{
    opcode = memory[program_counter] << 8 | memory[program_counter + 1];

    switch (opcode & 0xF000)
    {
    case 0x0000:
    {
        switch (opcode & 0x000F)
        {
        case 0x0000:
        {
            const int SCREEN_WIDTH = 64;
            const int SCREEN_HEIGHT = 32;
            for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
            {
                graphics[i] = 0;
            }
            drawFlag = true;
            program_counter += 2;
            break;
        }

        case 0x000E:
        {
            --stack_pointer;
            program_counter = stack[stack_pointer];
            break;
        }

        default:
            // printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
            break;
        }
        break;
    }

    case 0x1000:
    {
        program_counter = opcode & 0x0FFF;
        break;
    }

    case 0x2000:
    {
        stack[stack_pointer] = program_counter + 2;
        ++stack_pointer;
        program_counter = opcode & 0x0FFF;
        break;
    }

    case 0x3000:
    {
        if (reg[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
        {
            program_counter += 4;
        }
        else
        {
            program_counter += 2;
        }
        break;
    }

    case 0x4000:
    {
        if (reg[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
        {
            program_counter += 4;
        }
        else
        {
            program_counter += 2;
        }
        break;
    }

    case 0x5000:
    {
        if (reg[(opcode & 0x0F00) >> 8] == reg[(opcode & 0x00F0) >> 4])
        {
            program_counter += 4;
        }
        else
        {
            program_counter += 2;
        }
        break;
    }

    case 0x6000:
    {
        reg[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        program_counter += 2;
        break;
    }

    case 0x7000:
    {
        reg[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        program_counter += 2;
        break;
    }

    case 0x8000:
    {
        switch (opcode & 0x000F)
        {
        case 0x0000:
        {
            reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4];
            program_counter += 2;
            break;
        }

        case 0x0001:
        {
            reg[(opcode & 0x0F00) >> 8] |= reg[(opcode & 0x00F0) >> 4];
            program_counter += 2;
            break;
        }

        case 0x0002:
        {
            reg[(opcode & 0x0F00) >> 8] &= reg[(opcode & 0x00F0) >> 4];
            program_counter += 2;
            break;
        }

        case 0x0003:
        {
            reg[(opcode & 0x0F00) >> 8] ^= reg[(opcode & 0x00F0) >> 4];
            program_counter += 2;
            break;
        }

        case 0x0004:
        {
            unsigned short sum = reg[(opcode & 0x0F00) >> 8] + reg[(opcode & 0x00F0) >> 4];
            reg[0xF] = sum > 0xFF ? 1 : 0;
            reg[(opcode & 0x0F00) >> 8] = sum & 0xFF;
            program_counter += 2;
            break;
        }

        case 0x0005:
        {
            reg[0xF] = reg[(opcode & 0x0F00) >> 8] >= reg[(opcode & 0x00F0) >> 4] ? 1 : 0;
            reg[(opcode & 0x0F00) >> 8] -= reg[(opcode & 0x00F0) >> 4];
            program_counter += 2;
            break;
        }

        case 0x0006:
        {
            reg[0xF] = reg[(opcode & 0x0F00) >> 8] & 0x1;
            reg[(opcode & 0x0F00) >> 8] >>= 1;
            program_counter += 2;
            break;
        }

        case 0x0007:
        {
            reg[0xF] = reg[(opcode & 0x00F0) >> 4] >= reg[(opcode & 0x0F00) >> 8] ? 1 : 0;
            reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4] - reg[(opcode & 0x0F00) >> 8];
            program_counter += 2;
            break;
        }

        case 0x000E:
        {
            reg[0xF] = (reg[(opcode & 0x0F00) >> 8] & 0x80) >> 7;
            reg[(opcode & 0x0F00) >> 8] <<= 1;
            program_counter += 2;
            break;
        }

        default:
        {
            // printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
            break;
        }
        }
        break;
    }

    case 0x9000:
    {
        if (reg[(opcode & 0x0F00) >> 8] != reg[(opcode & 0x00F0) >> 4])
        {
            program_counter += 4;
        }
        else
        {
            program_counter += 2;
        }
        break;
    }

    case 0xA000:
    {
        instruction_register = opcode & 0x0FFF;
        program_counter += 2;
        break;
    }

    case 0xB000:
    {
        program_counter = (opcode & 0x0FFF) + reg[0];
        break;
    }

    case 0xC000:
    {
        reg[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
        program_counter += 2;
        break;
    }

    case 0xD000:
    {
        const int SCREEN_WIDTH = 64;
        const int SCREEN_HEIGHT = 32;
        unsigned short x = reg[(opcode & 0x0F00) >> 8];
        unsigned short y = reg[(opcode & 0x00F0) >> 4];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        reg[0xF] = 0;

        for (int yline = 0; yline < height; yline++)
        {
            pixel = memory[instruction_register + yline];

            for (int xline = 0; xline < 8; xline++)
            {
                if ((pixel & (0x80 >> xline)) != 0)
                {
                    int xpos = (x + xline) % SCREEN_WIDTH;
                    int ypos = (y + yline) % SCREEN_HEIGHT;
                    int index = xpos + (ypos * SCREEN_WIDTH);

                    if (graphics[index] == 1)
                    {
                        reg[0xF] = 1; // collision flag
                    }

                    graphics[index] ^= 1;
                }
            }
        }

        drawFlag = true;
        program_counter += 2;
        break;
    }

    case 0xE000:
    {
        switch (opcode & 0x00FF)
        {
        case 0x009E:
        {
            if (key[reg[(opcode & 0x0F00) >> 8]] != 0)
            {
                program_counter += 4;
            }
            else
            {
                program_counter += 2;
            }
            break;
        }

        case 0x00A1:
        {
            if (key[reg[(opcode & 0x0F00) >> 8]] == 0)
            {
                program_counter += 4;
            }
            else
            {
                program_counter += 2;
            }
            break;
        }

        default:
            // printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
            break;
        }
        break;
    }

    case 0xF000:
    {
        switch (opcode & 0x00FF)
        {
        case 0x0007:
        {
            reg[(opcode & 0x0F00) >> 8] = delay_timer;
            program_counter += 2;
            break;
        }

        case 0x000A:
        {
            bool keyPressed = false;
            for (int i = 0; i < 16; ++i)
            {
                if (key[i] != 0)
                {
                    reg[(opcode & 0x0F00) >> 8] = i;
                    keyPressed = true;
                    break;
                }
            }
            if (!keyPressed)
                return;

            program_counter += 2;
            break;
        }

        case 0x0015:
        {
            delay_timer = reg[(opcode & 0x0F00) >> 8];
            program_counter += 2;
            break;
        }
        case 0x0018:
        {
            sound_timer = reg[(opcode & 0x0F00) >> 8];
            program_counter += 2;
            break;
        }

        case 0x001E:
        {
            instruction_register += reg[(opcode & 0x0F00) >> 8];
            program_counter += 2;
            break;
        }

        case 0x0029:
        {
            instruction_register = 0x50 + (reg[(opcode & 0x0F00) >> 8] * 0x5);
            program_counter += 2;
            break;
        }

        case 0x0033:
        {
            unsigned char value = reg[(opcode & 0x0F00) >> 8];
            memory[instruction_register] = value / 100;
            memory[instruction_register + 1] = (value / 10) % 10;
            memory[instruction_register + 2] = value % 10;
            program_counter += 2;
            break;
        }

        case 0x0055:
        {
            unsigned char x = (opcode & 0x0F00) >> 8;
            for (int i = 0; i <= x; i++)
            {
                memory[instruction_register + i] = reg[i];
            }
            instruction_register += x + 1;
            program_counter += 2;
            break;
        }

        case 0x0065:
        {
            unsigned char x = (opcode & 0x0F00) >> 8;
            for (int i = 0; i <= x; i++)
            {
                reg[i] = memory[instruction_register + i];
            }
            instruction_register += x + 1;
            program_counter += 2;
            break;
        }

        default:
        {
            // printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
            break;
        }
        }
        break;
    }

    default:
        printf("Unknown opcode: 0x%04X at PC: 0x%04X\n", opcode, program_counter);
        break;
    }

    if (delay_timer > 0)
    {
        --delay_timer;
    }

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
        {
            printf("BEEP!\n");
        }
        --sound_timer;
    }
}

void chip8::loadRom(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open())
    {
        // Gotten a buffer
        std::streampos size = file.tellg();
        char *buffer = new char[size];

        // Want to load the contents of the file into the buffer

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i = 0; i < size; i++)
        {
            memory[0x200 + i] = buffer[i];
        }

        // clear buffer
        delete[] buffer;
    }
}
