/*
  MWorksDigitalPulse.cpp
 */

#include "MWorksDigitalPulse.h"


MWorksDigitalPulse::MWorksDigitalPulse()
{
  reset();
}

void MWorksDigitalPulse::handleCapability(byte pin)
{
  if (IS_PIN_DIGITAL(pin)) {
    Firmata.write(MWORKS_PIN_MODE_INPUT_PULSE);
    Firmata.write(DURATION_RESOLUTION);
    Firmata.write(MWORKS_PIN_MODE_OUTPUT_PULSE);
    Firmata.write(DURATION_RESOLUTION);
  }
}

boolean MWorksDigitalPulse::handlePinMode(byte pin, int mode)
{
  if (IS_PIN_DIGITAL(pin)) {
    if (mode == MWORKS_PIN_MODE_INPUT_PULSE) {
      setPinBit(inputPins, pin);
      clearPinBit(outputPins, pin);
      clearPinBit(activePins, pin);
      setPinBit(reportPins, pin);  // Enable reporting by default

      pinMode(PIN_TO_DIGITAL(pin), INPUT);  // Disable output driver
      Firmata.setPinMode(pin, MWORKS_PIN_MODE_INPUT_PULSE);

      return true;
    }

    if (mode == MWORKS_PIN_MODE_OUTPUT_PULSE) {
      setPinBit(outputPins, pin);
      clearPinBit(inputPins, pin);
      clearPinBit(activePins, pin);

      if (Firmata.getPinMode(pin) == PIN_MODE_PWM) {
        // Disable PWM if pin mode was previously set to PWM
        digitalWrite(PIN_TO_DIGITAL(pin), LOW);
      }
      pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
      Firmata.setPinMode(pin, MWORKS_PIN_MODE_OUTPUT_PULSE);

      return true;
    }
  }

  return false;
}

boolean MWorksDigitalPulse::handleSysex(byte command, byte argc, byte *argv)
{
  if (command == MWORKS_DIGITAL_PULSE) {
    if (argc > 1) {
      byte pin = argv[0];
      if (pin < TOTAL_PINS && isPinBitSet(outputPins, pin)) {
        durationMillis[pin] = 0;
        while (true) {
          durationMillis[pin] |= (argv[argc - 1] & 0x7F);
          argc--;
          if (argc < 2) {
            break;
          }
          durationMillis[pin] <<= 7;
        }
        if (durationMillis[pin]) {
          startMillis[pin] = millis();
          setPinBit(activePins, pin);
          Firmata.setPinState(pin, 1);
          digitalWrite(PIN_TO_DIGITAL(pin), HIGH);
        } else if (isPinBitSet(activePins, pin)) {
          // If duration is zero, cancel any current pulse
          clearPinBit(activePins, pin);
          Firmata.setPinState(pin, 0);
          digitalWrite(PIN_TO_DIGITAL(pin), LOW);
        }
      }
    }
    return true;
  }

  if (command == MWORKS_REPORT_DIGITAL_PULSE) {
    if (argc == 2) {
      byte pin = argv[0];
      if (pin < TOTAL_PINS && isPinBitSet(inputPins, pin)) {
        if (argv[1]) {
          setPinBit(reportPins, pin);
        } else {
          clearPinBit(reportPins, pin);
        }
      }
    }
    return true;
  }

  return false;
}

void MWorksDigitalPulse::update()
{
  unsigned long currentMillis = millis();

  // Outputs
  for (byte port = 0; port < TOTAL_PORTS; port++) {
    byte activeOutputPins = outputPins[port] & activePins[port];
    byte bit = 0;
    while (activeOutputPins) {
      if (activeOutputPins & 1) {
        byte pin = getPinNumber(port, bit);
        // Compare durations, instead of absolute tick counts, as the former
        // are insensitive to overflow
        if (currentMillis - startMillis[pin] >= durationMillis[pin]) {
          clearPinBit(activePins, pin);
          Firmata.setPinState(pin, 0);
          digitalWrite(PIN_TO_DIGITAL(pin), LOW);
        }
      }
      activeOutputPins >>= 1;
      bit++;
    }
  }

  // Inputs
  for (byte port = 0; port < TOTAL_PORTS; port++) {
    byte activeInputPins = inputPins[port] & activePins[port];
    byte inactiveInputPins = inputPins[port] & ~(activePins[port]);
    byte pinState = readPort(port, inputPins[port]);
    byte bit = 0;
    while (activeInputPins || inactiveInputPins) {
      if (pinState & 1) {
        // Pin is high.  If it's currently inactive, mark it active and record
        // the start time.
        if (inactiveInputPins & 1) {
          byte pin = getPinNumber(port, bit);
          setPinBit(activePins, pin);
          startMillis[pin] = currentMillis;
        }
      } else {
        // Pin is low.  If it's currently active, mark it inactive and report
        // the pulse duration.
        if (activeInputPins & 1) {
          byte pin = getPinNumber(port, bit);
          clearPinBit(activePins, pin);
          durationMillis[pin] = currentMillis - startMillis[pin];
          if (isPinBitSet(reportPins, pin)) {
            Firmata.write(START_SYSEX);
            Firmata.write(MWORKS_DIGITAL_PULSE);
            Firmata.write(pin);
            while (durationMillis[pin]) {
              Firmata.write(byte(durationMillis[pin]) & 0x7F);
              durationMillis[pin] >>= 7;
            }
            Firmata.write(END_SYSEX);
          }
        }
      }
      activeInputPins >>= 1;
      inactiveInputPins >>= 1;
      pinState >>= 1;
      bit++;
    }
  }
}

void MWorksDigitalPulse::reset()
{
  for (byte port = 0; port < TOTAL_PORTS; port++) {
    inputPins[port] = 0;
    outputPins[port] = 0;
  }
}
