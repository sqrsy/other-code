///////////////////////////////////////////////////////////////////////////////
/// DEFINE HARDWARE PARAMETERS FOR MODULE
///////////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_JACKS 2
#define V_DIVIDER_R1 220
#define V_DIVIDER_R2 150
#define JACK_IS_DIGITAL \
  { false, false }
#define PINS_JACK \
  { A3, A4 }

#define NUMBER_OF_POTS 2
#define MAX_POT_VOLTAGE 4900
#define REVERSE_POT true
#define PINS_POT \
  { A0, A1 }

#define NUMBER_OF_SWITCHES 1
#define PINS_SWITCH \
  { 7 }

#define NUMBER_OF_DIGITAL_OUTPUTS 0
#define PINS_DIGITAL_OUTPUT \
  {}
#define PINS_DAC_A \
  { -1, -1, -1, -1 }
#define PINS_DAC_B \
  { -1, -1, -1, -1 }

#define PINS_SN76489 \
  { 5, 6, 12, 11, 10, 9, 8, 2, 3, 4 }  // WE, CE, D0-D7
