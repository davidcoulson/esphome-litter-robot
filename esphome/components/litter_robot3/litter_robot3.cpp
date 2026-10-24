#include "litter_robot3.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3";

static void log_status_field(const char *label, const char *name, uint8_t value) {
  if (name != nullptr) {
    ESP_LOGD(TAG, "%s: %s", label, name);
  } else {
    ESP_LOGD(TAG, "%s: Unknown (0x%02X)", label, static_cast<unsigned>(value));
  }
}

static size_t split_tokens(char *line, char *tokens[], size_t max_tokens) {
  size_t count = 0;
  char *p = line;
  while (count < max_tokens) {
    while (*p == ' ')
      p++;
    if (*p == '\0')
      break;
    tokens[count++] = p;
    while (*p != '\0' && *p != ' ')
      p++;
    if (*p == ' ')
      *p++ = '\0';
  }
  return count;
}

void LitterRobot3Component::dump_config() { ESP_LOGCONFIG(TAG, "Litter Robot 3:"); }

void LitterRobot3Component::send_command(char command, const char *value) {
  char buffer[16];
  int length;
  if (value != nullptr)
    length = snprintf(buffer, sizeof(buffer), ")%c%s\r\n", command, value);
  else
    length = snprintf(buffer, sizeof(buffer), ")%c\r\n", command);
  if (length <= 0 || length >= static_cast<int>(sizeof(buffer)))
    return;
  this->write_array(reinterpret_cast<const uint8_t *>(buffer), length);
  if (command == 'S' && value != nullptr) {
    if (strcmp(value, "0") == 0) {
      ESP_LOGD(TAG, "ESP set %s to Off", command_name(command));
    } else if (value[0] == '1' && strlen(value) == 9) {
      ESP_LOGD(TAG, "ESP set %s to On (%s elapsed)", command_name(command), value + 1);
    } else {
      ESP_LOGD(TAG, "ESP set %s to %s", command_name(command), value);
    }
  } else if (value == nullptr) {
    ESP_LOGD(TAG, "ESP sent %s", command_name(command));
  } else {
    ESP_LOGD(TAG, "ESP set %s to %s", command_name(command), command_value_name(command, value));
  }
}

void LitterRobot3Component::set_cycle_delay(uint8_t minutes) {
  if (minutes > 15)
    return;
  char value[2] = {format_hex_pretty_char(minutes), '\0'};
  this->send_command('W', value);
}

void LitterRobot3Component::set_sleep(uint32_t seconds) {
  // "1" enables sleep; the rest is the elapsed position in the sleep cycle,
  // not a wall-clock time.
  char value[16];
  snprintf(value, sizeof(value), "1%02u:%02u:%02u", static_cast<unsigned>(seconds / 3600),
           static_cast<unsigned>((seconds / 60) % 60), static_cast<unsigned>(seconds % 60));
  this->send_command('S', value);
}

void LitterRobot3Component::disable_sleep() { this->send_command('S', "0"); }

void LitterRobot3Component::loop() {
  while (this->available()) {
    char c = static_cast<char>(this->read());

    // Frames end with "\r\n"; only '\n' marks the end of a line.
    if (c == '\r')
      continue;

    if (c != '\n') {
      if (this->rx_len_ < sizeof(this->rx_line_) - 1) {
        this->rx_line_[this->rx_len_++] = c;
      } else {
        // Frame too long for the buffer: drop it and resync on the next newline.
        this->rx_len_ = 0;
      }
      continue;
    }

    // End of line: dispatch if non-empty, then start collecting the next one.
    if (this->rx_len_ > 0) {
      this->rx_line_[this->rx_len_] = '\0';
      this->handle_line_(this->rx_line_);
    }
    this->rx_len_ = 0;
  }
}

void LitterRobot3Component::handle_response_(const char *response) {
  // '(' replies "({cmd}AOK" on success or "({cmd}NOK{reason}" on failure.
  char command = response[0];
  if (command == '\0') {
    ESP_LOGW(TAG, "PIC sent an empty command response");
    return;
  }

  const char *result = &response[1];
  if (strncmp(result, "AOK", 3) == 0) {
    ESP_LOGD(TAG, "PIC acknowledged %s", command_name(command));
    return;
  }
  if (strncmp(result, "NOK", 3) == 0) {
    ESP_LOGW(TAG, "PIC rejected %s (%d)", command_name(command), atoi(&result[3]));
    return;
  }

  ESP_LOGW(TAG, "Unexpected command response: (%s", response);
}

void LitterRobot3Component::handle_line_(char *line) {
  // '(' is a command acknowledgement from the PIC, e.g. "(CAOK" or "(CNOK4".
  if (line[0] == '(') {
    this->handle_response_(&line[1]);
    return;
  }

  // ")F0"/")F1" report the PIC's WiFi provisioning state; echo the same state back.
  if (line[0] == ')' && line[1] == 'F' && (line[2] == '0' || line[2] == '1') && line[3] == '\0') {
    this->send_command('F', line[2] == '1' ? "1" : "0");
    return;
  }

  // ")Y####" is sent mid clean cycle and carries the total clean cycle count.
  if (line[0] == ')' && line[1] == 'Y') {
    this->handle_clean_cycle_count_(&line[2]);
    return;
  }

  // A status frame starts at the power-supply field, so the character after ')' is
  // '0' (AC) or '1' (battery). Pass the whole line so the field indices stay aligned.
  if (line[0] == ')' && (line[1] == '0' || line[1] == '1')) {
    this->handle_status_(line);
    return;
  }

  ESP_LOGD(TAG, "Unexpected message format: %s", line);
}

void LitterRobot3Component::handle_status_(char *line) {
  // split_tokens() terminates each field in place, so keep the raw line for logging.
  char raw[LINE_BUFFER_SIZE];
  snprintf(raw, sizeof(raw), "%s", line);

  ESP_LOGV(TAG, "PIC status: %s", raw);

  char *tokens[7];
  if (split_tokens(line, tokens, 7) < 7) {
    ESP_LOGW(TAG, "Malformed status packet: %s", raw);
    return;
  }

  // Fields: )P XS W N [1hh:mm:ss] L ZCCC
  // The last field is one digit followed by the 3-hex cat sensor value.
  int16_t cat_sensor = -1;
  if (strlen(tokens[6]) == 4) {
    if (auto parsed = parse_hex<uint16_t>(&tokens[6][1], 3); parsed.has_value())
      cat_sensor = static_cast<int16_t>(*parsed);
  }

  const Status status{
      .power_type = static_cast<uint8_t>(tokens[0][1] - '0'),
      .power_state = static_cast<uint8_t>(tokens[1][0] - '0'),
      .system_state = parse_hex_char(tokens[1][1]),
      .cycle_delay = parse_hex_char(tokens[2][0]),
      .night_light = tokens[3][0] != '0',
      .sleep_enabled = tokens[4][0] != '0',
      .panel_lockout = tokens[5][0] != '0',
      .cat_sensor = cat_sensor,
  };

  if (!this->has_status_ || status.power_type != this->last_status_.power_type) {
    log_status_field("Power source", power_type_name(status.power_type), status.power_type);
  }
  if (!this->has_status_ || status.power_state != this->last_status_.power_state) {
    log_status_field("Power", power_state_name(status.power_state), status.power_state);
  }
  if (!this->has_status_ || status.system_state != this->last_status_.system_state) {
    log_status_field("System state", system_state_name(status.system_state), status.system_state);
  }
  if (!this->has_status_ || status.cycle_delay != this->last_status_.cycle_delay) {
    log_status_field("Cycle delay", cycle_delay_name(status.cycle_delay), status.cycle_delay);
  }
  if (!this->has_status_ || status.night_light != this->last_status_.night_light) {
    ESP_LOGD(TAG, "Night light: %s", status.night_light ? LOG_STR_LITERAL("On") : LOG_STR_LITERAL("Off"));
  }
  if (!this->has_status_ || status.sleep_enabled != this->last_status_.sleep_enabled) {
    ESP_LOGD(TAG, "Sleep mode: %s", status.sleep_enabled ? LOG_STR_LITERAL("On") : LOG_STR_LITERAL("Off"));
  }
  if (!this->has_status_ || status.panel_lockout != this->last_status_.panel_lockout) {
    ESP_LOGD(TAG, "Panel lockout: %s", status.panel_lockout ? LOG_STR_LITERAL("On") : LOG_STR_LITERAL("Off"));
  }
  if (!this->has_status_ || status.cat_sensor != this->last_status_.cat_sensor) {
    if (status.cat_sensor < 0) {
      ESP_LOGD(TAG, "Cat sensor: Unknown");
    } else {
      ESP_LOGD(TAG, "Cat sensor: %u", static_cast<unsigned>(status.cat_sensor));
    }
  }

  this->last_status_ = status;
  this->has_status_ = true;

  this->status_callback_.call(LR3_FIELD_POWER_TYPE, status.power_type);
  this->status_callback_.call(LR3_FIELD_POWER_STATE, status.power_state);
  this->status_callback_.call(LR3_FIELD_SYSTEM_STATE, status.system_state);
  this->status_callback_.call(LR3_FIELD_CYCLE_DELAY, status.cycle_delay);
  this->status_callback_.call(LR3_FIELD_NIGHT_LIGHT, status.night_light ? 1 : 0);
  this->status_callback_.call(LR3_FIELD_SLEEP_ENABLED, status.sleep_enabled ? 1 : 0);
  this->status_callback_.call(LR3_FIELD_PANEL_LOCKOUT, status.panel_lockout ? 1 : 0);
  if (status.cat_sensor >= 0)
    this->status_callback_.call(LR3_FIELD_CAT_SENSOR, static_cast<uint32_t>(status.cat_sensor));
}

void LitterRobot3Component::handle_clean_cycle_count_(const char *value) {
  char *end = nullptr;
  unsigned long count = strtoul(value, &end, 16);
  if (end == value) {
    ESP_LOGW(TAG, "Malformed cycle count packet: )Y%s", value);
    return;
  }
  ESP_LOGD(TAG, "PIC reported clean cycle count %lu", count);
  this->status_callback_.call(LR3_FIELD_CLEAN_CYCLE_COUNT, static_cast<uint32_t>(count));
}

}  // namespace esphome::litter_robot3
