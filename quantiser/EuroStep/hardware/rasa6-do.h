///////////////////////////////////////////////////////////////////////////////
/// DEFINE HARDWARE PARAMETERS FOR MODULE
///////////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_JACKS 1
#define V_DIVIDER_R1 220
#define V_DIVIDER_R2 150
#define JACK_IS_DIGITAL \
  { false }
#define PINS_JACK \
  { A2 }

#define NUMBER_OF_POTS 2
#define MAX_POT_VOLTAGE 4900
#define REVERSE_POT true
#define PINS_POT \
  { A0, A1 }

#define NUMBER_OF_SWITCHES 0
#define PINS_SWITCH \
  {}

#define NUMBER_OF_DIGITAL_OUTPUTS 2
#define PINS_DIGITAL_OUTPUT \
  { 10, 11 }
#define PINS_DAC_A \
  { -1, -1, -1, -1 }
#define PINS_DAC_B \
  { -1, -1, -1, -1 }
