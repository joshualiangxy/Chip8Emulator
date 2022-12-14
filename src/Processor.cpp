#include "Processor.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>

#include "Display.h"
#include "Keypad.h"

const uint8_t VIDEO_WIDTH = 64;
const uint8_t VIDEO_HEIGHT = 32;
const uint8_t BYTE_SIZE = 8;

const std::size_t Processor::FONT_SET_SIZE = 80;
const Processor::Font Processor::FONT_SET[Processor::FONT_SET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};
const Processor::Address Processor::FONT_SET_START_ADDRESS = 0x50;

Processor::Processor(const std::string& rom_path, Display& display,
                     const Keypad& keypad)
    : program_counter{0x200},
      index_register{0x0},
      delay_timer{0x0},
      sound_timer{0x0},
      memory{},
      display{display},
      keypad{keypad},
      random_engine{
          std::chrono::system_clock::now().time_since_epoch().count()},
      should_update_display{false} {
  this->initializeInstructionProcessors();
  this->uniform_int_distribution =
      std::uniform_int_distribution<short>{0, 255u};
  std::fill_n(this->registers, 16, 0x0);
  memcpy(memory + FONT_SET_START_ADDRESS, FONT_SET, FONT_SET_SIZE);

  std::ifstream rom{rom_path, std::ios::binary | std::ios::ate};
  if (rom.is_open()) {
    std::streampos size = rom.tellg();
    char* buffer = (char*)malloc(size * sizeof(char));

    // Go back to beginning of the file and fill the buffer
    rom.seekg(0, std::ios::beg);
    rom.read(buffer, size);
    rom.close();

    memcpy(memory + program_counter, buffer, size);
    free(buffer);
  }
}

void Processor::initializeInstructionProcessors() {
  std::fill_n(this->instruction_table, 0x10, &Processor::noop);

  this->instruction_table[0x0] = &Processor::processInstruction0;
  this->instruction_table[0x1] = &Processor::jump;
  this->instruction_table[0x2] = &Processor::call;
  this->instruction_table[0x3] = &Processor::constantComparisonSkip;
  this->instruction_table[0x4] = &Processor::constantComparisonSkip;
  this->instruction_table[0x5] = &Processor::registerComparisonSkip;
  this->instruction_table[0x6] = &Processor::setRegister;
  this->instruction_table[0x7] = &Processor::addToRegister;
  this->instruction_table[0x8] = &Processor::processArithmeticInstruction;
  this->instruction_table[0x9] = &Processor::registerComparisonSkip;
  this->instruction_table[0xA] = &Processor::setIndexRegister;
  this->instruction_table[0xB] = &Processor::jumpWithOffset;
  this->instruction_table[0xC] = &Processor::genRandomNumber;
  this->instruction_table[0xD] = &Processor::draw;
  this->instruction_table[0xE] = &Processor::skipIfKey;
  this->instruction_table[0xF] = &Processor::processInstructionF;

  std::fill_n(this->arithmetic_instruction_table, 0x10,
              &Processor::noopArithmetic);

  this->arithmetic_instruction_table[0x0] = &Processor::set;
  this->arithmetic_instruction_table[0x1] = &Processor::logicalOr;
  this->arithmetic_instruction_table[0x2] = &Processor::logicalAnd;
  this->arithmetic_instruction_table[0x3] = &Processor::logicalXor;
  this->arithmetic_instruction_table[0x4] = &Processor::add;
  this->arithmetic_instruction_table[0x5] = &Processor::subtractYFromX;
  this->arithmetic_instruction_table[0x6] = &Processor::shiftRight;
  this->arithmetic_instruction_table[0x7] = &Processor::subtractXFromY;
  this->arithmetic_instruction_table[0xE] = &Processor::shiftLeft;
}

void Processor::process() {
  Instruction instruction = this->getInstruction();
  uint16_t first_nibble = instruction >> 12;
  this->should_update_display = false;

  (this->*instruction_table[first_nibble])(instruction);

  if (this->delay_timer > 0) this->delay_timer--;
  if (this->sound_timer > 0) this->sound_timer--;
}

Processor::Instruction Processor::getInstruction() {
  MemoryValue first_half = this->memory[this->program_counter++];
  MemoryValue second_half = this->memory[this->program_counter++];
  Instruction instruction = (first_half << 8) | second_half;

  return instruction;
}

void Processor::noop(const Instruction& instruction) {}

void Processor::processInstruction0(const Instruction& instruction) {
  uint16_t second_nibble = (instruction & 0xF00) >> 8;
  uint16_t third_nibble = (instruction & 0xF0) >> 4;
  uint16_t fourth_nibble = (instruction & 0xF);

  if (second_nibble != 0x0 || third_nibble != 0xE) {
    return;
  }

  switch (fourth_nibble) {
    case 0x0:
      this->display.clear();
      this->should_update_display = true;
      break;

    case 0xE:
      if (this->stack.empty()) {
        throw std::logic_error(
            "Stack should not be empty when 0x00EE is called");
      }

      this->program_counter = this->stack.top();
      this->stack.pop();
      break;
  }
}

void Processor::jump(const Instruction& instruction) {
  Address new_address = (instruction & 0xFFF);
  this->program_counter = new_address;
}

void Processor::call(const Instruction& instruction) {
  Address new_address = (instruction & 0xFFF);
  this->stack.push(this->program_counter);
  this->program_counter = new_address;
}

void Processor::constantComparisonSkip(const Instruction& instruction) {
  uint16_t register_to_check = (instruction & 0xF00) >> 8;
  RegisterValue value_to_check = instruction & 0xFF;

  uint16_t instruction_type = instruction >> 12;
  switch (instruction_type) {
    case 0x3:
      if (this->registers[register_to_check] == value_to_check) {
        this->program_counter += 2;
      }
      break;

    case 0x4:
      if (this->registers[register_to_check] != value_to_check) {
        this->program_counter += 2;
      }
      break;

    default:
      throw std::logic_error(std::format(
          "First nibble of compare constant skip instruction should not be {}",
          instruction_type));
  }
}

void Processor::registerComparisonSkip(const Instruction& instruction) {
  if ((instruction & 0xF) != 0) {
    throw std::logic_error(
        "Compare register skip instruction should have last nibble as 0x0");
  }

  uint16_t register_x = (instruction & 0xF00) >> 8;
  uint16_t register_y = (instruction & 0xF0) >> 4;

  uint16_t instruction_type = instruction >> 12;
  switch (instruction_type) {
    case 0x5:
      if (this->registers[register_x] == this->registers[register_y]) {
        this->program_counter += 2;
      }
      break;

    case 0x9:
      if (this->registers[register_x] != this->registers[register_y]) {
        this->program_counter += 2;
      }
      break;

    default:
      throw std::logic_error(std::format(
          "First nibble of compare register skip instruction should not be {}",
          instruction_type));
  }
}

void Processor::setRegister(const Instruction& instruction) {
  uint16_t register_to_set = (instruction & 0xF00) >> 8;
  RegisterValue value_to_set = instruction & 0xFF;
  this->registers[register_to_set] = value_to_set;
}

void Processor::addToRegister(const Instruction& instruction) {
  uint16_t register_to_add = (instruction & 0xF00) >> 8;
  RegisterValue value_to_add = instruction & 0xFF;
  this->registers[register_to_add] += value_to_add;
}

void Processor::processArithmeticInstruction(const Instruction& instruction) {
  uint16_t register_x = (instruction & 0xF00) >> 8;
  uint16_t register_y = (instruction & 0xF0) >> 4;
  uint16_t instruction_type = instruction & 0xF;

  (this->*arithmetic_instruction_table[instruction_type])(register_x,
                                                          register_y);
}

void Processor::setIndexRegister(const Instruction& instruction) {
  uint16_t value_to_set = instruction & 0xFFF;
  this->index_register = value_to_set;
}

void Processor::jumpWithOffset(const Instruction& instruction) {
  Address new_address = (instruction & 0xFFF);
  this->program_counter = new_address;

#ifdef ORIGINAL_CHIP8
  this->program_counter += this->registers[0x0];
#else
  uint16_t register_x = (instruction & 0xF00) >> 8;
  this->program_counter += this->registers[register_x];
#endif
}

void Processor::genRandomNumber(const Instruction& instruction) {
  uint16_t register_to_set = (instruction & 0xF00) >> 8;
  uint16_t value = instruction & 0xFF;
  this->registers[register_to_set] =
      this->uniform_int_distribution(this->random_engine) & value;
}

void Processor::draw(const Instruction& instruction) {
  uint16_t x_register = (instruction & 0xF00) >> 8;
  uint16_t y_register = (instruction & 0xF0) >> 4;
  uint16_t height = instruction & 0xF;
  RegisterValue x_pos = this->registers[x_register] % VIDEO_WIDTH;
  RegisterValue y_pos = this->registers[y_register] % VIDEO_HEIGHT;

  this->registers[Processor::FLAG_REGISTER] = 0;

  for (uint16_t row = 0; row < height; row++) {
    MemoryValue sprite_byte = this->memory[this->index_register + row];

    for (uint16_t col = 0; col < BYTE_SIZE; col++) {
      MemoryValue sprite_pixel = sprite_byte & (0x80 >> col);

      if (!sprite_pixel) {
        continue;
      }

      size_t screen_pixel_index = (y_pos + row) * VIDEO_WIDTH + (x_pos + col);
      if (screen_pixel_index >= this->display.getSize()) {
        continue;
      }

      if (this->display.flipPixel(screen_pixel_index))
        this->registers[Processor::FLAG_REGISTER] = 1;
    }
  }

  this->should_update_display = true;
}

void Processor::skipIfKey(const Instruction& instruction) {
  uint16_t register_to_be_checked = (instruction & 0xF00) >> 8;
  RegisterValue key_to_be_checked = this->registers[register_to_be_checked];

  bool is_valid_key = key_to_be_checked >= 0x0 && key_to_be_checked <= 0xF;
  if (!is_valid_key) {
    throw std::logic_error(
        std::format("Skip if key instruction encountered invalid key: {}",
                    key_to_be_checked));
  }

  bool is_key_pressed = this->keypad.isKeyPressed(key_to_be_checked);

  uint16_t instruction_type = instruction & 0xFF;
  switch (instruction_type) {
    case 0x9E:
      if (is_key_pressed) {
        this->program_counter += 2;
      }
      break;

    case 0xA1:
      if (!is_key_pressed) {
        this->program_counter += 2;
      }
      break;

    default:
      throw std::logic_error(
          std::format("Skip if key instruction should not have last byte as {}",
                      instruction_type));
  }
}

void Processor::processInstructionF(const Instruction& instruction) {
  uint16_t sum;
  int key;

  uint16_t register_x = (instruction & 0xF00) >> 8;
  RegisterValue value = this->registers[register_x];

  uint16_t instruction_type = instruction & 0xFF;
  switch (instruction_type) {
    // Timer instructions
    case 0x07:
      this->registers[register_x] = this->delay_timer;
      break;
    case 0x15:
      this->delay_timer = this->registers[register_x];
      break;
    case 0x18:
      this->sound_timer = this->registers[register_x];
      break;

    // Add to index
    case 0x1E:
      sum = this->index_register + this->registers[register_x];
      if (sum < this->index_register || sum < this->registers[register_x]) {
        this->registers[Processor::FLAG_REGISTER] = 1;
      }

      this->index_register = sum;
      break;

    // Get key
    case 0x0A:
      key = this->keypad.getKey();
      if (key == -1) {
        this->program_counter -= 2;
        break;
      }

      this->registers[register_x] = key;
      break;

    // Font instruction
    case 0x29:
      this->index_register = Processor::FONT_SET_START_ADDRESS + (5 * value);
      break;

    // Binary-coded decimal conversion
    case 0x33:
      for (int i = 2; i >= 0; i--) {
        this->memory[this->index_register + i] = value % 10;
        value /= 10;
      }
      break;

    // Store memory
    case 0x55:
      for (int i = 0; i <= register_x; i++) {
        this->memory[this->index_register + i] = this->registers[i];
      }
      break;

    // Load memory
    case 0x65:
      for (int i = 0; i <= register_x; i++) {
        this->registers[i] = this->memory[this->index_register + i];
      }
      break;

    default:
      throw std::logic_error(std::format(
          "Instruction F should not have last byte {}", instruction_type));
  }
}

void Processor::noopArithmetic(const uint16_t register_x,
                               const uint16_t register_y) {}

void Processor::set(const uint16_t register_x, const uint16_t register_y) {
  this->registers[register_x] = this->registers[register_y];
}

void Processor::logicalOr(const uint16_t register_x,
                          const uint16_t register_y) {
  this->registers[register_x] =
      this->registers[register_x] | this->registers[register_y];
}

void Processor::logicalAnd(const uint16_t register_x,
                           const uint16_t register_y) {
  this->registers[register_x] =
      this->registers[register_x] & this->registers[register_y];
}

void Processor::logicalXor(const uint16_t register_x,
                           const uint16_t register_y) {
  this->registers[register_x] =
      this->registers[register_x] ^ this->registers[register_y];
}

void Processor::add(const uint16_t register_x, const uint16_t register_y) {
  RegisterValue sum = this->registers[register_x] + this->registers[register_y];
  this->registers[Processor::FLAG_REGISTER] =
      sum < this->registers[register_x] || sum < this->registers[register_y];
  this->registers[register_x] = sum;
}

void Processor::subtractYFromX(const uint16_t register_x,
                               const uint16_t register_y) {
  RegisterValue result =
      this->registers[register_x] - this->registers[register_y];
  this->registers[Processor::FLAG_REGISTER] =
      this->registers[register_x] > this->registers[register_y];
  this->registers[register_x] = result;
}

void Processor::subtractXFromY(const uint16_t register_x,
                               const uint16_t register_y) {
  RegisterValue result =
      this->registers[register_y] - this->registers[register_x];
  this->registers[Processor::FLAG_REGISTER] =
      this->registers[register_y] > this->registers[register_x];
  this->registers[register_x] = result;
}

void Processor::shiftRight(const uint16_t register_x,
                           const uint16_t register_y) {
#ifdef ORIGINAL_CHIP8
  this->registers[register_x] = this->registers[register_y];
#endif

  this->registers[Processor::FLAG_REGISTER] = this->registers[register_x] & 0x1;
  this->registers[register_x] = this->registers[register_x] >> 1;
}

void Processor::shiftLeft(const uint16_t register_x,
                          const uint16_t register_y) {
#ifdef ORIGINAL_CHIP8
  this->registers[register_x] = this->registers[register_y];
#endif

  this->registers[Processor::FLAG_REGISTER] = this->registers[register_x] >> 7;
  this->registers[register_x] = this->registers[register_x] << 1;
}

bool Processor::shouldUpdateDisplay() { return this->should_update_display; }
