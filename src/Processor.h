#ifndef GUARD_PROCESSOR_H
#define GUARD_PROCESSOR_H

#include <cstdint>
#include <random>
#include <stack>
#include <string>

#include "Display.h"
#include "Keypad.h"

class Processor {
 public:
  typedef uint16_t Address;
  typedef uint8_t Font;
  typedef uint16_t IndexRegisterValue;
  typedef uint16_t Instruction;
  typedef uint8_t MemoryValue;
  typedef uint8_t RegisterValue;
  typedef std::stack<Address> Stack;
  typedef uint8_t Timer;

  Processor(const std::string& rom_path, Display& display,
            const Keypad& keypad);

  void process();
  bool shouldUpdateDisplay();

 private:
  static const std::size_t FONT_SET_SIZE;
  static const Font FONT_SET[];
  static const Address FONT_SET_START_ADDRESS;
  static const uint16_t FLAG_REGISTER = 0xF;

  Stack stack;
  Address program_counter;
  MemoryValue memory[4096];
  RegisterValue registers[16];
  IndexRegisterValue index_register;
  Timer delay_timer;
  Timer sound_timer;
  Display& display;
  const Keypad& keypad;
  std::default_random_engine random_engine;
  std::uniform_int_distribution<short> uniform_int_distribution;
  bool should_update_display;

  Instruction getInstruction();

  void initializeInstructionProcessors();

  // Regular instructions
  void noop(const Instruction& instruction);
  void processInstruction0(const Instruction& instruction);
  void jump(const Instruction& instruction);
  void call(const Instruction& instruction);
  void constantComparisonSkip(const Instruction& instruction);
  void registerComparisonSkip(const Instruction& instruction);
  void setRegister(const Instruction& instruction);
  void addToRegister(const Instruction& instruction);
  void processArithmeticInstruction(const Instruction& instruction);
  void setIndexRegister(const Instruction& instruction);
  void jumpWithOffset(const Instruction& instruction);
  void genRandomNumber(const Instruction& instruction);
  void draw(const Instruction& instruction);
  void skipIfKey(const Instruction& instruction);
  void processInstructionF(const Instruction& instruction);

  // Arithmetic instructions
  void noopArithmetic(const uint16_t register_x, const uint16_t register_y);
  void set(const uint16_t register_x, const uint16_t register_y);
  void logicalOr(const uint16_t register_x, const uint16_t register_y);
  void logicalAnd(const uint16_t register_x, const uint16_t register_y);
  void logicalXor(const uint16_t register_x, const uint16_t register_y);
  void add(const uint16_t register_x, const uint16_t register_y);
  void subtractYFromX(const uint16_t register_x, const uint16_t register_y);
  void subtractXFromY(const uint16_t register_x, const uint16_t register_y);
  void shiftRight(const uint16_t register_x, const uint16_t register_y);
  void shiftLeft(const uint16_t register_x, const uint16_t register_y);

  typedef void (Processor::*InstructionProcessor)(
      const Instruction& instruction);
  typedef void (Processor::*ArithmeticInstructionProcessor)(
      const uint16_t register_x, const uint16_t register_y);

  InstructionProcessor instruction_table[0x10];
  ArithmeticInstructionProcessor arithmetic_instruction_table[0x10];
};

#endif
