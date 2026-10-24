#include "litter_robot3.h"

namespace esphome::litter_robot3 {

struct CommandInfo {
  char command;
  const char *name;
};

struct SystemStateInfo {
  uint8_t state;
  const char *name;
};

struct BoolValueInfo {
  char value;
  const char *name;
};

struct CycleDelayInfo {
  char code;
  const char *name;
};

static const CommandInfo COMMAND_NAMES[] = {
    {'C', "Clean cycle"},   {'D', "Factory reset"},  {'F', "WiFi provisioning"},
    {'L', "Panel lockout"}, {'N', "Night light"},    {'P', "Power"},
    {'R', "Reset"},         {'S', "Sleep schedule"}, {'W', "Cycle delay"},
};

static const SystemStateInfo SYSTEM_STATE_NAMES[] = {
    {0, "Calibrating or provisioning"},
    {1, "Ready"},
    {2, "Cat detected"},
    {3, "Clean cycle in progress"},
    {5, "Cycle interrupted"},
    {6, "Cycle paused"},
    {7, "Bonnet removed"},
    {8, "Pinch detected"},
    {9, "Waste drawer full"},
    {10, "Waste drawer full"},
    {11, "Waste drawer full"},
};

static const BoolValueInfo BOOL_VALUE_NAMES[] = {
    {'0', "Off"},
    {'1', "On"},
};

static const CycleDelayInfo CYCLE_DELAY_NAMES[] = {
    {'3', "3 minutes"},
    {'7', "7 minutes"},
    {'F', "15 minutes"},
};

const char *command_name(char command) {
  for (const auto &info : COMMAND_NAMES) {
    if (info.command == command)
      return info.name;
  }
  return "Unknown command";
}

const char *command_value_name(char command, const char *value) {
  switch (command) {
    case 'L':
    case 'N':
    case 'P':
      for (const auto &info : BOOL_VALUE_NAMES) {
        if (info.value == value[0])
          return info.name;
      }
      break;
    case 'W':
      for (const auto &info : CYCLE_DELAY_NAMES) {
        if (info.code == value[0])
          return info.name;
      }
      break;
    default:
      break;
  }
  return value;
}

const char *power_type_name(uint8_t type) {
  switch (type) {
    case 0:
      return "AC";
    case 1:
      return "Battery";
    default:
      return nullptr;
  }
}

const char *power_state_name(uint8_t state) {
  switch (state) {
    case 0:
      return "On";
    case 1:
      return "Off (standby)";
    default:
      return nullptr;
  }
}

const char *cycle_delay_name(uint8_t delay) {
  switch (delay) {
    case 3:
      return "3 minutes";
    case 7:
      return "7 minutes";
    case 15:
      return "15 minutes";
    default:
      return nullptr;
  }
}

const char *system_state_name(uint8_t state) {
  for (const auto &info : SYSTEM_STATE_NAMES) {
    if (info.state == state)
      return info.name;
  }
  return nullptr;
}

}  // namespace esphome::litter_robot3
