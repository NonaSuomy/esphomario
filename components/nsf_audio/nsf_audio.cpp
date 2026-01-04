#include "nsf_audio.h"
#include "sfx_data.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace nsf_audio {

static const char *TAG = "nsf_audio";

void NSFAudioComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up NSF Audio...");
  
  // Initialize triangle LUT
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 8; j++) {
      nes_triangle_lut[i * 8 + j] = nes_triangle_ref[i];
    }
  }
  triangle_.lut = nes_triangle_lut;
  
  // Initialize DMC delta map
  for (int i = 1; i < 256; i++) {
    dmc_delta_map[i] = dmc_delta_map[i & 3];
  }
  
  // Configure I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = this->bclk_pin_->get_pin(),
    .ws_io_num = this->lrclk_pin_->get_pin(),
    .data_out_num = this->dout_pin_->get_pin(),
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install I2S driver: %d", err);
    return;
  }
  
  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set I2S pins: %d", err);
    return;
  }
  
  i2s_zero_dma_buffer(I2S_NUM_0);
  
  // Create audio task on core 1
  xTaskCreatePinnedToCore(
    audio_task,
    "NSFAudio",
    4096,
    this,
    2,
    &this->audio_task_handle_,
    1
  );
  
  ESP_LOGCONFIG(TAG, "NSF Audio initialized");
}

void NSFAudioComponent::loop() {}

void NSFAudioComponent::update_sfx() {
  if (!sfx_playing_ || sfx_data_ == nullptr) return;
  
  if (sfx_pos_ - sfx_base_ >= sfx_len_) {
    sfx_playing_ = false;
    return;
  }
  
  uint8_t control = sfx_data_[sfx_pos_++];
  
  bool sqr1_f = (control & 1);
  bool sqr2_f = (control & 2);
  bool tri_f = (control & 4);
  bool sqr1_p = (control & 8);
  bool sqr2_p = (control & 16);
  bool noise_p = (control & 32);
  bool tri_active = (control & 64);
  
  if (sqr1_f) {
    sqr1_.phase_delta = (sfx_data_[sfx_pos_] * 256 + sfx_data_[sfx_pos_ + 1]) * PHASE_1HZ;
    sfx_pos_ += 2;
  }
  if (sqr2_f) {
    sqr2_.phase_delta = (sfx_data_[sfx_pos_] * 256 + sfx_data_[sfx_pos_ + 1]) * PHASE_1HZ;
    sfx_pos_ += 2;
  }
  if (tri_f) {
    triangle_.phase_delta = (sfx_data_[sfx_pos_] * 256 + sfx_data_[sfx_pos_ + 1]) * PHASE_1HZ;
    sfx_pos_ += 2;
  }
  if (sqr1_p) {
    uint8_t data = sfx_data_[sfx_pos_++];
    sqr1_.volume = data & 0x0F;
    sqr1_.duty = pulse_duty_table[data >> 4];
  }
  if (sqr2_p) {
    uint8_t data = sfx_data_[sfx_pos_++];
    sqr2_.volume = data & 0x0F;
    sqr2_.duty = pulse_duty_table[data >> 4];
  }
  if (noise_p) {
    uint8_t data = sfx_data_[sfx_pos_++];
    noise_.volume = data & 0x0F;
    noise_.bit_clock_delta = nes_lfsr_freq_ntsc[data >> 4] * PHASE_1HZ;
  }
  triangle_.set_gate(tri_active);
}

void NSFAudioComponent::update_music() {
  if (!music_playing_) return;
  
  const uint8_t *music_data = nullptr;
  
  // Select music track
  if (music_track_ == 1) {
    music_data = music_smb1_data;
  } else if (music_track_ == 2) {
    music_data = music_flagpole_data;
  } else if (music_track_ == 3) {
    music_data = music_death_data;
  }
  
  if (music_data == nullptr) return;
  
  // Use same NSF format as SFX
  if (music_pos_ - 6 >= (music_data[0] * 256 + music_data[1])) {
    if (music_loop_) {
      music_pos_ = 6; // Loop back to start of frames
    } else {
      music_playing_ = false; // Stop if not looping
    }
    return;
  }
  
  uint8_t control = music_data[music_pos_++];
  
  bool sqr1_f = (control & 1);
  bool sqr2_f = (control & 2);
  bool tri_f = (control & 4);
  bool sqr1_p = (control & 8);
  bool sqr2_p = (control & 16);
  bool noise_p = (control & 32);
  bool tri_active = (control & 64);
  
  if (sqr1_f) {
    sqr1_.phase_delta = (music_data[music_pos_] * 256 + music_data[music_pos_ + 1]) * PHASE_1HZ;
    music_pos_ += 2;
  }
  if (sqr2_f) {
    sqr2_.phase_delta = (music_data[music_pos_] * 256 + music_data[music_pos_ + 1]) * PHASE_1HZ;
    music_pos_ += 2;
  }
  if (tri_f) {
    triangle_.phase_delta = (music_data[music_pos_] * 256 + music_data[music_pos_ + 1]) * PHASE_1HZ;
    music_pos_ += 2;
  }
  if (sqr1_p) {
    uint8_t data = music_data[music_pos_++];
    sqr1_.volume = data & 0x0F;
    sqr1_.duty = pulse_duty_table[data >> 4];
  }
  if (sqr2_p) {
    uint8_t data = music_data[music_pos_++];
    sqr2_.volume = data & 0x0F;
    sqr2_.duty = pulse_duty_table[data >> 4];
  }
  if (noise_p) {
    uint8_t data = music_data[music_pos_++];
    noise_.volume = data & 0x0F;
    noise_.bit_clock_delta = nes_lfsr_freq_ntsc[data >> 4] * PHASE_1HZ;
  }
  triangle_.set_gate(tri_active);
}

void NSFAudioComponent::audio_task(void *param) {
  NSFAudioComponent *comp = (NSFAudioComponent *)param;
  int16_t buffer[128];
  size_t bytes_written;
  uint32_t sample_counter = 0;
  const uint32_t samples_per_frame = 267;  // 16000Hz / 60fps
  
  while (true) {
    // Generate 64 stereo samples
    for (int i = 0; i < 64; i++) {
      // Update both SFX and music every frame (60Hz)
      if (sample_counter == 0) {
        comp->update_music();
        comp->update_sfx();
      }
      sample_counter = (sample_counter + 1) % samples_per_frame;
      
      uint8_t s1 = comp->sqr1_.synthesize();
      uint8_t s2 = comp->sqr2_.synthesize();
      uint8_t tri = comp->triangle_.synthesize();
      uint8_t noise = comp->noise_.synthesize();
      
      uint8_t tnd = nes_dac_tnd_lut[tri * 3 + noise * 2];
      uint8_t pulse = nes_dac_pulse_lut[s1 + s2];
      
      int16_t mixed = (pulse + tnd) * 70 / 100;
      int16_t sample = (mixed << 7) - 16384;
      
      buffer[i * 2] = sample;
      buffer[i * 2 + 1] = sample;
    }
    
    i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
  }
}

void NSFAudioComponent::play_sfx(const char *type) {
  ESP_LOGI(TAG, "Playing SFX: %s", type);
  
  sfx_playing_ = false;
  
  if (strcmp(type, "jump") == 0) {
    sfx_data_ = sfx_jump_data;
    sfx_len_ = sfx_jump_data[0] * 256 + sfx_jump_data[1];
    sfx_base_ = 6;
    sfx_pos_ = sfx_base_;
    sfx_playing_ = true;
  } else if (strcmp(type, "coin") == 0) {
    sfx_data_ = sfx_coin_data;
    sfx_len_ = sfx_coin_data[0] * 256 + sfx_coin_data[1];
    sfx_base_ = 6;
    sfx_pos_ = sfx_base_;
    sfx_playing_ = true;
  } else if (strcmp(type, "powerup") == 0) {
    sfx_data_ = sfx_powerup_data;
    sfx_len_ = sfx_powerup_data[0] * 256 + sfx_powerup_data[1];
    sfx_base_ = 6;
    sfx_pos_ = sfx_base_;
    sfx_playing_ = true;
  } else if (strcmp(type, "brickbreak") == 0) {
    sfx_data_ = sfx_brickbreak_data;
    sfx_len_ = sfx_brickbreak_data[0] * 256 + sfx_brickbreak_data[1];
    sfx_base_ = 6;
    sfx_pos_ = sfx_base_;
    sfx_playing_ = true;
  } else if (strcmp(type, "fireball") == 0) {
    sfx_data_ = sfx_fireball_data;
    sfx_len_ = sfx_fireball_data[0] * 256 + sfx_fireball_data[1];
    sfx_base_ = 6;
    sfx_pos_ = sfx_base_;
    sfx_playing_ = true;
  } else if (strcmp(type, "jumpon") == 0) {
    sfx_data_ = sfx_jumpon_data;
    sfx_len_ = sfx_jumpon_data[0] * 256 + sfx_jumpon_data[1];
    sfx_base_ = 6;
    sfx_pos_ = sfx_base_;
    sfx_playing_ = true;
  } else if (strcmp(type, "flagpole") == 0) {
    sfx_data_ = sfx_flagpole_data;
    sfx_len_ = sfx_flagpole_data[0] * 256 + sfx_flagpole_data[1];
    sfx_base_ = 6;
    sfx_pos_ = sfx_base_;
    sfx_playing_ = true;
  }
}

void NSFAudioComponent::play_music(int track) {
  ESP_LOGI(TAG, "Playing music track: %d", track);
  music_track_ = track;
  music_pos_ = 6; // Skip header, start at APU frames
  music_loop_ = (track == 1); // Only loop main game music (track 1)
  music_playing_ = true;
}

void NSFAudioComponent::stop_music() {
  music_playing_ = false;
  if (!sfx_playing_) {
    sqr1_.volume = 0;
    sqr2_.volume = 0;
    triangle_.set_gate(false);
    noise_.volume = 0;
  }
}

}  // namespace nsf_audio
}  // namespace esphome
