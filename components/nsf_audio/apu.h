#pragma once

#include <cstdint>
#include <cmath>

namespace esphome {
namespace nsf_audio {

#define PHASE_1HZ 4.096f

// NES APU LUTs
static uint8_t pulse_duty_table[4] = {32, 64, 128, 192};
static uint8_t nes_triangle_lut[256];
static uint8_t nes_triangle_ref[32] = {
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static uint32_t nes_lfsr_freq_ntsc[] = {
  447443, 223722, 111861, 55930, 27965, 18643, 13983, 11186, 8860, 7046, 4710, 3523, 2349, 1762, 880, 440
};

static int8_t dmc_delta_map[256] = {-2, 2, -2, 2};

static uint8_t nes_dac_pulse_lut[] = {
  0, 3, 6, 9, 11, 14, 17, 19, 22, 24, 27, 29, 31, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 57, 59, 61, 62, 64, 66
};

static uint8_t nes_dac_tnd_lut[] = {
  0, 2, 3, 5, 7, 8, 10, 12, 13, 15, 16, 18, 20, 21, 23, 24, 26, 27, 29, 30, 32, 33, 35, 36, 37, 39, 40, 42, 43, 44, 46, 47,
  49, 50, 51, 52, 54, 55, 56, 58, 59, 60, 61, 63, 64, 65, 66, 68, 69, 70, 71, 72, 73, 75, 76, 77, 78, 79, 80, 81, 83, 84, 85,
  86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
  114, 115, 115, 116, 117, 118, 119, 120, 121, 122, 122, 123, 124, 125, 126, 127, 127, 128, 129, 130, 131, 132, 132, 133, 134, 135,
  136, 136, 137, 138, 139, 139, 140, 141, 142, 142, 143, 144, 145, 145, 146, 147, 148, 148, 149, 150, 150, 151, 152, 152, 153, 154,
  155, 155, 156, 157, 157, 158, 159, 159, 160, 160, 161, 162, 162, 163, 164, 164, 165, 166, 166, 167, 167, 168, 169, 169, 170, 170,
  171, 172, 172, 173, 173, 174, 175, 175, 176, 176, 177, 177, 178, 179, 179, 180, 180, 181, 181, 182, 182, 183, 184, 184, 185, 185,
  186, 186, 187, 187, 188, 188, 189, 189
};

class PulseWave {
 public:
  union {
    uint16_t accumulator;
    struct {
      uint8_t accu_low;
      uint8_t value;
    };
  } phase = {0};
  uint16_t phase_delta = 0;
  uint8_t duty = 128;
  uint8_t volume = 0;

  inline uint8_t synthesize() {
    phase.accumulator += phase_delta;
    return (phase.value < duty) * volume;
  }
};

class GatedLUTWave {
 public:
  uint8_t *lut;
  union {
    uint16_t accumulator;
    struct {
      uint8_t accu_low;
      uint8_t value;
    };
  } phase = {0};
  uint16_t phase_delta = 0;
  bool gate = false;
  uint8_t sample = 0;

  void set_gate(bool set_gate) {
    if (set_gate && !gate) {
      phase.accumulator = 32768;
    }
    gate = set_gate;
  }

  inline uint8_t synthesize() {
    if (gate) {
      sample = lut[phase.value];
      phase.accumulator += phase_delta;
    } else {
      if (sample > 0) sample--;
    }
    return sample;
  }
};

class LFSR {
 public:
  union {
    uint32_t accumulator;
    struct {
      uint16_t accu_low;
      uint16_t value;
    };
  } bit_clock = {0};
  uint16_t cmp_bit_clock = 0;
  uint32_t bit_clock_delta = 0;
  bool short_mode = false;
  uint16_t seed = 1;
  uint8_t feedback;
  bool output;
  uint8_t volume = 0;

  inline uint8_t synthesize() {
    bit_clock.accumulator += bit_clock_delta;
    if (bit_clock.value != cmp_bit_clock) {
      output = seed & 1;
      if (short_mode) {
        feedback = output ^ ((seed & 64) >> 6);
      } else {
        feedback = output ^ ((seed & 2) >> 1);
      }
      seed >>= 1;
      seed |= (feedback << 14);
      cmp_bit_clock = bit_clock.value;
    }
    return output * volume;
  }
};

}  // namespace nsf_audio
}  // namespace esphome
