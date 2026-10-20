#include "ot_boiler_emulator.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ot_boiler_emulator {

static const char *const TAG = "ot_boiler_emulator";

OTBoilerEmulator *OTBoilerEmulator::instance = nullptr;

void IRAM_ATTR OTBoilerEmulator::handle_interrupt_static() {
  if (instance != nullptr && instance->ot_ != nullptr)
    instance->ot_->handleInterrupt();
}

void OTBoilerEmulator::on_request_static(unsigned long request, OpenThermResponseStatus status, void *context) {
  static_cast<OTBoilerEmulator *>(context)->on_request(request, status);
}

void OTBoilerEmulator::setup() {
  instance = this;
  this->ot_ = new OpenTherm(this->in_pin_, this->out_pin_, /*isSlave=*/true);
  this->ot_->begin(&OTBoilerEmulator::handle_interrupt_static, &OTBoilerEmulator::on_request_static, this);
}

void OTBoilerEmulator::dump_config() {
  ESP_LOGCONFIG(TAG, "OT Boiler Emulator:");
  ESP_LOGCONFIG(TAG, "  In Pin: GPIO%d", this->in_pin_);
  ESP_LOGCONFIG(TAG, "  Out Pin: GPIO%d", this->out_pin_);
  ESP_LOGCONFIG(TAG, "  Comms timeout: %u ms", this->comms_timeout_ms_);
}

void OTBoilerEmulator::loop() {
  if (this->ot_ != nullptr)
    this->ot_->process();

  bool active = this->last_request_ms_ != 0 && (millis() - this->last_request_ms_) < this->comms_timeout_ms_;
  if (active != this->comms_was_active_) {
    this->comms_was_active_ = active;
    if (this->comms_active_sensor_ != nullptr)
      this->comms_active_sensor_->publish_state(active);
    if (!active) {
      ESP_LOGW(TAG, "No request received from thermostat in %u ms", this->comms_timeout_ms_);
    }
  }
}

void OTBoilerEmulator::on_request(unsigned long request, OpenThermResponseStatus status) {
  if (status != OpenThermResponseStatus::SUCCESS) {
    // Logged even on failure so a log capture can tell "no electrical
    // activity at all" apart from "frames arriving but rejected" (e.g. bad
    // parity from a pin-direction mismatch) - the two look identical from
    // the HA entity side, since neither ever reaches a successful reply.
    ESP_LOGD(TAG, "Rejected request (%08lX): %s", request, this->ot_->statusToString(status));
    return;
  }

  this->last_request_ms_ = millis();

  ESP_LOGD(TAG, "Request (%08lX): MessageType: %s, DataID: %d", request,
           this->ot_->messageTypeToString(this->ot_->getMessageType(request)), this->ot_->getDataID(request));

  unsigned long reply = this->build_reply_(request);
  this->ot_->sendResponse(reply);
}

unsigned long OTBoilerEmulator::build_reply_(unsigned long request) {
  OpenThermMessageType type = this->ot_->getMessageType(request);
  OpenThermMessageID id = this->ot_->getDataID(request);
  uint16_t data = (uint16_t) request;
  uint8_t hb = (uint8_t) (data >> 8);

  switch (id) {
    case OpenThermMessageID::Status: {
      // Master (the Nest) status flags are in the high byte of the
      // request: bit0 = CH enable, bit1 = DHW enable. Confirmed against
      // this project's real boiler traffic: the slave's reply echoes the
      // master's high byte back unchanged, and fills in its own status
      // flags in the low byte.
      this->ch_enable_requested_ = (hb & 0x01) != 0;
      this->dhw_enable_requested_ = (hb & 0x02) != 0;
      if (this->ch_enable_sensor_ != nullptr)
        this->ch_enable_sensor_->publish_state(this->ch_enable_requested_);
      if (this->dhw_enable_sensor_ != nullptr)
        this->dhw_enable_sensor_->publish_state(this->dhw_enable_requested_);

      // Slave status flags we report back, low byte:
      //   bit0 fault, bit1 CH active, bit2 DHW active, bit3 flame,
      //   bit4 cooling, bit5 CH2, bit6 diagnostic
      // Nothing is actually firing here - we just mirror back "active" for
      // whatever the Nest just asked us to enable, which is the simplest
      // way to look like a healthy, compliant boiler without ever
      // asserting a fault. Real demand is handled entirely separately by
      // the actual master-mode OTGW device talking to the real boiler;
      // this unit's only job is to keep the Nest satisfied.
      uint8_t slave_status = 0;
      if (this->ch_enable_requested_)
        slave_status |= 0x02;
      if (this->dhw_enable_requested_)
        slave_status |= 0x04;
      if (this->ch_enable_requested_ || this->dhw_enable_requested_)
        slave_status |= 0x08;  // flame
      uint16_t reply_data = ((uint16_t) hb << 8) | slave_status;
      return this->ot_->buildResponse(OpenThermMessageType::READ_ACK, id, reply_data);
    }

    case OpenThermMessageID::TSet: {
      float setpoint = this->ot_->getFloat(request);
      if (this->boiler_setpoint_sensor_ != nullptr)
        this->boiler_setpoint_sensor_->publish_state(setpoint);
      // Accept whatever flow-temperature setpoint the Nest asked for.
      return this->ot_->buildResponse(OpenThermMessageType::WRITE_ACK, id, data);
    }

    default:
      if (type == OpenThermMessageType::READ_DATA) {
        // Unimplemented read: tell the master this Data-ID isn't
        // supported. This is the spec-defined, non-fatal way to decline,
        // rather than making up a plausible-looking value for something
        // we have no real data for.
        return this->ot_->buildResponse(OpenThermMessageType::UNKNOWN_DATA_ID, id, 0);
      } else {
        // Unimplemented write (or anything else unexpected): accept it
        // without acting on it, so the master never sees a failed write.
        return this->ot_->buildResponse(OpenThermMessageType::WRITE_ACK, id, data);
      }
  }
}

}  // namespace ot_boiler_emulator
}  // namespace esphome
