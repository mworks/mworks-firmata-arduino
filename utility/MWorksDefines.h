/*
  MWorksDefines.h
 */

#ifndef MWorksDefines_h
#define MWorksDefines_h


namespace mworks {
  // Per the Firmata protocol spec, sysex feature ID's 0x01-0x0F are reserved
  // for user-defined commands
  static const int DIGITAL_PULSE =        0x01;
  static const int REPORT_DIGITAL_PULSE = 0x02;

  // It'd be nice if the protocol allocated a range of pin modes for user
  // applications.  Since the "maximum" pin mode is IGNORE (0x7F), reserving
  // 0x70-0x7E would leave 15 pin modes available for applications (which neatly
  // mirrors the 15 reserved sysex feature ID's).  We've chosen our custom pin
  // modes as if this were the case.
  static const int PIN_MODE_INPUT_PULSE =  0x70;
  static const int PIN_MODE_OUTPUT_PULSE = 0x71;
}


#define MWORKS_DIGITAL_PULSE        ::mworks::DIGITAL_PULSE
#define MWORKS_REPORT_DIGITAL_PULSE ::mworks::REPORT_DIGITAL_PULSE

#define MWORKS_PIN_MODE_INPUT_PULSE  ::mworks::PIN_MODE_INPUT_PULSE
#define MWORKS_PIN_MODE_OUTPUT_PULSE ::mworks::PIN_MODE_OUTPUT_PULSE


#endif /* MWorksDefines_h */
