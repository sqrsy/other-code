///////////////////////////////////////////////////////////////////////////////
/// DEFINE HARDWARE PARAMETERS FOR MODULE
///////////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_INPUTS 1
#define V_DIVIDER_R1 220
#define V_DIVIDER_R2 150
#define PINS_INPUT \
  { A2 }

#define NUMBER_OF_POTS 2
#define MAX_POT_VOLTAGE 5000
#define REVERSE_POT true
#define PINS_POT \
  { A0, A1 }

#define NUMBER_OF_SWITCHES 0
#define PINS_SWITCH \
  {}

#define NUMBER_OF_OUTPUTS 2
#define PINS_OUTPUT \
  { 10, 11 }
#define PINS_DAC_A \
  { 11, 10, 9, -1 }
#define PINS_DAC_B \
  { -1, -1, -1, -1 }
