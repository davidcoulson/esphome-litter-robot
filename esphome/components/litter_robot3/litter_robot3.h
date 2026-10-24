#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome::litter_robot3 {

static const uint8_t LINE_BUFFER_SIZE = 32;

enum LitterRobot3Field : uint8_t {
  LR3_FIELD_POWER_STATE,
  LR3_FIELD_SYSTEM_STATE,
  LR3_FIELD_NIGHT_LIGHT,
  LR3_FIELD_CLEAN_CYCLE_COUNT,
  LR3_FIELD_POWER_TYPE,
  LR3_FIELD_CYCLE_DELAY,
  LR3_FIELD_PANEL_LOCKOUT,
  LR3_FIELD_SLEEP_ENABLED,
  LR3_FIELD_CAT_SENSOR,
};

const char *command_name(char command);
const char *command_value_name(char command, const char *value);
const char *power_type_name(uint8_t type);
const char *power_state_name(uint8_t state);
const char *cycle_delay_name(uint8_t delay);
const char *system_state_name(uint8_t state);

class LitterRobot3Component final : public uart::UARTDevice, public Component {
 public:
  void loop() override;
  void dump_config() override;

  void send_command(char command, const char *value = nullptr);
  void set_cycle_delay(uint8_t minutes);
  void set_sleep(uint32_t seconds);
  void disable_sleep();

  // Parsed fields are broadcast as (field, value); consumers own their own state.
  template<typename F> void add_status_callback(F &&callback) { this->status_callback_.add(std::forward<F>(callback)); }

 protected:
  struct Status {
    uint8_t power_type{0};
    uint8_t power_state{0};
    uint8_t system_state{0};
    uint8_t cycle_delay{0};
    bool night_light{false};
    bool sleep_enabled{false};
    bool panel_lockout{false};
    int16_t cat_sensor{-1};
  };

  void handle_line_(char *line);
  void handle_response_(const char *response);
  void handle_status_(char *line);
  void handle_clean_cycle_count_(const char *value);

  char rx_line_[LINE_BUFFER_SIZE];
  size_t rx_len_{0};
  Status last_status_{};
  bool has_status_{false};

  LazyCallbackManager<void(LitterRobot3Field, uint32_t)> status_callback_;
};

}  // namespace esphome::litter_robot3
