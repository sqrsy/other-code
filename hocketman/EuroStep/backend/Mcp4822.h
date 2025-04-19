/*
Class Name: Mcp4822

Purpose: Send output to DAC.

Dependencies: None.

Use: Create an instance of the class and then configure the settings:
-- set_pins(int cs, int sck, int sdi, int ldac): Sets the control pins for the DAC.
-- set_debug(bool value): Enables or disables debug mode for additional logging.

You actually write to the DAC via:
-- send_to_channel_A(int mV_out): Sends a specified voltage (mV) to channel A of the DAC.
-- send_to_channel_B(int mV_out): Sends a specified voltage (mV) to channel B of the DAC.
*/

class Mcp4822 {

private:

  bool debug = false;

  // pins used to write output
  int pin_cs = -1;
  int pin_sck = -1;
  int pin_sdi = -1;
  int pin_ldac = -1;

  // bit instructions
  bool dac_code[16] = { 0 };
  int gain = 1;

  // keep history
  int last_mV_out = 0;

  ///////////////////////////////////////////////////////////////////////////////
  /// Interpret Chip Register
  ///////////////////////////////////////////////////////////////////////////////

  void enable_channel_A() {
    dac_code[0] = 0;
  }

  void enable_channel_B() {
    dac_code[0] = 1;
  }

  void enable_gain() {
    dac_code[2] = 0;
    gain = 2;
  }

  void disable_gain() {
    dac_code[2] = 1;
    gain = 1;
  }

  void turn_on_dac() {
    dac_code[3] = 1;
  }

  void map_dac_value_to_dac_code(int value) {
    int remainder = value;
    for (int i = 11; i > -1; i--) {                   // from 2^11 down to 2^0
      int index = 11 - i + 4;                         // map 2^i to position in DAC register
      dac_code[index] = remainder / power_int(2, i);  // set true if remainder equals 2^i
      remainder = remainder % power_int(2, i);        // get remainder from 2^i
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Update DAC code
  ///////////////////////////////////////////////////////////////////////////////

  void update_dac_code(int mV_out, bool use_channel_B) {

    // Step 1: turn on DAC
    turn_on_dac();

    // Step 2: select channel
    if (use_channel_B) {
      enable_channel_B();
    } else {
      enable_channel_A();
    }

    // Step 3: determine gain
    if (mV_out > 2048) {
      enable_gain();
    } else {
      disable_gain();
    }

    // Step 4: convert output voltage to DAC value
    int dac_value = 2 * mV_out / gain;
    map_dac_value_to_dac_code(dac_value);

    // Debug if needed
    if (debug) {
      Serial.print("The output voltage is: ");
      Serial.println(mV_out);
      Serial.print("The DAC value is: ");
      Serial.println(dac_value);
      Serial.print("The 16-bit code is: ");
      Serial.println("");
      for (int i = 0; i < 16; i++) {
        Serial.print(dac_code[i]);
      }
      Serial.println("");
    }
  };

  ///////////////////////////////////////////////////////////////////////////////
  /// Write DAC code
  ///////////////////////////////////////////////////////////////////////////////

  void write_dac_code() {

    // Step 1: lower CS & write 16 bits to DAC
    digitalWrite(pin_cs, LOW);
    for (int i = 0; i < 16; i++) {
      if (dac_code[i] == 1) {
        digitalWrite(pin_sdi, HIGH);
      } else {
        digitalWrite(pin_sdi, LOW);
      }
      digitalWrite(pin_sck, HIGH);
      digitalWrite(pin_sck, LOW);
    }

    // Step 3: raise CS & finish write
    // note: use delay to make sure pin_cs stays high for a tiny bit
    // this avoids trouble writing to chan B right after chan A
    digitalWrite(pin_cs, HIGH);
    if (pin_ldac > -1) {
      digitalWrite(pin_ldac, LOW);
      digitalWrite(pin_ldac, HIGH);
    }
    delayMicroseconds(1);
  };

  ///////////////////////////////////////////////////////////////////////////////
  /// Run program
  ///////////////////////////////////////////////////////////////////////////////

  void send_to_dac(int mV_out, bool use_channel_B) {
    if (debug) {
      Serial.print("Sending to DAC via CS pin: ");
      Serial.println(pin_cs);
      Serial.print("Sending value: ");
      Serial.println(mV_out);
    }
    if (pin_cs > -1) {  // pin_cs = -1 used to skip whole thing
      if (mV_out < 0) mV_out = 0;
      if (mV_out > 4095) mV_out = 4095;  // chip cannot write 4096!!
      if (mV_out != last_mV_out) {
        update_dac_code(mV_out, use_channel_B);
        write_dac_code();
        last_mV_out = mV_out;
      }
    }
  }

public:

  ///////////////////////////////////////////////////////////////////////////////
  /// Set up chip
  ///////////////////////////////////////////////////////////////////////////////

  void set_pins(int cs, int sck, int sdi, int ldac) {
    pin_cs = cs;
    pin_sck = sck;
    pin_sdi = sdi;
    pin_ldac = ldac;
    pinMode(pin_cs, OUTPUT);
    pinMode(pin_sck, OUTPUT);
    pinMode(pin_sdi, OUTPUT);
    pinMode(pin_ldac, OUTPUT);
  }

  void set_debug(bool value) {
    debug = value;
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// Send data to chip
  ///////////////////////////////////////////////////////////////////////////////

  void send_to_channel_A(int mV_out) {
    send_to_dac(mV_out, 0);
  };

  void send_to_channel_B(int mV_out) {
    send_to_dac(mV_out, 1);
  };
};
