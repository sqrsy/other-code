/*
Class Name: Envelope

Purpose: Create an envelope based on a number of settings.

Dependencies: This class inherits from the Timer class.

Use: Create an instance of the class and configure settings, then run:
-- advance_envelope(): advances the timer to update the current envelope size
-- get_envelope_size(): returns the current envelope size (in mV)

You will also need to trigger the envelope:
-- turn_on_gate(): triggers the attack stage and enables sustain
-- -- note: sustain may be skipped is attack and decay take too long
-- turn_off_gate(): disables sustain and allows the release stage to begin
-- -- note: if sustain stage is on, release begins immediately

You may wish to configure the following settings:
-- set_envelope_limits(min, max): the minimum and maximum values of the envelope size (in mV)
-- set_sustain_level(level): the level of the envelope during the sustain stage (in mV)
-- set_ADSR_rate(stage, value): sets the rate for each step change for A, D, and R (S has no rate)
-- -- note: measured in time units until changing envelope by 'step' percent of envelope range
-- set_ADSR_step(stage, value): sets the size of each step change for A, D, and R (S has no step)
-- -- note: measured size of change in 'step' percent of envelope range

The following settings are relevant from the Timer class:
-- use_millis(): use milliseconds as the time units for the set_ADSR_rate (default)
-- use_micros(): use microseconds as the time units for the set_ADSR_rate
*/

class Envelope : public Timer {

private:

  // gate used to trigger envelope and manage sustain
  int gate_on = false;
  bool ADSR_progress[4] = { false, false, false, false };
  bool ADSR_was_interrupted = false;  // used to handle ADSR interruption

  // min, max, and sustain level
  int min_limit = 0;
  int sustain_level;
  int max_limit = 4000;
  int delta_limit = 4000;
  int current_limit;  // which limit is active

  // the current size of envelope to send to analog out
  int current_size;

  // how fast to make each stage (in time units waited before changing envelope by 1%)
  int ADSR_rate[4] = { -1, -1, -1, -1 };

  // how much to speed up each stage (optional)
  int ADSR_step[4] = { 1, 1, 1, 1 };

public:

  ///////////////////////////////////////////////////////////////////////////////
  /// Getters and setters
  ///////////////////////////////////////////////////////////////////////////////

  int get_envelope_size() {
    return current_size;
  }

  void set_envelope_limits(int min = 0, int max = 4000) {
    min_limit = min;
    max_limit = max;
    delta_limit = max_limit - min_limit;
  }

  void set_sustain_level(int level) {
    sustain_level = level;
  }

  void set_ADSR_rate(int stage, int value) {
    ADSR_rate[stage] = value;
  }

  void set_ADSR_step(int stage, int value) {
    ADSR_step[stage] = value;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Toggle between GATE ON vs. GATE OFF -- needed for to track sustain
  ///////////////////////////////////////////////////////////////////////////////

  void turn_on_gate() {

    // if last ADSR was interrupted,
    // safely end last ADSR as part of new attack
    if (current_size > min_limit) {
      ADSR_was_interrupted = true;
    }

    gate_on = true;
    ADSR_progress[0] = true;  // attack begins
    ADSR_progress[1] = false;
    ADSR_progress[2] = false;
    ADSR_progress[3] = false;
    reset_timer();
  }

  void turn_off_gate() {
    gate_on = false;
    ADSR_progress[0] = false;
    ADSR_progress[1] = false;
    ADSR_progress[2] = false;
    ADSR_progress[3] = true;  // release begins
    reset_timer();
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Attack stage
  ///////////////////////////////////////////////////////////////////////////////

  void end_ADSR_safely() {

    // end ADSR wherever it is, quickly but not instantly
    // no need for timer, want this to happen fast!
    current_size -= 0.15 * delta_limit;

    if (current_size <= min_limit) {
      current_size = min_limit;
      ADSR_was_interrupted = false;
    }
  }

  void continue_attack_as_normal() {

    if (get_timer() > ADSR_rate[0]) {
      current_size += .01 * ADSR_step[0] * delta_limit;
      if (current_size >= max_limit) {
        current_size = max_limit;
        ADSR_progress[0] = false;
        ADSR_progress[1] = true;  // decay begins
        ADSR_progress[2] = false;
        ADSR_progress[3] = false;
      }
      reset_timer();
    }
  }

  void attack() {

    if (ADSR_was_interrupted) {
      end_ADSR_safely();
    } else {
      continue_attack_as_normal();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Decay stage
  ///////////////////////////////////////////////////////////////////////////////

  void decay() {

    if (get_timer() > ADSR_rate[1]) {
      current_size -= .01 * ADSR_step[1] * delta_limit;
      if (current_size <= sustain_level) {
        current_size = sustain_level;  // use whatever sustain is set when decay ends
        ADSR_progress[0] = false;
        ADSR_progress[1] = false;
        ADSR_progress[2] = true;  // sustain begins
        ADSR_progress[3] = false;
      }
      reset_timer();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Sustain and release stage
  ///////////////////////////////////////////////////////////////////////////////

  void sustain() {

    // do nothing
  }

  void release() {

    if (get_timer() > ADSR_rate[3]) {
      current_size -= .01 * ADSR_step[3] * delta_limit;
      if (current_size <= min_limit) {
        current_size = min_limit;
        ADSR_progress[0] = false;
        ADSR_progress[1] = false;
        ADSR_progress[2] = false;
        ADSR_progress[3] = false;  // stop everything
      }
      reset_timer();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Run the envelope program
  ///////////////////////////////////////////////////////////////////////////////

  void advance_envelope() {

    if (ADSR_progress[0]) {
      attack();
    } else if (ADSR_progress[1]) {
      decay();
    } else if (ADSR_progress[2]) {
      sustain();
    } else if (ADSR_progress[3]) {
      release();
    }
  }
};
