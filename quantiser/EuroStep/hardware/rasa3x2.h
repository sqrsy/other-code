///////////////////////////////////////////////////////////////////////////////
/// DEFINE HARDWARE PARAMETERS FOR MODULE
///////////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_INPUTS 2
#define V_DIVIDER_R1 220
#define V_DIVIDER_R2 150
#define PINS_INPUT \
  { A0, A7 }

#define NUMBER_OF_POTS 6
#define MAX_POT_VOLTAGE 5000
#define REVERSE_POT true
#define PINS_POT \
  { A2, A6, A5, A1, A3, A4 }

#define NUMBER_OF_SWITCHES 4
#define PINS_SWITCH \
  { 12, 11, 10, 9 }

#define NUMBER_OF_OUTPUTS 2
#define PINS_OUTPUT \
  { -1, -1 }
#define PINS_DAC_A \
  { 4, 5, 6, 7 }
#define PINS_DAC_B \
  { -1, -1, -1, -1 }
