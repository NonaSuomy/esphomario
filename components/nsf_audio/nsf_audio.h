#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "apu.h"
#include <driver/i2s.h>

namespace esphome {
namespace nsf_audio {

class NSFAudioComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::LATE; }
  
  void set_bclk_pin(InternalGPIOPin *pin) { bclk_pin_ = pin; }
  void set_lrclk_pin(InternalGPIOPin *pin) { lrclk_pin_ = pin; }
  void set_dout_pin(InternalGPIOPin *pin) { dout_pin_ = pin; }
  
  void play_sfx(const char *type);
  void play_music(int track);
  void stop_music();
  void update_sfx();
  void update_music();
  
  // APU channels
  PulseWave sqr1_;
  PulseWave sqr2_;
  GatedLUTWave triangle_;
  LFSR noise_;
  
 protected:
  InternalGPIOPin *bclk_pin_;
  InternalGPIOPin *lrclk_pin_;
  InternalGPIOPin *dout_pin_;
  
  static void audio_task(void *param);
  TaskHandle_t audio_task_handle_{nullptr};
  
  // NSF playback state
  volatile bool sfx_playing_{false};
  volatile const uint8_t *sfx_data_{nullptr};
  volatile uint16_t sfx_len_{0};
  volatile uint16_t sfx_base_{0};
  volatile uint16_t sfx_pos_{0};
  
  // Music playback state
  volatile bool music_playing_{false};
  volatile int music_track_{0};
  volatile uint16_t music_pos_{0};
  volatile bool music_loop_{true};
};

}  // namespace nsf_audio
}  // namespace esphome
