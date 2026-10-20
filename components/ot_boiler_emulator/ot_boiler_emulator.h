#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "OpenTherm.h"

namespace esphome {
namespace ot_boiler_emulator {

// Pretends to be an OpenTherm boiler so a real OpenTherm thermostat (e.g. a
// Google Nest Heat Link) can stay wired up and keep thinking it is driving a
// boiler, without this device ever touching a real boiler. It always
// replies to every request so the thermostat never sees "no response", and
// publishes what the thermostat is actually asking for (room/CH setpoint,
// CH/DHW enable) as plain HA entities, so Home Assistant can read - but does
// not have to obey - what the Nest wants. The real boiler is driven
// separately and independently by the master-mode OTGW device.
class OTBoilerEmulator : public Component {
 public:
  void set_pins(int in_pin, int out_pin) {
    in_pin_ = in_pin;
    out_pin_ = out_pin;
  }
  void set_comms_timeout(uint32_t seconds) { comms_timeout_ms_ = seconds * 1000UL; }

  void set_boiler_setpoint_sensor(sensor::Sensor *s) { boiler_setpoint_sensor_ = s; }
  void set_ch_enable_sensor(binary_sensor::BinarySensor *s) { ch_enable_sensor_ = s; }
  void set_dhw_enable_sensor(binary_sensor::BinarySensor *s) { dhw_enable_sensor_ = s; }
  void set_comms_active_sensor(binary_sensor::BinarySensor *s) { comms_active_sensor_ = s; }

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  // Called (via on_request_static) whenever the OpenTherm library hands us a
  // complete request frame from the real thermostat.
  void on_request(unsigned long request, OpenThermResponseStatus status);

  static OTBoilerEmulator *instance;
  static void IRAM_ATTR handle_interrupt_static();
  static void on_request_static(unsigned long request, OpenThermResponseStatus status, void *context);

 protected:
  unsigned long build_reply_(unsigned long request);

  int in_pin_{4};
  int out_pin_{5};
  uint32_t comms_timeout_ms_{60000};
  uint32_t last_request_ms_{0};
  // Starts true so the very first loop() iteration (where active is always
  // false, since no request has arrived yet) is detected as a change and
  // published - otherwise the binary_sensor sits at "unknown" until the
  // first real transition instead of reading a real "off" from boot.
  bool comms_was_active_{true};

  bool ch_enable_requested_{false};
  bool dhw_enable_requested_{false};

  OpenTherm *ot_{nullptr};

  sensor::Sensor *boiler_setpoint_sensor_{nullptr};
  binary_sensor::BinarySensor *ch_enable_sensor_{nullptr};
  binary_sensor::BinarySensor *dhw_enable_sensor_{nullptr};
  binary_sensor::BinarySensor *comms_active_sensor_{nullptr};
};

}  // namespace ot_boiler_emulator
}  // namespace esphome
