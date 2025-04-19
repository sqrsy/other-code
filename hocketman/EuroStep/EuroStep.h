/*
Class Name: EuroStep

Purpose: Manage events that get called often by synthesis programs.

Dependencies: Hardware specific pin configuration. See 'hardware/' for examples.

Use: Create a sub-class for each new program, then over-ride any of the virtual functions:
-- on_start_do(): routines called once when program starts
-- on_clock_rise_do(): routines called whenever clock rises
-- on_clock_fall_do(): routines called whenever clock falls
-- on_clock_2_rise_do(): routines called whenever the second clock rises
-- on_clock_2_fall_do(): routines called whenever the second clock falls
-- on_step_do(): routines called every step in the main program loop

Incoming Values: Any jacks, pots, or switches get read automatically during each step.
The following getters can be used by the virtual functions:
-- get_jack_value(index): returns the i-th read jack values (in mV or bool)
-- -- or can use jack_values[index]
-- get_pot_value(index): returns the i-th read pot values (in percent, from 0-100)
-- -- or can use pot_values[index]
-- get_switch_value(index): returns the i-th switch state (as a bool)
-- -- or can use switch_values[index]

Outgoing Values: Any outputs get written automatically during each step.
The following setters and can be used by the virtual functions:
-- output_value_to_dac(index, value): writes to DAC output channel as mV (range 0-4 V)
-- output_value_to_digital(index, value): writes to digital output channel via digitalWrite()

Setup Instructions: The following settings are available during setup:
-- enable_clock_as_jack(index): use input jack as a clock signal (i.e., based on 500 mV threshold)
-- enable_clock_2_as_jack(index): use input jack as a clock signal (i.e., based on 500 mV threshold)
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
  module.enable_clock_as_jack(0);         // treat input 0 as a clock signal (optional)
  module.enable_clock_2_as_jack(1);       // treat input 1 as second clock signal (optional)
  module.set_debug(false);               // toggle debug
  module.start();                        // required to initialise pins
}

// RUNS EVERY STEP
void loop() {
  module.step();  // runs all user-defined *_do() calls
}
*/

#include <assert.h>
#include "backend/power_funcs.h"
#include "backend/transfer_funcs.h"
#include "backend/bit_funcs.h"
#include "backend/map_funcs.h"
#include "backend/read_funcs.h"
#include "backend/Timer.h"
#include "backend/Input.h"
#include "backend/Mcp4822.h"

class EuroStep {

public:

  bool debug = false;

  // incoming values
  int jack_values[NUMBER_OF_JACKS];  // the input as averaged over 8 readings
  int pot_values[NUMBER_OF_POTS];
  bool switch_values[NUMBER_OF_SWITCHES];

  // input classes to manage pin read
  Input Jack[NUMBER_OF_JACKS];
  Input Pot[NUMBER_OF_POTS];
  Input Switch[NUMBER_OF_SWITCHES];

  // split output between DAC and digital streams
  int output_values_to_dac[4] = { 0 };
  bool output_values_to_digital[NUMBER_OF_DIGITAL_OUTPUTS];

  // output classes
  Mcp4822 DAC1;
  Mcp4822 DAC2;

  ///////////////////////////////////////////////////////////////////////////////
  /// Hardware variables
  ///////////////////////////////////////////////////////////////////////////////

  bool jack_is_digital[NUMBER_OF_JACKS] = JACK_IS_DIGITAL;
  int pins_jack[NUMBER_OF_JACKS] = PINS_JACK;
  int pins_pot[NUMBER_OF_POTS] = PINS_POT;
  int pins_switch[NUMBER_OF_SWITCHES] = PINS_SWITCH;
  int pins_digital_output[NUMBER_OF_DIGITAL_OUTPUTS] = PINS_DIGITAL_OUTPUT;
  int pins_dac_a[4] = PINS_DAC_A;
  int pins_dac_b[4] = PINS_DAC_B;

  ///////////////////////////////////////////////////////////////////////////////
  /// Getters and setters
  ///////////////////////////////////////////////////////////////////////////////

  // incoming values
  int get_jack_value(int index) {
    return jack_values[index];
  }
  int get_pot_value(int index) {
    return pot_values[index];
  }
  int get_switch_value(int index) {
    return switch_values[index];
  }

  // in order to run events on clock rise or fall, we need to know
  //  what input to track as the clock signal
  int clock_as_jack = -1;  // -1 disables clock events
  void enable_clock_as_jack(int jack_number) {
    clock_as_jack = jack_number;
  }

  // allow for a second clock
  int clock_2_as_jack = -1;  // -1 disables clock events
  void enable_clock_2_as_jack(int jack_number) {
    clock_2_as_jack = jack_number;
  }

  // outgoing values
  void output_value_to_dac(int index, int value) {
    output_values_to_dac[index] = value;
  }
  void output_value_to_digital(int index, int value) {
    output_values_to_digital[index] = value;
  }

  // options
  void set_debug(bool value = true) {
    debug = value;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// These functions are intended to be written by the derived program
  ///////////////////////////////////////////////////////////////////////////////

  virtual void on_start_do() {}
  virtual void on_clock_rise_do() {}
  virtual void on_clock_fall_do() {}
  virtual void on_clock_2_rise_do() {}
  virtual void on_clock_2_fall_do() {}
  virtual void on_step_do() {}

  ///////////////////////////////////////////////////////////////////////////////
  /// This is run at start up to initialise pins based on hardware
  ///////////////////////////////////////////////////////////////////////////////

  void initialise_pins() {

    if (debug) Serial.begin(9600);

    // initialise inputs
    for (int i = 0; i < NUMBER_OF_JACKS; i++) {
      if (jack_is_digital[i]) {
        Jack[i].setup_as_digital_jack(pins_jack[i]);
        Jack[i].set_debug(debug);
      } else {
        Jack[i].setup_as_analog_jack(pins_jack[i], V_DIVIDER_R1, V_DIVIDER_R2);
        Jack[i].set_debug(debug);
      }
    }
    for (int i = 0; i < NUMBER_OF_POTS; i++) {
      Pot[i].setup_as_pot(pins_pot[i]);
      Pot[i].set_max_input_mV(MAX_POT_VOLTAGE);
      Pot[i].set_reverse_input(REVERSE_POT);
      Pot[i].set_debug(debug);
    }
    for (int i = 0; i < NUMBER_OF_SWITCHES; i++) {
      Switch[i].setup_as_switch(pins_switch[i]);
      Switch[i].set_debug(debug);
    }

    // initialise digital outputs
    for (int i = 0; i < NUMBER_OF_DIGITAL_OUTPUTS; i++) {
      pinMode(pins_digital_output[i], OUTPUT);
    }

    // initial DAC 1
    if (pins_dac_a[0] != -1) {
      DAC1.set_pins(pins_dac_a[0], pins_dac_a[1], pins_dac_a[2], pins_dac_a[3]);
      DAC1.set_debug(debug);
    }

    // initial DAC 2
    if (pins_dac_b[0] != -1) {
      DAC2.set_pins(pins_dac_b[0], pins_dac_b[1], pins_dac_b[2], pins_dac_b[3]);
      DAC2.set_debug(debug);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Use the Input Class to manage all pin reads
  ///////////////////////////////////////////////////////////////////////////////

  void read_jacks() {
    for (int i = 0; i < NUMBER_OF_JACKS; i++) {
      jack_values[i] = Jack[i].get_input_as_mV();
    }
  }

  void read_pots() {
    for (int i = 0; i < NUMBER_OF_POTS; i++) {
      pot_values[i] = Pot[i].get_input_as_percent();
    }
  }

  void read_switches() {
    for (int i = 0; i < NUMBER_OF_SWITCHES; i++) {
      switch_values[i] = Switch[i].get_input_as_bool();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Run clock rise and clock fall (if enabled)
  ///////////////////////////////////////////////////////////////////////////////

  void run_clock_events() {
    if (clock_as_jack > -1) {  // by default program assumes no clock exists
      if (Jack[clock_as_jack].check_if_input_went_low_to_high()) {
        on_clock_rise_do();
      }
      if (Jack[clock_as_jack].check_if_input_went_high_to_low()) {
        on_clock_fall_do();
      }
    }
  }

  void run_clock_2_events() {
    if (clock_2_as_jack > -1) {  // by default program assumes no clock exists
      if (Jack[clock_2_as_jack].check_if_input_went_low_to_high()) {
        on_clock_2_rise_do();
      }
      if (Jack[clock_2_as_jack].check_if_input_went_high_to_low()) {
        on_clock_2_fall_do();
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// This is a generic way to write digital outputs or use MCP4822 DAC
  ///////////////////////////////////////////////////////////////////////////////

  void write_outputs() {

    // write analog output
    DAC1.send_to_channel_A(output_values_to_dac[0]);
    DAC1.send_to_channel_B(output_values_to_dac[1]);
    DAC2.send_to_channel_A(output_values_to_dac[2]);
    DAC2.send_to_channel_B(output_values_to_dac[3]);

    for (int i = 0; i < NUMBER_OF_DIGITAL_OUTPUTS; i++) {
      if (debug) {
        Serial.print("Writing digital output ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(output_values_to_digital[i]);
        Serial.println("");
      }
      digitalWrite(pins_digital_output[i], output_values_to_digital[i]);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// This runs through all sub-routines
  ///////////////////////////////////////////////////////////////////////////////

  void start() {
    initialise_pins();
    on_start_do();
  }

  void step() {
    read_jacks();
    read_pots();
    read_switches();
    run_clock_events();
    run_clock_2_events();
    on_step_do();
    write_outputs();
    if (debug) delay(250);
  }
};
