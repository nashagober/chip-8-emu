#include <Chip8.hpp>
#include <fstream>
#include <chrono>
#include <random>

const unsigned int FREE_START_ADDRESS = 0x200;        // Beginning of the free address space
const unsigned int CHARACTER_SIZE = 80;               // 16 Hex Characters, 5 bytes each
const unsigned int CHARACTERSET_START_ADDRESS = 0x50; // Beginning of memory reserved for character set

uint8_t characterset[CHARACTER_SIZE] =
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

// Chip8 Constructor
Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    // Initialize Program Counter
    pc = FREE_START_ADDRESS;

    // Store character set in memory
    for (unsigned int i = 0; i < CHARACTER_SIZE; ++i)
    {
        memory[CHARACTERSET_START_ADDRESS + i] = characterset[i];
    }

    // initializing RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

// Load data from the ROM file
void Chip8::LoadROM(char const *filename)
{

    // open file as binary data (std::ios::binary) and move position to end of the file (std::ios::ate)
    std::ifstream romfile(filename, std::ios::binary | std::ios::ate);

    if (romfile.is_open())
    {
        // Get size of the file,
        // (tellg returns the position in the stream, which is the size since were at the end)
        std::streampos fsize = romfile.tellg();

        // allocate a buffer to hold ROM data
        char *buffer = new char[fsize];

        // Fill buffer by going to the start of the file
        romfile.seekg(0, std::ios::beg);
        romfile.read(buffer, fsize);
        romfile.close();

        // Move data from the buffer to the Chip8 memory, starting at 0x200,
        // which is the beginning of the free space in the chip's address space
        for (long i = 0; i < fsize; ++i)
        {
            memory[FREE_START_ADDRESS + i] = buffer[i];
        }

        // FREE ALLOCATED SPACE
        delete[] buffer;
    }
}

// BEGINNING OF INSTRUCTIONS

// CLS: Clear the display
void Chip8::OP_00E0()
{
    memset(video, 0, sizeof(video));
}

// RET: Return from a call
void Chip8::OP_00EE()
{
    --sp;
    pc = stack[sp];
}

// JP addr: Jump to location nnn
void Chip8::OP_1NNN()
{
    uint16_t addr = opcode & 0x0FFFu;
    pc = addr;
}

// CALL addr: Same as jump, but stores return location on stack
void Chip8::OP_2NNN()
{
    uint16_t addr = opcode & 0x0FFFu;
    stack[sp] = pc;
    sp++;
    pc = addr;
}

// SE Vx, byte: Skip next instruction if Vx == NN
void Chip8::OP_3XNN()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte)
    {
        pc += 2;
    }
}

// SNE Vx, byte: Skip next instruction if Vx != NN
void Chip8::OP_3XNN()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] != byte)
    {
        pc += 2;
    }
}

// SE Vx, Vy: Skip next instruction if Vx == Vy
void Chip8::OP_5XY0()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy])
    {
        pc += 2;
    }
}

// ADD Vx, byte: Add NN to the value of Vx
void Chip8::OP_7XNN()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] += byte;
}

// LD Vx, Vy: Load the value from Vy into Vx
void Chip8::OP_8XY0()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

// OR Vx, Vy: Load the value from (Vy OR Vx) into Vx
void Chip8::OP_8XY1()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
}

// AND Vx, Vy: Load the value from (Vy AND Vx) into Vx
void Chip8::OP_8XY2()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
}

// XOR Vx, Vy: Load the value from (Vy XOR Vx) into Vx
void Chip8::OP_8XY3()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
}

// ADD Vx, Vy: Load the value from (Vy + Vx) into Vx, VF = carry
void Chip8::OP_8XY4()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];

    if (sum > 255U)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu;
}

// SUB Vx, Vy: Load the value from (Vx - Vy) into Vx, VF = NOT borrow
void Chip8::OP_8XY5()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] > registers[Vy])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] -= registers[Vy];
}

// SHR Vx: Shift Vx right one, if LSB of Vx is 1 set VF to 1
void Chip8::OP_8XY6()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] >>= 1;
}

// SUBN Vx, Vy: Load the value from (Vy - Vx) into Vx, VF = NOT borrow
void Chip8::OP_8XY5()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vy] > registers[Vx])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

// SHL Vx: Shift Vx left one, if MSB of Vx is 1 set VF to 1
void Chip8::OP_8XYE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[Vx] & 0x1u) >> 7u;
    registers[Vx] <<= 1;
}

// SNE Vx, Vy: Skip next instruction if Vx != Vy
void Chip8::OP_9XY0()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy])
    {
        pc += 2;
    }
}

// LD I, addr: set index = NNN
void Chip8::OP_ANNN()
{
    uint16_t addr = opcode & 0x0FFFu;
    index = addr;
}

// JP V0, addr
void Chip8::OP_BNNN()
{
    uint16_t addr = opcode & 0x0FFFu;

    pc = registers[0] + addr;
}

// RND Vx, byte
void Chip8::OP_CXNN()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = randByte(randGen) & byte;
}

// DRW Vx, Vy, nibble: Set or Unset pixels on the screen
void Chip8::OP_DXYN()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    // Wrap around screen
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; +row)
    {
        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8; ++col)
        {
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t *screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            if (spritePixel)
            {
                // Detect a pixel collision
                if (*screenPixel == 0xFFFFFFFF)
                {
                    registers[0xF] = 1;
                }

                // XOR with sprite pixel to set or unset the value
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}