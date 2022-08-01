#ifndef GUARD_PROCESSOR_H
#define GUARD_PROCESSOR_H

#include <cstdint>
#include <stack>
#include <string>

#include "Display.h"

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

  Processor(const std::string& rom_path, Display& display);

  void process();

 private:
  static const std::size_t FONT_SET_SIZE;
  static const Font FONT_SET[];
  static const Address FONT_SET_START_ADDRESS;

  Stack stack;
  Address program_counter;
  MemoryValue memory[4096];
  RegisterValue registers[16];
  IndexRegisterValue index_register;
  Timer delay_timer;
  Timer sound_timer;
  Display& display;

  Instruction getInstruction();

  void initializeInstructionProcessors();
  void noop(const Instruction& instruction);
  void processInstruction0(const Instruction& instruction);
  void jump(const Instruction& instruction);
  void call(const Instruction& instruction);
  void constantComparisonSkip(const Instruction& instruction);
  void registerComparisonSkip(const Instruction& instruction);
  void setRegister(const Instruction& instruction);
  void addToRegister(const Instruction& instruction);
  void setIndexRegister(const Instruction& instruction);
  void draw(const Instruction& instruction);

  typedef void (Processor::*InstructionProcessor)(
      const Instruction& instruction);

  InstructionProcessor instruction_table[0x10];
};

#endif
