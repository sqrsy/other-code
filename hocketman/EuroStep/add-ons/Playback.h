/*
Class Name: Playback

Purpose: To manage the playback of a WAV file saved as a C++ array object.

Dependencies: This class inherits from the Timer class.

Use: Create an instance of the class and configure settings, then run:
-- set_audio(audio_array, audio_array_length): associate Playback with another C++ array
-- run_playback(): advance the timer to update the current Playback position
-- -- note: run_playback() will not play if Playback is paused
-- get_current_value(): return the value associated with the current Playback position

You will eventually want to start the Playback:
-- restart_playback(): manually restart the Playback from the start position

You may also want other Playback options:
-- rewind_playback(): rewind Playback but do not restart
-- pause_playback(): pause the Playback
-- start_playback(): unpause the Playback
-- loop_playback(): begin looping the Playback
-- unloop_playback(): stop looping the Playback

You may wish to configure the following settings:
-- set_playback_rate(value): set the rate for each step change for Playback
-- set_playback_step(value): set the size of each step change for Playback
-- set_start_position(value): change the start position for Playback

You can also play wavetables at a specific pitch:
-- set_playback_rate_from_Hz(Hz, values_per_cycle): for pitch in Hz

The following settings are relevant from the Timer class:
-- use_millis(): use milliseconds as the time units for the set_ADSR_rate (default)
-- use_micros(): use microseconds as the time units for the set_ADSR_rate
*/

class Playback : public Timer {

private:

  // how fast to playback array (in ms until advancing current position)
  int playback_rate = 100;

  // how much to speed up playback (optional)
  int playback_step = 1;

  // load a reference "file" to sample
  byte* audio;
  int audio_length = 0;

  // the position along the playback reference file
  int current_value = 0;
  int current_position = 0;
  int start_position = 0;

  // other args
  bool now_restarting_safely = false;  // protect from interruption
  int safe_restart_increment = 16;
  bool pause = false;
  bool loop = false;

  void loop_now() {
    rewind_playback();
    unpause_playback();
  }

public:

  ///////////////////////////////////////////////////////////////////////////////
  /// Getters and setters
  ///////////////////////////////////////////////////////////////////////////////

  void set_playback_rate(int value) {
    playback_rate = value;
  }

  // alternative playback rate to play wavetables at a specific pitch
  void set_playback_rate_from_Hz(int Hz, int values_per_cycle) {
    use_micros();  // force use micro-seconds
    playback_rate = map_Hz_to_micros(Hz) / values_per_cycle;
  }

  void set_playback_step(int value) {
    playback_step = value;
  }

  void set_audio(byte* audio_array, int audio_array_length) {
    audio = audio_array;  // just point to new file, which will already live in RAM
    audio_length = audio_array_length;
    safe_restart_increment = 16;  // tested for sample values 0-255
  }

  void set_start_position(int value) {
    start_position = value;
  }

  int get_current_value() {
    return current_value;
  }

  bool is_restarting_safely() {
    return now_restarting_safely;
  }

  ~Playback() {
    delete[] audio;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Playback functions
  ///////////////////////////////////////////////////////////////////////////////

  void pause_playback(bool new_value = true) {
    pause = new_value;
  }

  void unpause_playback() {
    pause = false;
  }

  void loop_playback(bool new_value = true) {
    loop = new_value;
  }

  void unloop_playback() {
    loop = false;
  }

  void rewind_playback() {
    pause_playback();
    current_position = start_position;
    reset_timer();
  }

  void restart_playback() {
    rewind_playback();
    unpause_playback();
    now_restarting_safely = true;
  }

  void stop_playback_safely() {
    // end Playback wherever it is, quickly but not instantly
    // no need for timer, want this to happen fast!
    if (current_value >= 0) {  // if current value is positive, decrease it
      current_value -= safe_restart_increment;
      if (current_value <= 0) {  // if close to zero
        current_value = 0;
        now_restarting_safely = false;
      }
    } else {  // if current value is negative, increase it
      current_value += safe_restart_increment;
      if (current_value >= 0) {  // if close to zero
        current_value = 0;
        now_restarting_safely = false;
      }
    }
  }

  void continue_playback() {
    if (get_timer() > playback_rate) {
      current_value = audio[current_position];
      current_position += 1;  // increment after
      reset_timer();
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Run the playback program
  ///////////////////////////////////////////////////////////////////////////////

  void run_playback() {

    // update start position if it is changed mid-stream
    if (current_position < start_position) {
      current_position = start_position;
    }

    if (!pause) {
      if (now_restarting_safely) {
        stop_playback_safely();
      } else {
        if (current_position >= audio_length) {  // prevent overflow
          if (loop) {
            loop_now();
          } else {
            rewind_playback();
          }
        } else {
          continue_playback();  // remember audio[audio_length] is an overflow!
        }
      }
    }
  }
};
