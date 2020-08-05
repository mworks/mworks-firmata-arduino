/*
  MWorksDigitalPulse.h
 */

#ifndef MWorksDigitalPulse_h
#define MWorksDigitalPulse_h

#include <Firmata.h>
#include "FirmataFeature.h"

#include "MWorksDefines.h"

#define MWORKS_DIGITAL_PULSE_FEATURE


class MWorksDigitalPulse: public FirmataFeature
{
  public:
    MWorksDigitalPulse();

    void handleCapability(byte pin);
    boolean handlePinMode(byte pin, int mode);
    boolean handleSysex(byte command, byte argc, byte *argv);
    void update();
    void reset();

    void disablePin(byte pin) {
      if (pin < TOTAL_PINS) {
        clearPinBit(inputPins, pin);
        clearPinBit(outputPins, pin);
      }
    }

  private:
    static const int DURATION_RESOLUTION = sizeof(unsigned long) * 8;
    static const byte PINS_PER_PORT = 8;

    static byte getPinNumber(byte portNumber, byte bitNumber) {
      return portNumber * PINS_PER_PORT + bitNumber;
    }
    static boolean isPinBitSet(byte ports[], byte pin) {
      return ports[pin / PINS_PER_PORT] & (1 << (pin % PINS_PER_PORT));
    }
    static void setPinBit(byte ports[], byte pin) {
      ports[pin / PINS_PER_PORT] |= (1 << (pin % PINS_PER_PORT));
    }
    static void clearPinBit(byte ports[], byte pin) {
      ports[pin / PINS_PER_PORT] &= ~(1 << (pin % PINS_PER_PORT));
    }

    byte inputPins[TOTAL_PORTS];
    byte outputPins[TOTAL_PORTS];
    byte activePins[TOTAL_PORTS];
    byte reportPins[TOTAL_PORTS];
    unsigned long startMillis[TOTAL_PINS];
    unsigned long durationMillis[TOTAL_PINS];

};


#endif /* MWorksDigitalPulse_h */
