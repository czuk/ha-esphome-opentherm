# OpenTherm Gateway (ESPHome)

ESPHome configs for a two-device OpenTherm setup, built to let a Home
Assistant installation control a boiler directly while keeping a Google
Nest Heat Link usable as a day-to-day thermostat/display.

## Devices

- **`otgw.yaml`** (`esphome-otgw`) - Wemos D1 Mini using the master shield of
  a DIYLESS dual-shield OpenTherm board. Drives the real boiler directly
  using ESPHome's official `opentherm:` component, with a PID climate loop
  setting the boiler flow temperature from a whole-house average radiator
  temperature pulled in from Home Assistant.

- **`nest_otgw_listener.yaml`** (`esphome-nest-otgw`) - Wemos D1 Mini using
  the slave shield. Runs the custom `ot_boiler_emulator` component below so
  a Nest Heat Link stays wired up and believes it's driving a boiler,
  without it ever touching the real one. Publishes what the Nest is asking
  for (CH/DHW enable, requested flow setpoint) as plain HA entities.

## Custom component

`components/ot_boiler_emulator/` - an ESPHome external component that acts
as an OpenTherm slave, answering every request from a real thermostat with
a plausible, locally-fabricated reply. Built on `OpenTherm.h`/`.cpp`, the
low-level OpenTherm driver from
[ihormelnyk/OpenTherm](https://github.com/ihormelnyk/OpenTherm) (MIT
licensed, vendored here unmodified).

## Setup

Copy `secrets.yaml.example` to `secrets.yaml` and fill in real values
(`secrets.yaml` is gitignored). Generate each API key with:

```
openssl rand -base64 32
```
