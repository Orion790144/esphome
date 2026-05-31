#include "dxs238xw.h"
namespace esphome {
namespace dxs238xw {
static const char *const TAG = "dxs238xw";

#ifdef USE_SENSOR
#define UPDATE_SENSOR(name, value) \
  if (this->name##_sensor_ != nullptr) { \
    if (this->name##_sensor_->get_raw_state() != (value) || this->get_component_state() == COMPONENT_STATE_SETUP) { \
      this->name##_sensor_->publish_state(value); \
    } \
  }
#else
#define UPDATE_SENSOR(name, value)
#endif

#ifdef USE_SENSOR
#define UPDATE_SENSOR_MEASUREMENTS_(name, value, factor) \
  if (this->name##_sensor_ != nullptr) { \
    float value_float = value; \
    if (factor > 0 && value_float >= factor) { \
      value_float = factor - value_float; \
    } \
    if (this->name##_sensor_->get_raw_state() != value_float || this->get_component_state() == COMPONENT_STATE_SETUP) { \
      this->name##_sensor_->publish_state(value_float); \
    } \
  }
#define UPDATE_SENSOR_MEASUREMENTS(name, value) UPDATE_SENSOR_MEASUREMENTS_(name, value, 0)
#define UPDATE_SENSOR_MEASUREMENTS_POWER(name, value) UPDATE_SENSOR_MEASUREMENTS_(name, value, 100)
#define UPDATE_SENSOR_MEASUREMENTS_CURRENT(name, value) UPDATE_SENSOR_MEASUREMENTS_(name, value, 1000)
#else
#define UPDATE_SENSOR_MEASUREMENTS(name, value)
#define UPDATE_SENSOR_MEASUREMENTS_POWER(name, value)
#define UPDATE_SENSOR_MEASUREMENTS_CURRENT(name, value)
#endif

#ifdef USE_TEXT_SENSOR
#define UPDATE_TEXT_SENSOR(name, value) \
  if (this->name##_text_sensor_ != nullptr) { \
    if (this->name##_text_sensor_->get_raw_state() != (value) || this->get_component_state() == COMPONENT_STATE_SETUP) { \
      this->name##_text_sensor_->publish_state(value); \
    } \
  }
#else
#define UPDATE_TEXT_SENSOR(name, value)
#endif

#ifdef USE_BINARY_SENSOR
#define UPDATE_BINARY_SENSOR(name, value) \
  if (this->name##_binary_sensor_ != nullptr) { \
    if (this->name##_binary_sensor_->state != (value) || this->get_component_state() == COMPONENT_STATE_SETUP) { \
      this->name##_binary_sensor_->publish_state(value); \
    } \
  }
#else
#define UPDATE_BINARY_SENSOR(name, value)
#endif

#ifdef USE_NUMBER
#define UPDATE_NUMBER(name, value) \
  if (this->name##_number_ != nullptr) { \
    if (this->name##_number_->state != (value) || this->get_component_state() == COMPONENT_STATE_SETUP) { \
      this->name##_number_->publish_state(value); \
    } \
  }
#else
#define UPDATE_NUMBER(name, value)
#endif

#ifdef USE_SWITCH
#define UPDATE_SWITCH(name, value) \
  if (this->name##_switch_ != nullptr) { \
    if (this->name##_switch_->state != (value) || this->get_component_state() == COMPONENT_STATE_SETUP) { \
      this->name##_switch_->publish_state(value); \
    } \
  }
#else
#define UPDATE_SWITCH(name, value)
#endif

#define LOAD_PREFERENCE(name, preference_string, default_value, data) \
  this->preference_##name##_ = global_preferences->make_preference<uint32_t>(this->hash_##name##_); \
  if (this->name##_number_ != nullptr) { \
    this->data##_.name = this->read_initial_number_value_(this->preference_##name##_, preference_string, default_value); \
  }

#define LOAD_PREFERENCE_MS(name, preference_string, default_value) LOAD_PREFERENCE(name, preference_string, default_value, ms_data)
#define LOAD_PREFERENCE_LP(name, preference_string, default_value) LOAD_PREFERENCE(name, preference_string, default_value, lp_data)

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------

float Dxs238xwComponent::get_setup_priority() const {
  return setup_priority::LATE;
}

void Dxs238xwComponent::setup() {
  if (this->postpone_setup_time_ == 0) {
    this->postpone_setup_time_ = millis() + SM_POSTPONE_SETUP_TIME;
  }
  if (postpone_setup_time_ > millis()) {
    this->component_state_ &= ~COMPONENT_STATE_MASK;
    this->component_state_ |= COMPONENT_STATE_CONSTRUCTION;
  } else {
    if (this->count_error_data_acquisition_ == 0) {
      ESP_LOGI(TAG, "In --- setup");
      ESP_LOGI(TAG, "* Get Initial Values");
      LOAD_PREFERENCE_MS(delay_value_set, SM_STR_DELAY_VALUE_SET, SmLimitValue::MAX_DELAY_SET)
      LOAD_PREFERENCE_MS(starting_kWh, SM_STR_STARTING_KWH, 0.0f)
      LOAD_PREFERENCE_MS(price_kWh, SM_STR_PRICE_KWH, 0.0f)
      LOAD_PREFERENCE_LP(energy_purchase_value, SM_STR_ENERGY_PURCHASE_VALUE, SmLimitValue::MIN_ENERGY_PURCHASE_VALUE)
      LOAD_PREFERENCE_LP(energy_purchase_alarm, SM_STR_ENERGY_PURCHASE_ALARM, SmLimitValue::MIN_ENERGY_PURCHASE_ALARM)
      UPDATE_NUMBER(delay_value_set, this->ms_data_.delay_value_set)
      UPDATE_NUMBER(starting_kWh, this->ms_data_.starting_kWh)
      UPDATE_NUMBER(price_kWh, this->ms_data_.price_kWh)
      UPDATE_SENSOR(price_kWh, this->ms_data_.price_kWh)
      UPDATE_NUMBER(energy_purchase_value, this->lp_data_.energy_purchase_value)
      UPDATE_NUMBER(energy_purchase_alarm, this->lp_data_.energy_purchase_alarm)
    }
    if (!this->first_data_acquisition_()) {
      this->component_state_ &= ~COMPONENT_STATE_MASK;
      this->component_state_ |= COMPONENT_STATE_CONSTRUCTION;
      this->status_set_error();
      this->count_error_data_acquisition_++;
      this->postpone_setup_time_ = 0;
      return;
    } else {
      if (this->count_error_data_acquisition_ > 0) {
        this->status_clear_error();
      }
    }
    ESP_LOGI(TAG, "Out --- setup");
  }
}

void Dxs238xwComponent::loop() {
  this->incoming_messages_();
  this->send_command_(SmCommandSend::GET_POWER_STATE);
  this->send_command_(SmCommandSend::GET_LIMIT_AND_PURCHASE_DATA);
}

void Dxs238xwComponent::update() {
  if (this->get_component_state() == COMPONENT_STATE_LOOP) {
    this->send_command_(SmCommandSend::GET_MEASUREMENT_DATA);
  }
}

void Dxs238xwComponent::dump_config() {
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "*** COMPONENT VERSION: %s ***", SM_STR_COMPONENT_VERSION);
}

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------

void Dxs238xwComponent::meter_state_toggle() {
  this->set_meter_state_(!this->ms_data_.meter_state);
}

void Dxs238xwComponent::meter_state_on() {
  this->set_meter_state_(true);
}

void Dxs238xwComponent::meter_state_off() {
  this->set_meter_state_(false);
}

void Dxs238xwComponent::set_meter_state_(bool state) {
  this->ms_data_.warning_off_by_user = !state;
  this->send_command_(SmCommandSend::SET_POWER_STATE, state);
}

void Dxs238xwComponent::hex_message(std::string message, bool check_crc) {
  ESP_LOGD(TAG, "In --- send_hex_message");
  ESP_LOGD(TAG, "* message in = %s", message.c_str());
  this->error_type_ = SmErrorType::NO_ERROR;
  this->error_code_ = SmErrorCode::NO_ERROR;
  uint8_t length_message = message.length();
  if (length_message == 0 || length_message > SM_MAX_HEX_MSG_LENGTH) {
    this->error_type_ = SmErrorType::INPUT_DATA;
    this->error_code_ = SmErrorCode::MESSAGE_LENGTH;
  }
  if (this->error_code_ == SmErrorCode::NO_ERROR) {
    char tmp_message[SM_MAX_HEX_MSG_LENGTH_PARSE];
    uint8_t size_message_without_dots = 0;
    uint8_t character_hex_index = 0;
    for (uint8_t i = 0; i < length_message; i++) {
      if (message[i] == '.') {
        if (character_hex_index == 2) {
          character_hex_index = 0;
        } else {
          this->error_type_ = SmErrorType::INPUT_DATA;
          this->error_code_ = SmErrorCode::WRONG_MSG;
          break;
        }
      } else {
        if (character_hex_index == 2) {
          this->error_type_ = SmErrorType::INPUT_DATA;
          this->error_code_ = SmErrorCode::WRONG_MSG;
          break;
        } else {
          tmp_message[size_message_without_dots] = message[i];
          size_message_without_dots++;
          character_hex_index++;
        }
      }
    }
    if (this->error_code_ == SmErrorCode::NO_ERROR) {
      if ((size_message_without_dots % 2) != 0) {
        this->error_type_ = SmErrorType::INPUT_DATA;
        this->error_code_ = SmErrorCode::WRONG_MSG;
      }
      if (this->error_code_ == SmErrorCode::NO_ERROR) {
        const char *hex_message = tmp_message;
        uint8_t length_array = size_message_without_dots / 2;
        uint8_t send_array[length_array];
        parse_hex(hex_message, size_message_without_dots, send_array, length_array);
        if (check_crc) {
          if (this->calculate_crc_(send_array, length_array) != send_array[length_array - 1]) {
            this->error_type_ = SmErrorType::INPUT_DATA;
            this->error_code_ = SmErrorCode::CRC;
          }
        } else {
          send_array[length_array - 1] = this->calculate_crc_(send_array, length_array);
        }
        if (this->error_code_ == SmErrorCode::NO_ERROR) {
          ESP_LOGD(TAG, "* Message send: %s", format_hex_pretty(send_array, length_array).c_str());
          if (this->transmit_serial_data_(send_array, length_array)) {
            ESP_LOGD(TAG, "* Waiting answer:");
            if (this->receive_serial_data_(this->receive_array_, HEKR_TYPE_RECEIVE)) {
              ESP_LOGD(TAG, "* Successful answer: %s", format_hex_pretty(this->receive_array_, this->receive_array_[1]).c_str());
              this->process_and_update_data_(this->receive_array_);
              ESP_LOGD(TAG, "Out --- send_hex_message");
              return;
            }
            ESP_LOGD(TAG, "* Failed answer");
          }
        }
      }
    }
  }
  this->print_error_();
  ESP_LOGD(TAG, "Out --- send_hex_message");
}

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------

void Dxs238xwComponent::set_switch_value(SmIdEntity entity, bool value) {
  if (this->get_component_state() == COMPONENT_STATE_LOOP) {
    SmCommandSend tmp_cmd_send;
    switch (entity) {
      case SmIdEntity::SWITCH_ENERGY_PURCHASE_STATE: {
        tmp_cmd_send = SmCommandSend::SET_PURCHASE_DATA;
        break;
      }
      case SmIdEntity::SWITCH_METER_STATE: {
        this->ms_data_.warning_off_by_user = !value;
        tmp_cmd_send = SmCommandSend::SET_POWER_STATE;
        break;
      }
      case SmIdEntity::SWITCH_DELAY_STATE: {
        tmp_cmd_send = SmCommandSend::SET_DELAY;
        break;
      }
      default: {
        ESP_LOGE(TAG, "ID %hhu is not a SWITCH or is not included in the case list", entity);
        return;
      }
    }
    this->send_command_(tmp_cmd_send, value);
  }
}

void Dxs238xwComponent::set_button_value(SmIdEntity entity) {
  if (this->get_component_state() == COMPONENT_STATE_LOOP) {
    switch (entity) {
      case SmIdEntity::BUTTON_RESET_DATA: {
        this->send_command_(SmCommandSend::SET_PURCHASE_DATA, false);
        this->send_command_(SmCommandSend::SET_RESET);
        break;
      }
      default: {
        ESP_LOGE(TAG, "ID %hhu is not a BUTTON or is not included in the case list", entity);
        break;
      }
    }
  }
}

void Dxs238xwComponent::set_number_value(SmIdEntity entity, float value) {
  if (this->get_component_state() == COMPONENT_STATE_LOOP) {
    uint32_t tmp_value = std::round(value);
    switch (entity) {
      case SmIdEntity::NUMBER_MAX_CURRENT_LIMIT: {
        this->lp_data_.max_current_limit = tmp_value;
        this->send_command_(SmCommandSend::SET_LIMIT_DATA);
        UPDATE_NUMBER(max_current_limit, this->lp_data_.max_current_limit)
        break;
      }
      case SmIdEntity::NUMBER_MAX_VOLTAGE_LIMIT: {
        if (tmp_value > this->lp_data_.min_voltage_limit) {
          this->lp_data_.max_voltage_limit = tmp_value;
          this->send_command_(SmCommandSend::SET_LIMIT_DATA);
        } else {
          ESP_LOGW(TAG, "max_voltage_limit - Value %u must not be less than min_voltage_limit %u", tmp_value, this->lp_data_.min_voltage_limit);
        }
        UPDATE_NUMBER(max_voltage_limit, this->lp_data_.max_voltage_limit)
        break;
      }
      case SmIdEntity::NUMBER_MIN_VOLTAGE_LIMIT: {
        if (tmp_value < this->lp_data_.max_voltage_limit) {
          this->lp_data_.min_voltage_limit = tmp_value;
          this->send_command_(SmCommandSend::SET_LIMIT_DATA);
        } else {
          ESP_LOGW(TAG, "min_voltage_limit - Value %u must not be greater than max_voltage_limit %u", tmp_value, this->lp_data_.max_voltage_limit);
        }
        UPDATE_NUMBER(min_voltage_limit, this->lp_data_.min_voltage_limit)
        break;
      }
      case SmIdEntity::NUMBER_ENERGY_PURCHASE_VALUE: {
        this->lp_data_.energy_purchase_value = tmp_value;
        this->save_initial_number_value_(this->preference_energy_purchase_value_, this->lp_data_.energy_purchase_value);
        if (this->lp_data_.energy_purchase_state) {
          if (this->send_command_(SmCommandSend::SET_PURCHASE_DATA, false)) {
            this->send_command_(SmCommandSend::SET_PURCHASE_DATA, true);
          }
        }
        UPDATE_NUMBER(energy_purchase_value, this->lp_data_.energy_purchase_value)
        break;
      }
      case SmIdEntity::NUMBER_ENERGY_PURCHASE_ALARM: {
        this->lp_data_.energy_purchase_alarm = tmp_value;
        this->save_initial_number_value_(this->preference_energy_purchase_alarm_, this->lp_data_.energy_purchase_alarm);
        if (this->lp_data_.energy_purchase_state) {
          if (this->send_command_(SmCommandSend::SET_PURCHASE_DATA, false)) {
            this->send_command_(SmCommandSend::SET_PURCHASE_DATA, true);
          }
        }
        UPDATE_NUMBER(energy_purchase_alarm, this->lp_data_.energy_purchase_alarm)
        break;
      }
      case SmIdEntity::NUMBER_DELAY_VALUE_SET: {
        this->ms_data_.delay_value_set = tmp_value;
        this->save_initial_number_value_(this->preference_delay_value_set_, this->ms_data_.delay_value_set);
        if (this->ms_data_.delay_state) {
          this->send_command_(SmCommandSend::SET_DELAY, true);
        }
        UPDATE_NUMBER(delay_value_set, this->ms_data_.delay_value_set)
        break;
      }
      case SmIdEntity::NUMBER_STARTING_KWH: {
        this->ms_data_.starting_kWh = ((float) ((uint32_t) (value * 10))) / 10;
        this->save_initial_number_value_(this->preference_starting_kWh_, this->ms_data_.starting_kWh);
        UPDATE_SENSOR(contract_total_energy, this->ms_data_.starting_kWh + this->ms_data_.total_energy)
        UPDATE_NUMBER(starting_kWh, this->ms_data_.starting_kWh)
        break;
      }
      case SmIdEntity::NUMBER_PRICE_KWH: {
        this->ms_data_.price_kWh = ((float) ((uint32_t) (value * 10))) / 10;
        this->save_initial_number_value_(this->preference_price_kWh_, this->ms_data_.price_kWh);
        UPDATE_SENSOR(total_energy_price, this->ms_data_.price_kWh * this->ms_data_.total_energy)
        UPDATE_NUMBER(price_kWh, this->ms_data_.price_kWh)
        UPDATE_SENSOR(price_kWh, this->ms_data_.price_kWh)
        break;
      }
      default: {
        ESP_LOGE(TAG, "ID %hhu is not a NUMBER or is not included in the case list", entity);
        return;
      }
    }
  }
}

//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------

bool Dxs238xwComponent::first_data_acquisition_() {
  ESP_LOGI(TAG, "* Try to load GET_METER_ID");
  if (!this->send_command_(SmCommandSend::GET_METER_ID)) {
    return false;
  }
  ESP_LOGI(TAG, "* Try to load GET_POWER_STATE");
  if (!this->send_command_(SmCommandSend::GET_POWER_STATE)) {
    return false;
  }
  ESP_LOGI(TAG, "* Try to load GET_LIMIT_AND_PURCHASE_DATA");
  if (!this->send_command_(SmCommandSend::GET_LIMIT_AND_PURCHASE_DATA)) {
    return false;
  }
  ESP_LOGI(TAG, "* Try to load GET_MEASUREMENT_DATA");
  if (!this->send_command_(SmCommandSend::GET_MEASUREMENT_DATA)) {
    return false;
  }
  return true;
}

bool Dxs238xwComponent::transmit_serial_data_(uint8_t *array, uint8_t size) {
  while (available() > 0) {
    read();
    delay(2);
  }
  write_array(array, size);
  flush();
  ESP_LOGV(TAG, "* Waiting confirmation:");
  if (this->receive_serial_data_(array, HEKR_TYPE_SEND, array[4], size)) {
    ESP_LOGV(TAG, "* Successful Confirmation");
    return true;
  }
  ESP_LOGV(TAG, "* Confirmation Failed");
  return false;
}

bool Dxs238xwComponent::pre_transmit_serial_data_(uint8_t cmd, const uint8_t *array_data, uint8_t array_size) {
  static uint8_t version = 0;
  uint8_t send_array_size = (6 + array_size);
  uint8_t send_array[send_array_size];
  send_array[0] = HEKR_HEADER;
  send_array[1] = send_array_size;
  send_array[2] = HEKR_TYPE_SEND;
  send_array[3] = version++;
  send_array[4] = cmd;
  if (array_data != nullptr) {
    uint8_t send_array_index = 5;
    for (uint8_t i = 0; i < array_size; i++) {
      send_array[send_array_index] = array_data[i];
      send_array_index++;
    }
  }
  send_array[send_array_size - 1] = this->calculate_crc_(send_array, send_array_size);
  ESP_LOGV(TAG, "* Message send: %s", format_hex_pretty(send_array, send_array_size).c_str());
  return this->transmit_serial_data_(send_array, send_array_size);
}

bool Dxs238xwComponent::receive_serial_data_(uint8_t *array, uint8_t type_message, uint8_t cmd, uint8_t size_expected) {
  uint32_t response_time;
  uint8_t index_size = 0;
  SmErrorCode read_error = SmErrorCode::NO_ERROR;
  response_time = millis() + SM_MAX_MILLIS_TO_RESPONSE;
  while (true) {
    if (response_time < millis()) {
      if (index_size > 0) {
        read_error = SmErrorCode::NOT_ENOUGHT_BYTES;
      } else {
        read_error = SmErrorCode::TIMEOUT;
      }
      break;
    } else {
      if (available() > 0) {
        array[index_size] = read();
        if (index_size == 0 && array[0] != HEKR_HEADER) {
          ESP_LOGV(TAG, "* WRONG_BYTES: HEKR_HEADER / Expected = %u, Receive = %u", HEKR_HEADER, array[0]);
          read_error = SmErrorCode::WRONG_BYTES_HEADER;
          break;
        } else if (index_size == 1 && size_expected > 0 && array[1] != size_expected) {
          ESP_LOGV(TAG, "* WRONG_BYTES: HEKR_LENGTH / Expected = %u, Receive = %u", size_expected, array[1]);
          read_error = SmErrorCode::WRONG_BYTES_LENGTH;
          break;
        } else if (index_size == 2 && array[2] != type_message && array[2] != 0xFE && array[2] != 0x02) {
          ESP_LOGV(TAG, "* WRONG_BYTES: HEKR_TYPE_MESSAGE / Expected = %u, Receive = %u", type_message, array[2]);
          read_error = SmErrorCode::WRONG_BYTES_TYPE_MESSAGE;
          break;
        } else if (index_size == 4 && cmd > 0 && array[4] != cmd && array[2] != 0xFE && array[2] != 0x02) {
          ESP_LOGV(TAG, "* WRONG_BYTES: HEKR_COMMAND / Expected = %u, Receive = %u", cmd, array[4]);
          read_error = SmErrorCode::WRONG_BYTES_COMMAND;
          break;
        } else if (index_size > 4) {
          if (index_size == array[1] - 1) {
            ESP_LOGV(TAG, "* Message received: %s", format_hex_pretty(array, array[1]).c_str());
            if (array[2] != 0xFE && array[2] != 0x02 && this->calculate_crc_(array, array[1]) != array[index_size]) {
              read_error = SmErrorCode::CRC;
            }
            break;
          }
        }
        index_size++;
      }
    }
    yield();
  }
  delay(2);
  while (available() > 0) {
    read();
    delay(2);
  }
  if (read_error != SmErrorCode::NO_ERROR) {
    if (read_error == SmErrorCode::WRONG_BYTES_HEADER || read_error == SmErrorCode::WRONG_BYTES_LENGTH || read_error == SmErrorCode::WRONG_BYTES_TYPE_MESSAGE || read_error == SmErrorCode::WRONG_BYTES_COMMAND || read_error == SmErrorCode::CRC) {
      ESP_LOGD(TAG, "* Message with
