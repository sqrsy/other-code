#include "EuroStep/hardware/hocketman.h"
#include "EuroStep/EuroStep.h"

#define DEBUG false

///////////////////////////////////////////////////////////////////////////////
/// Configure sensitivity of module to CV source
///////////////////////////////////////////////////////////////////////////////

#define HARDWARE_CALIBRATION 20              // my Arturia KeyStep 37 is off by ~1 note
#define REPEAT_ANALOG_READ_N_TIMES 8         // how many times to check input each loop
#define REPEAT_HOCKET_CALCULATION_N_TIMES 4  // how many times to check next hocket direction before changing

///////////////////////////////////////////////////////////////////////////////
/// Smooth out hocket readings by tracking history
///////////////////////////////////////////////////////////////////////////////

bool hocket_history[REPEAT_HOCKET_CALCULATION_N_TIMES] = { false };
int update_hocket_history_sum(bool new_value) {

  // update history
  int history_length = REPEAT_HOCKET_CALCULATION_N_TIMES - 1;
  for (int i = 0; i < history_length; i++) {  // move history back one step
    hocket_history[i] = hocket_history[i + 1];
  }
  hocket_history[history_length] = new_value;

  // return sum
  int hocket_history_sum = 0;
  for (int i = 0; i < REPEAT_HOCKET_CALCULATION_N_TIMES; i++) {
    hocket_history_sum += hocket_history[i];
  }
  return hocket_history_sum;
}

///////////////////////////////////////////////////////////////////////////////
/// Actual module
///////////////////////////////////////////////////////////////////////////////

class make_hocket : public EuroStep::EuroStep {
public:

  bool ready_to_write_out_hocket = false;

  // when clock rises:
  // -- break hocket history (force module to wait for CV to stabilise before sending)
  // -- turn gate ON
  void on_clock_rise_do() {
    // break history:
    hocket_history[0] = 0;
    hocket_history[1] = 1;
    gate_in = true;
  }

  // when clock falls:
  // -- turn all gates OFF
  void on_clock_fall_do() {
    // force all gates off
    gate_in = false;
    gate_out_A = 0;
    gate_out_B = 0;
  }

  bool hocket_note_right[12] = { false };
  int hocket_history_sum;
  bool next_hocket_direction;
  bool current_direction_is_right = false;
  int note_to_hocket;

  long cv_in;
  bool gate_in;

  long cv_out_A;
  bool gate_out_A;
  long cv_out_B;
  bool gate_out_B;

  void on_step_do() {

    // refresh input read before making any decision
    ready_to_write_out_hocket = false;
    for (int i = 0; i < REPEAT_ANALOG_READ_N_TIMES; i++) {
      cv_in = Jack[1].get_input_immediately();
    }

    //gate_in = Jack[0].get_input_as_bool();
    cv_in = jack_values[1] - jack_values[1] / 1000 * HARDWARE_CALIBRATION;

    // use switches to decide whether to hocket left or right
    for (int i = 0; i < 12; i++) {
      hocket_note_right[i] = switch_values[i];
    }

    // figure out note for incoming CV
    note_to_hocket = map_mV_to_note_number(cv_in) % 12;
    next_hocket_direction = hocket_note_right[note_to_hocket];

    // check if next hocket direction has stabilised
    hocket_history_sum = update_hocket_history_sum(next_hocket_direction);
    if (hocket_history_sum == REPEAT_HOCKET_CALCULATION_N_TIMES) {
      ready_to_write_out_hocket = true;
      current_direction_is_right = true;
    } else if (hocket_history_sum == 0) {
      ready_to_write_out_hocket = true;
      current_direction_is_right = false;
    } else {
      ready_to_write_out_hocket = false;
    }

    // if next hocket direction has stabilised
    // then update program to hocket note left or right
    if (ready_to_write_out_hocket) {
      if (current_direction_is_right) {
        cv_out_B = cv_in;
        gate_out_B = gate_in;
        // cv_out_A does not change
        gate_out_A = 0;
      } else {
        cv_out_A = cv_in;
        gate_out_A = gate_in;
        // cv_out_B does not change
        gate_out_B = 0;
      }
    }

    // write out
    output_value_to_dac(0, cv_out_A);
    output_value_to_digital(0, gate_out_A);
    output_value_to_dac(1, cv_out_B);
    output_value_to_digital(1, gate_out_B);
  }
};

make_hocket module;
void setup() {
  module.enable_clock_as_jack(0);  // treat input 0 as a clock signal (optional)
  module.set_debug(DEBUG);         // toggle debug
  module.start();                  // required to initialise pins
}

void loop() {
  module.step();  // runs all user-defined *_do() calls
}
