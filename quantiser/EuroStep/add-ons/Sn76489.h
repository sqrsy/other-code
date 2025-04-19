class Sn76489 {

private:

  // used to write to chip
  // by default, pins are pulled from PINS_SN76489 constant
  // can over-ride using function `set_new_pin_array`
  int pin_array[10] = PINS_SN76489;
  int pin_we;
  int pin_ce;
  int pin_d0;
  int pin_d1;
  int pin_d2;
  int pin_d3;
  int pin_d4;
  int pin_d5;
  int pin_d6;
  int pin_d7;

  // second byte used only for tone
  bool dac_code[16] = { false };
  bool use_byte_2 = true;

  bool debug = false;
  long master_clock_frequency = 8000000;
  int current_channel = 0;
  int previous_channel = 0;
  int previous_volume = 0;
  int previous_tone = 0;

  // configure the volume / tone register
  int volume_register_size = 4;
  int volume_register_position[4] = { 7, 6, 5, 4 };
  int tone_register_size = 10;
  int tone_register_position[10] = { 7, 6, 5, 4, 15, 14, 13, 12, 11, 10 };

  void enable_byte_2(bool value) {
    use_byte_2 = value;
    dac_code[8] = 0;
    dac_code[9] = 0;
  }

  void use_channel_1() {
    previous_channel = current_channel;
    current_channel = 1;
    dac_code[0] = 1;
    dac_code[1] = 0;
    dac_code[2] = 0;
  }

  void use_channel_2() {
    previous_channel = current_channel;
    current_channel = 2;
    dac_code[0] = 1;
    dac_code[1] = 0;
    dac_code[2] = 1;
  }

  void use_channel_3() {
    previous_channel = current_channel;
    current_channel = 3;
    dac_code[0] = 1;
    dac_code[1] = 1;
    dac_code[2] = 0;
  }

  void use_channel_noise() {
    previous_channel = current_channel;
    current_channel = 4;
    dac_code[0] = 1;
    dac_code[1] = 1;
    dac_code[2] = 1;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Backend: Write to Chip
  ///////////////////////////////////////////////////////////////////////////////

  void write_to_chip() {
    if (debug) {
      Serial.println("Writing byte 1...");
      Serial.print("pin_ce set to: ");
      Serial.println(pin_ce);
    }
    digitalWrite(pin_ce, 0);
    digitalWrite(pin_d0, dac_code[7]);
    digitalWrite(pin_d1, dac_code[6]);
    digitalWrite(pin_d2, dac_code[5]);
    digitalWrite(pin_d3, dac_code[4]);
    digitalWrite(pin_d4, dac_code[3]);
    digitalWrite(pin_d5, dac_code[2]);
    digitalWrite(pin_d6, dac_code[1]);
    digitalWrite(pin_d7, dac_code[0]);
    digitalWrite(pin_we, 0);
    delay(1);
    digitalWrite(pin_ce, 1);
    digitalWrite(pin_we, 1);
    if (use_byte_2) {
      if (debug) {
        Serial.println("Writing byte 2...");
      }
      digitalWrite(pin_ce, 0);
      digitalWrite(pin_d0, dac_code[15]);
      digitalWrite(pin_d1, dac_code[14]);
      digitalWrite(pin_d2, dac_code[13]);
      digitalWrite(pin_d3, dac_code[12]);
      digitalWrite(pin_d4, dac_code[11]);
      digitalWrite(pin_d5, dac_code[10]);
      digitalWrite(pin_d6, dac_code[9]);
      digitalWrite(pin_d7, dac_code[8]);
      digitalWrite(pin_we, 0);
      delay(1);
      digitalWrite(pin_ce, 1);
      digitalWrite(pin_we, 1);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Backend: Write Volume
  ///////////////////////////////////////////////////////////////////////////////

  void map_volume_to_dac_code(int new_volume) {
    enable_byte_2(false);
    dac_code[3] = 1;  // 1**1 for volume
    long residual_value = new_volume;
    for (int i = volume_register_size - 1; i > -1; i--) {
      int base = power_int(2, i);
      int index = volume_register_position[i];
      dac_code[index] = residual_value / base;
      residual_value = residual_value % base;
    }

    if (debug) {
      Serial.print("The current SN code is: ");
      Serial.println("");
      for (int i = 0; i < 16; i++) {
        Serial.print(dac_code[i]);
      }
      Serial.println("");
    }
  }

  void write_volume(int new_volume) {
    new_volume = transfer_value_to_range(new_volume, 0, 31);
    new_volume = 31 - new_volume;  // invert volume amount to attenuation amount
    if (new_volume != previous_volume | current_channel != previous_channel) {
      map_volume_to_dac_code(new_volume);
      write_to_chip();
      previous_volume = new_volume;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Backend: Write Tone
  ///////////////////////////////////////////////////////////////////////////////

  void map_tone_to_dac_code(int new_tone) {
    enable_byte_2(true);
    dac_code[3] = 0;                                                   // 1**0 for tone
    long residual_value = master_clock_frequency / (32.0 * new_tone);  // 8 Mhz clock divide by 32*Hz
    Serial.print("Based on the master clock, the tone value is: ");
    Serial.println(residual_value);
    for (int i = tone_register_size - 1; i > -1; i--) {
      int base = power_int(2, i);
      int index = tone_register_position[i];
      dac_code[index] = residual_value / base;
      residual_value = residual_value % base;
    }

    if (debug) {
      Serial.print("The current SN code is: ");
      Serial.println("");
      for (int i = 0; i < 16; i++) {
        Serial.print(dac_code[i]);
      }
      Serial.println("");
    }
  }

  void write_tone(int new_tone) {
    if (new_tone != previous_tone | current_channel != previous_channel) {
      map_tone_to_dac_code(new_tone);
      write_to_chip();
      previous_tone = new_tone;
    }
  }


public:

  ///////////////////////////////////////////////////////////////////////////////
  /// Pin Management / Setup
  ///////////////////////////////////////////////////////////////////////////////

  void set_master_clock(int value) {
    master_clock_frequency = value;
  }

  void set_new_pin_array(int new_pin_array[10]) {
    for (int i = 0; i < 10; i++) {
      pin_array[i] = new_pin_array[i];
    }
  }

  void initialise_pins() {
    // LUT0 is the D input, which is connected to the flipflop output
    CCL.LUT0CTRLB = CCL_INSEL0_FEEDBACK_gc;  // Input from sequencer
    CCL.TRUTH0 = 1;                          // Invert the input
    CCL.SEQCTRL0 = CCL_SEQSEL0_DFF_gc;       // D flipflop
    // The following line configures using the system clock, OUTEN, and ENABLE
    CCL.LUT0CTRLA = CCL_OUTEN_bm | CCL_ENABLE_bm;
    // LUT1 is the D flipflop G input, which is always high
    CCL.TRUTH1 = 0xff;
    CCL.LUT1CTRLA = CCL_ENABLE_bm;  // Turn on LUT1
    CCL.CTRLA = 1;                  // Enable the CCL
    pin_we = pin_array[0];
    pin_ce = pin_array[1];
    pin_d0 = pin_array[2];
    pin_d1 = pin_array[3];
    pin_d2 = pin_array[4];
    pin_d3 = pin_array[5];
    pin_d4 = pin_array[6];
    pin_d5 = pin_array[7];
    pin_d6 = pin_array[8];
    pin_d7 = pin_array[9];
    pinMode(pin_we, OUTPUT);
    pinMode(pin_ce, OUTPUT);
    pinMode(pin_d0, OUTPUT);
    pinMode(pin_d1, OUTPUT);
    pinMode(pin_d2, OUTPUT);
    pinMode(pin_d3, OUTPUT);
    pinMode(pin_d4, OUTPUT);
    pinMode(pin_d5, OUTPUT);
    pinMode(pin_d6, OUTPUT);
    pinMode(pin_d7, OUTPUT);
    turn_off_channel_1();
    turn_off_channel_2();
    turn_off_channel_3();
    turn_off_channel_noise();
  }

  void set_debug(bool value) {
    debug = value;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Volume Front-end
  ///////////////////////////////////////////////////////////////////////////////

  void send_volume_to_channel_1(int value) {
    use_channel_1();
    write_volume(value);
  }

  void send_volume_to_channel_2(int value) {
    use_channel_2();
    write_volume(value);
  }

  void send_volume_to_channel_3(int value) {
    use_channel_3();
    write_volume(value);
  }

  void send_volume_to_channel_noise(int value) {
    use_channel_noise();
    write_volume(value);
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Tone Front-end
  ///////////////////////////////////////////////////////////////////////////////

  void send_tone_to_channel_1(int value) {
    use_channel_1();
    write_tone(value);
  }

  void send_tone_to_channel_2(int value) {
    use_channel_2();
    write_tone(value);
  }

  void send_tone_to_channel_3(int value) {
    use_channel_3();
    write_tone(value);
  }

  void send_tone_to_channel_noise(int value) {
    use_channel_noise();
    write_tone(value);
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Other Wrappers
  ///////////////////////////////////////////////////////////////////////////////

  void turn_off_channel_1() {
    send_volume_to_channel_1(0);
  }

  void turn_off_channel_2() {
    send_volume_to_channel_2(0);
  }

  void turn_off_channel_3() {
    send_volume_to_channel_3(0);
  }

  void turn_off_channel_noise() {
    send_volume_to_channel_noise(0);
  }
};
