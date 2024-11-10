/*
Class Name: Timer

Purpose: Create a timer that counts up like a stop watch.

Dependencies: None.

Use: Create an instance of the class and configure settings, then run:
-- get_timer(): returns how much time has passed since reset_timer() was called
-- reset_timer(): resets the timer to 0

You may wish to configure the following settings:
-- use_millis(): use milliseconds for the timer (default)
-- use_micros(): use microseconds for the timer
*/

class Timer {

private:

  bool time_in_micros = false;
  unsigned long last_timer = 0;

public:

  void use_micros() {
    time_in_micros = true;
  }

  void use_millis() {
    time_in_micros = false;
  }

  unsigned long time_right_now() {
    if (time_in_micros) {
      return micros();
    } else {
      return millis();
    }
  }

  unsigned long get_timer() {
    if (time_right_now() < last_timer) reset_timer();  // overflow catch
    return (time_right_now() - last_timer);
  }

  void reset_timer() {
    last_timer = time_right_now();
  }
};
