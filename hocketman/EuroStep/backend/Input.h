/*
Class Name: Input

Purpose: Read input from pins.

Dependencies: This class inherits from the Timer class.

Use: Create an instance of the class and then configure the settings:
-- setup_as_digital_jack(int pin): Configures the input for an digital jack.
-- setup_as_analog_jack(int pin, int r1, int r2): Configures the input for an analog jack with smoothing.
-- setup_as_pot(int pin): Configures the input for a potentiometer without smoothing.
-- setup_as_switch(int pin): Configures the input for a digital switch (binary input).

You can customise settings further via:
-- set_read_frequency(int value): Sets how often the input should be read (in milliseconds).
-- set_read_frequency_offset(int value): Sets a time offset for staggered input readings.
-- set_max_input_mV(int value): Sets the maximum expected input voltage in millivolts.
-- set_reverse_input(bool value): Enables or disables reversing of the input value.
-- set_smooth_input(bool value): Toggles whether to apply moving average to input readings.
-- set_debug(bool value): Enables or disables debug mode for additional logging.

You actually get the input value via:
-- get_input_as_mV(): Returns the current input value in millivolts (mV).
-- get_input_as_percent(): Returns the current input value as a percentage of the maximum.
-- get_input_as_bool(): Returns true or false based on the input threshold.
-- check_if_input_went_low_to_high(): Detects if the input transitioned from low to high.
-- -- Used to trigger a clock rise signal.
-- check_if_input_went_high_to_low(): Detects if the input transitioned from high to low.
-- -- Used to trigger a clock fall signal.
-- get_input_immediately(): Returns the current input value immediately.
-- -- Used to clear out input history.
*/

class Input : public Timer {

private:

  bool debug = false;

  // what input to read
  int input_pin = 0;

  // how often to read input
  int read_frequency = 0;

  // store & convert read value
  int last_value_mV = 0;  // used to add "clock rise" and "clock fall" logic
  int current_value_mV_history[8] = { 0 };
  int current_value_mV = 0;
  int current_value_percent = 0;
  int round_percent_to = 1;
  int input_as_bool_threshold = 500;
  int max_input_mV = 5000;

  // used to back-calculate mV from voltage divider network
  int r1_value = 0;
  int r2_value = 0;

  // used to smooth input using moving average (i.e., for "jacks")
  bool smooth_input = false;

  // used to swap the order in which percent gets mapped (i.e., for "pots")
  bool reverse_input = false;

  // used to read digital pins instead of analog pins (i.e., for "switches")
  bool input_is_digital = false;
  bool input_from_digital_read = 0;

  int round_to_nearest(int input, int value) {
    return (input / value * value);
  }

  void read_input_immediately() {
    last_value_mV = current_value_mV;  // save last value -- used for clock signals
    if (input_is_digital) {
      current_value_mV = max_input_mV * digitalRead(input_pin);  // convert digital to mV to unify getters()
    } else {
      if (smooth_input) {
        current_value_mV = read_analog_mV_smooth(input_pin, current_value_mV_history, r1_value, r2_value, debug);
      } else {
        current_value_mV = read_analog_mV(input_pin, r1_value, r2_value, debug);
      }
    }
    current_value_mV = transfer_value_to_range(current_value_mV, 0, max_input_mV);
    if (reverse_input) current_value_mV = max_input_mV - current_value_mV;
  }

  void read_input_if_ready() {
    if (get_timer() > read_frequency) {
      read_input_immediately();
      reset_timer();
    }
  }

public:

  ///////////////////////////////////////////////////////////////////////////////
  /// Wrappers for Input setup
  ///////////////////////////////////////////////////////////////////////////////

  void setup_as_digital_jack(int pin) {
    pinMode(pin, INPUT);
    input_pin = pin;
    r1_value = 0;
    r2_value = 0;
    set_read_frequency(2);
    set_read_frequency_offset(input_pin);
    smooth_input = false;
    input_is_digital = true;
  }

  void setup_as_analog_jack(int pin, int r1, int r2) {
    pinMode(pin, INPUT);
    input_pin = pin;
    r1_value = r1;
    r2_value = r2;
    set_read_frequency(2);
    set_read_frequency_offset(input_pin);
    smooth_input = true;
    input_is_digital = false;
  }

  void setup_as_pot(int pin) {
    pinMode(pin, INPUT);
    input_pin = pin;
    r1_value = 0;
    r2_value = 0;
    set_read_frequency(10);
    set_read_frequency_offset(input_pin);
    smooth_input = false;
    input_is_digital = false;
  }

  void setup_as_switch(int pin) {
    pinMode(pin, INPUT_PULLUP);
    input_pin = pin;
    r1_value = 0;
    r2_value = 0;
    set_read_frequency(10);
    set_read_frequency_offset(input_pin);
    smooth_input = false;
    input_is_digital = true;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Customise settings further
  ///////////////////////////////////////////////////////////////////////////////

  void set_read_frequency(int value) {
    read_frequency = value;
  }

  void set_read_frequency_offset(int value) {
    offset_timer(value);
  }

  void set_max_input_mV(int value) {
    max_input_mV = value;
  }

  void set_reverse_input(bool value) {
    reverse_input = value;
  }

  void set_smooth_input(bool value) {
    smooth_input = value;
  }

  void set_debug(bool value) {
    debug = value;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Actually get the Input
  ///////////////////////////////////////////////////////////////////////////////

  int get_input_as_mV() {
    read_input_if_ready();
    return current_value_mV;
  }

  int get_input_as_percent() {
    read_input_if_ready();
    current_value_percent = map_mV_to_percent(current_value_mV, max_input_mV);
    return round_to_nearest(current_value_percent, round_percent_to);
  }

  bool get_input_as_bool() {
    read_input_if_ready();
    return current_value_mV >= input_as_bool_threshold;
  }

  bool check_if_input_went_low_to_high() {
    // read_input_if_ready(); -- don't need since read_jacks() is already run
    return (last_value_mV < input_as_bool_threshold) & (current_value_mV >= input_as_bool_threshold);
  }

  bool check_if_input_went_high_to_low() {
    // read_input_if_ready(); -- don't need since read_jacks() is already run
    return (last_value_mV >= input_as_bool_threshold) & (current_value_mV < input_as_bool_threshold);
  }

  int get_input_immediately() {
    read_input_immediately();
    return current_value_mV;
  }
};
