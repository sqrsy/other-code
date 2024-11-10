///////////////////////////////////////////////////////////////////////////////
/// DEFINE HARDWARE PARAMETERS FOR MODULE
///////////////////////////////////////////////////////////////////////////////

#define NUMBER_OF_INPUTS 2
#define V_DIVIDER_R1 220
#define V_DIVIDER_R2 150
#define PINS_INPUT \
  { A5, A4 }

#define NUMBER_OF_POTS 2
#define MAX_POT_VOLTAGE 4900
#define REVERSE_POT true
#define PINS_POT \
  { A6, A7 }

#define NUMBER_OF_SWITCHES 12
#define PINS_SWITCH \
  { 2, A0, A1, A3, A2, 12, 11, 10, 9, 8, 7, 6 }

#define NUMBER_OF_OUTPUTS 2
#define PINS_OUTPUT \
  { 4, 3 }
#define PINS_DAC_A \
  { 13, 3, 4, 5 }
#define PINS_DAC_B \
  { -1, -1, -1, -1 }
