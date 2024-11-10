/*
Class Name: Predelay

Purpose: Delay the start of another program module.

Dependencies: This class inherits from the Timer class.

Use: Create an instance of the class and configure settings, then run:
-- restart_predelay(int duration): (Re-)Starts the pre-delay for a duration (default: milliseconds).
-- advance_predelay(): Advances the Timer and checks whether the pre-delay is finished.
-- predelay_is_finished(): Returns whether the pre-delay is finished.

You may wish to configure the following settings:
-- use_millis(): use milliseconds for the timer (default)
-- use_micros(): use microseconds for the timer
*/

class Predelay : public Timer {

private:

  int predelay_duration = 1000;  // Default delay duration in milliseconds
  bool predelay_finished = false;
  bool predelay_started = false;

public:

  // (Re-)Start the predelay timer
  void restart_predelay(int duration) {
    predelay_duration = duration;
    predelay_started = true;
    predelay_finished = false;
    reset_timer();  // Reset the timer from the Timer class
  }

  // Run this function to check and update the predelay status
  void advance_predelay() {
    if (predelay_started && get_timer() >= predelay_duration) {
      predelay_started = false;
      predelay_finished = true;
    }
  }

  // Check if the predelay has finished
  bool predelay_is_finished() const {
    return predelay_finished;
  }
};
