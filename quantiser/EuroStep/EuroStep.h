/*
Class Name: EuroStep

Purpose: Manage events that get called often by synthesis programs.

Dependencies: Hardware specific pin configuration. See 'hardware/' for examples.

Use: Create a sub-class for each new program, then over-ride any of the virtual functions:
-- on_start_do(): routines called once when program starts
-- on_clock_rise_do(): routines called whenever clock rises
-- -- User must enable the clock by assigning an INPUT via 'enable_clock_events(INPUT)';
-- on_clock_fall_do(): routines called whenever clock falls
-- -- Clock must be enabled, as above.
-- on_clock_rise_2_do(): routines called whenever the second clock rises
-- -- User must enable the clock by assigning an INPUT2 via 'enable_clock_Events_2(INPUT2)';
-- on_clock_fall_do(): routines called whenever the second clock falls
-- -- Clock must be enabled, as above.
-- on_step_do(): routines called every step in the main program loop

Incoming Values: Any inputs, pots, or switches get read automatically during each step.
The following getters can be used by the virtual functions:
-- get_input(index): returns the i-th read input values (in mV)
-- get_pot(index): returns the i-th read pot values (in percent, from 0-100)
-- get_switch(index): returns the i-th switch state (as a bool)

Outgoing Values: Any outputs get written automatically during each step.
The following setters and can be used by the virtual functions:
-- set_output(index, value): writes a value to an output channel
-- -- if index number is setup as analog, value is sent to DAC as mV (range 0-4 V)
-- -- if index number is setup as digital, value is sent by digitalWrite()
-- send_to_output(index, value): alias for set_output()

Setup Instructions: The following setters should be assigned in during setup:
-- set_input_to_analog(index, value): toggles whether the i-th input is used as analog input
-- -- if false, the input voltage is compared against 'input_is_true_threshold' (default is 500 mV)
-- -- note: false by default
-- set_output_to_analog(index, value): toggles whether the i-th output is used as analog output
-- -- if true, the write instructions will get sent to the MCP4822 to perform DAC (range 0-4 V)
-- -- note: the MCP4822 can support two analog outputs per chip (max two MCP4822 chips)
-- -- note: false by default
-- set_debug(value): sets whether to delay after each step and print to console
-- -- note: true by default

The following functions are required to run the program:
-- start(): put in setup to initialise class
-- step(): put in main to run the program

Example:

class make_new_module : public EuroStep::EuroStep {
public:
  void on_step_do() {
    // your program here
  }
};

make_new_module module;  // make the class

// RUNS ONCE
void setup() {
  module.set_input_to_analog(0, false);
  module.set_input_to_analog(1, false);
  module.enable_clock_events(0);         // treat input 0 as a clock signal (optional)
  module.enable_clock_events_2(1);       // treat input 1 as second clock signal (optional)
  module.set_output_to_analog(0, true);  // send output 0 to DAC
  module.set_output_to_analog(1, true);  // send output 1 to DAC
  module.set_debug(false);               // toggle debug
  module.start();                        // required to initialise pins
}

// RUNS EVERY STEP
void loop() {
  module.step();  // runs all user-defined *_do() calls
}
*/

#include <assert.h>
#include "backend/backend.h"
#include "chips/mcp4822.h"
#include "modules/Timer.h"

class EuroStep {

public:

  // incoming values
  int input_values[NUMBER_OF_INPUTS];      // the input as averaged over 8 readings
  int input_history[NUMBER_OF_INPUTS][8];  // the last 8 readings
  int pot_values[NUMBER_OF_POTS];
  bool switch_values[NUMBER_OF_SWITCHES];

  // outgoing values
  int output_values_old[NUMBER_OF_OUTPUTS];
  int output_values[NUMBER_OF_OUTPUTS];
  bool dac_code[16];  // used if output_mode_is_analog
  int dac_event;      // helps track which DAC to write

  // options
  bool debug = true;
  int input_is_true_threshold = 500;
  bool input_mode_is_analog[NUMBER_OF_INPUTS] = { 0 };
  bool output_mode_is_analog[NUMBER_OF_OUTPUTS] = { 0 };

  ///////////////////////////////////////////////////////////////////////////////
  /// Hardware variables
  ///////////////////////////////////////////////////////////////////////////////

  int pins_input[NUMBER_OF_INPUTS] = PINS_INPUT;
  int pins_pot[NUMBER_OF_POTS] = PINS_POT;
  int pins_switch[NUMBER_OF_SWITCHES] = PINS_SWITCH;
  int pins_output[NUMBER_OF_OUTPUTS] = PINS_OUTPUT;
  int pins_dac_a[4] = PINS_DAC_A;
  int pins_dac_b[4] = PINS_DAC_B;

  ///////////////////////////////////////////////////////////////////////////////
  /// Getters and setters
  ///////////////////////////////////////////////////////////////////////////////

  // incoming values
  int get_input(int index) {
    return input_values[index];
  }
  int get_pot(int index) {
    return pot_values[index];
  }
  int get_switch(int index) {
    return switch_values[index];
  }

  // outgoing values
  void set_output(int index, int value) {
    output_values[index] = value;
  }
  void send_to_output(int index, int value) {
    output_values[index] = value;
  }

  // options
  void set_input_to_analog(int index, bool value = true) {
    input_mode_is_analog[index] = value;
  }
  void set_output_to_analog(int index, bool value = true) {
    output_mode_is_analog[index] = value;
  }
  void set_debug(bool value = true) {
    debug = value;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// These functions are intended to be written by the derived program
  ///////////////////////////////////////////////////////////////////////////////

  virtual void on_start_do() {}
  virtual void on_clock_rise_do() {}
  virtual void on_clock_fall_do() {}
  virtual void on_clock_rise_2_do() {}
  virtual void on_clock_fall_2_do() {}
  virtual void on_step_do() {}

  ///////////////////////////////////////////////////////////////////////////////
  /// This is run at start up to initialise pins based on hardware
  ///////////////////////////////////////////////////////////////////////////////

  void initialise_pins() {

    if (debug) Serial.begin(9600);

    // initialise  inputs
    for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
      pinMode(pins_input[i], INPUT);
    }
    for (int i = 0; i < NUMBER_OF_POTS; i++) {
      pinMode(pins_pot[i], INPUT);
    }
    for (int i = 0; i < NUMBER_OF_SWITCHES; i++) {
      pinMode(pins_switch[i], INPUT_PULLUP);
    }

    // initialise outputs
    for (int i = 0; i < NUMBER_OF_OUTPUTS; i++) {
      pinMode(pins_output[i], OUTPUT);
    }
    for (int i = 0; i < 4; i++) {
      if (pins_dac_a[i] != -1) {
        pinMode(pins_dac_a[i], OUTPUT);
      }
      if (pins_dac_b[i] != -1) {
        pinMode(pins_dac_b[i], OUTPUT);
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// This is a generic way to read audio jack inputs as analog or digital
  ///////////////////////////////////////////////////////////////////////////////

  void read_inputs() {

    for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
      if (input_mode_is_analog[i]) {
        input_values[i] = read_analog_mV_smooth(
          pins_input[i],
          input_history[i],
          V_DIVIDER_R1,
          V_DIVIDER_R2,
          debug);
      } else {
        input_values[i] = read_analog_bool(
          pins_input[i],
          input_is_true_threshold,
          V_DIVIDER_R1,
          V_DIVIDER_R2,
          debug);
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// This is a generic way to read pots as a percent value
  ///////////////////////////////////////////////////////////////////////////////

  void read_pots() {

    for (int i = 0; i < NUMBER_OF_POTS; i++) {
      pot_values[i] = read_analog_pct(
        pins_pot[i],
        MAX_POT_VOLTAGE,
        REVERSE_POT,
        0,
        0,
        debug);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// This is a generic way to read switches as bools
  ///////////////////////////////////////////////////////////////////////////////

  void read_switches() {

    for (int i = 0; i < NUMBER_OF_SWITCHES; i++) {
      switch_values[i] = digitalRead(pins_switch[i]);
    }

    if (debug) {
      for (int i = 0; i < NUMBER_OF_SWITCHES; i++) {
        Serial.print(switch_values[i]);
      }
      Serial.println("");
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Provide helper functions for clock rise and clock fall
  ///////////////////////////////////////////////////////////////////////////////

  // in order to run events on clock rise or fall, we need to know
  //  what input to track as the clock signal
  int clock_input = -1;  // -1 disables clock events
  void enable_clock_events(int index) {
    clock_input = index;
  }

  bool current_state_is_high;
  bool last_state_is_low = true;
  void run_clock_events() {

    if (clock_input > -1) {  // by default program assumes no clock exists

      current_state_is_high = input_values[clock_input];  // for legibility

      // only run once per clock cycle
      // run on rising edge of incoming clock signal
      if (current_state_is_high & last_state_is_low) {
        if (debug) Serial.println("Clock has risen.");
        on_clock_rise_do();
        last_state_is_low = false;
      }

      // only run once per clock cycle
      // run on falling edge of incoming clock signal
      if (!current_state_is_high & !last_state_is_low) {
        if (debug) Serial.println("Clock has fallen.");
        on_clock_fall_do();
        last_state_is_low = true;
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Provide helper functions for clock rise and clock fall
  ///////////////////////////////////////////////////////////////////////////////

  // allow for a second clock
  int clock_input_2 = -1;  // -1 disables clock events
  void enable_clock_events_2(int index) {
    clock_input_2 = index;
  }

  bool current_state_is_high_2;
  bool last_state_is_low_2 = true;
  void run_clock_events_2() {

    if (clock_input_2 > -1) {  // by default program assumes no clock exists

      current_state_is_high_2 = input_values[clock_input_2];  // for legibility

      // only run once per clock cycle
      // run on rising edge of incoming clock signal
      if (current_state_is_high_2 & last_state_is_low_2) {
        if (debug) Serial.println("Clock 2 has risen.");
        on_clock_rise_2_do();
        last_state_is_low_2 = false;
      }

      // only run once per clock cycle
      // run on falling edge of incoming clock signal
      if (!current_state_is_high_2 & !last_state_is_low_2) {
        if (debug) Serial.println("Clock 2 has fallen.");
        on_clock_fall_2_do();
        last_state_is_low_2 = true;
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// This is a generic way to write digital outputs or use MCP4822 DAC
  ///////////////////////////////////////////////////////////////////////////////

  void write_outputs() {

    dac_event = 0;
    for (int i = 0; i < NUMBER_OF_OUTPUTS; i++) {

      if (output_mode_is_analog[i]) {

        // the program cannot currently handle more than 4 analog
        //  outputs since only 2x DACs are set up in memory
        dac_event++;
        assert(dac_event <= 4);

        if (output_values[i] != output_values_old[i]) {

          if (debug) {
            Serial.print("Writing analog output ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(output_values[i]);
            Serial.println("");
          }

          if (dac_event == 1 | dac_event == 3) {  // odd dac events are run on channel A
            update_dac_code(dac_code,
                            output_values[i],  // what to write
                            false,             // whether dac channel is odd or even
                            debug);
          } else {  // even dac events are run on channel B
            update_dac_code(dac_code,
                            output_values[i],  // what to write
                            true,              // whether dac channel is odd or even
                            debug);
          }

          if (dac_event == 1 | dac_event == 2) {  // the first two dac events run on dac A
            write_dac_code(dac_code,
                           pins_dac_a[0],  // PIN_CS,
                           pins_dac_a[1],  // PIN_SCK,
                           pins_dac_a[2],  // PIN_SDI,
                           pins_dac_a[3]   // PIN_LDAC
            );
          } else {  // the second two dac events run on dac B
            write_dac_code(dac_code,
                           pins_dac_b[0],  // PIN_CS,
                           pins_dac_b[1],  // PIN_SCK,
                           pins_dac_b[2],  // PIN_SDI,
                           pins_dac_b[3]   // PIN_LDAC
            );
          }
        }

      } else {  // if output mode is digital

        if (debug) {
          Serial.print("Writing digital output ");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(output_values[i]);
          Serial.println("");
        }

        if (output_values[i] != output_values_old[i]) {
          digitalWrite(pins_output[i], output_values[i]);
        }
      }

      // whether output mode is analog or digital,
      //  track history to avoid state changes when unnecessary
      output_values_old[i] = output_values[i];
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// A single timer is included for backwards compatibility
  ///////////////////////////////////////////////////////////////////////////////

  Timer timer;

  reset_timer() {
    timer.reset_timer();
  }

  get_timer() {
    timer.get_timer();
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// This runs through all sub-routines
  ///////////////////////////////////////////////////////////////////////////////

  void start() {
    initialise_pins();
    on_start_do();
  }

  void step() {
    read_inputs();
    read_pots();
    read_switches();
    run_clock_events();
    run_clock_events_2();
    on_step_do();
    write_outputs();
    if (debug) delay(250);
  }
};
