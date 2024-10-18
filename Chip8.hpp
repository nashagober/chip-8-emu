#include <cstdint>
#include <random>

const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int KEYS = 14;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int STACK_SIZE = 16;
const unsigned int REGISTER_AMT = 16;

class Chip8
{
public:
    Chip8();
    void LoadROM(char const *filename);

    uint8_t keypad[KEYS]{};
    uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};

private:
    void OP_00E0();
    void OP_00EE();
    void OP_1NNN();
    void OP_2NNN();
    void OP_3XNN();
    void OP_4XNN();
    void OP_5XY0();
    void OP_6XNN();
    void OP_7XNN();
    void OP_8XY0();
    void OP_8XY1();
    void OP_8XY2();
    void OP_8XY3();
    void OP_8XY4();
    void OP_8XY5();
    void OP_8XY6();
    void OP_8XY7();
    void OP_8XYE();
    void OP_9XY0();
    void OP_ANNN();
    void OP_BNNN();
    void OP_CXNN();
    void OP_DXYN();
    void OP_DXYN();
    void OP_EX9E();
    void OP_EXA1();
    void OP_FX07();
    void OP_FX0A();
    void OP_FX15();
    void OP_FX18();
    void OP_FX1E();
    void OP_FX29();
    void OP_FX33();
    void OP_FX55();
    void OP_FX65();

    uint8_t registers[REGISTER_AMT]{};
    uint8_t memory[MEMORY_SIZE]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[STACK_SIZE];
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint16_t opcode;

    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;
};