// terms used to make it easy to set registers
#define YM2149_MIXER 7
#define YM2149_VOLUME 8
#define YM2149_TONE_COARSE 1
#define YM2149_TONE_FINE 0
#define YM2149_ENV_SHAPE 13
#define YM2149_ENV_COARSE 12
#define YM2149_ENV_FINE 11

class YM2149 {

public:

  bool verbose = true;
  int data_pin[8] = { PIN_DA0, PIN_DA1, PIN_DA2, PIN_DA3, PIN_DA4, PIN_DA5, PIN_DA6, PIN_DA7 };

  ///////////////////////////////////////////////////////////////////////////////
  /// FUNCTIONS TO CONFIGURE BUS CONTROL AND IN/OUT
  ///////////////////////////////////////////////////////////////////////////////

  void config_000() {
    digitalWrite(PIN_BDIR, 0);
    digitalWrite(PIN_BC2, 0);
    digitalWrite(PIN_BC1, 0);
  }

  void config_010() {
    digitalWrite(PIN_BDIR, 0);
    digitalWrite(PIN_BC2, 1);
    digitalWrite(PIN_BC1, 0);
  }

  void config_latch() {
    digitalWrite(PIN_BDIR, 0);
    digitalWrite(PIN_BC2, 0);
    digitalWrite(PIN_BC1, 1);
  }

  void config_write() {
    digitalWrite(PIN_BDIR, 1);
    digitalWrite(PIN_BC2, 1);
    digitalWrite(PIN_BC1, 0);
  }

  void config_read() {
    digitalWrite(PIN_BDIR, 0);
    digitalWrite(PIN_BC2, 1);
    digitalWrite(PIN_BC1, 1);
  }

  void config_DA_as_output() {
    for (int i = 0; i < 8; i++) {
      pinMode(data_pin[i], OUTPUT);
    }
  }

  void config_DA_as_input() {
    for (int i = 0; i < 8; i++) {
      pinMode(data_pin[i], INPUT);
    }
  }

  ///////////////////////////////////////////////////////////////////////////////
  /// FUNCTIONS TO LACTH, WRITE, AND READ
  ///////////////////////////////////////////////////////////////////////////////

  void set_byte(char byte) {

    if (verbose) {
      Serial.print("Setting byte: ");
      for (int i = 7; i > -1; i--) {  // print from i=7 (DA7) then descend1
        Serial.print(get_bit(byte, i));
      }
      Serial.println("");
    }

    for (int i = 0; i < 8; i++) {
      digitalWrite(data_pin[i], get_bit(byte, i));
    }
  }

  void latch(char byte) {

    // setup bus to latch
    config_000();

    // write the register byte
    config_DA_as_output();
    set_byte(byte);
    config_latch();

    // clean up
    config_000();
  }

  void set_reg_to_val(char reg, char val) {

    // setup bus to write
    latch(reg);
    config_010();

    // write the byte (now that register is latched)
    config_DA_as_output();
    set_byte(val);
    config_write();

    // clean up
    config_010();
    config_000();
  }

  int read(char reg) {

    // setup bus to read
    latch(reg);
    config_010();

    // read the byte
    config_DA_as_input();
    config_read();

    // optional: print byte to console
    if (verbose) {
      Serial.print("Reading byte: ");
      for (int i = 7; i > -1; i--) {  // print from i=7 (DA7) then descend
        Serial.print(digitalRead(data_pin[i]));
      }
      Serial.println("");
    }

    // read byte as integer using bitwise logic
    int result = 0;
    for (int i = 0; i < 8; i++) {
      int value = digitalRead(data_pin[i]);
      result |= (value << i);  // i=0 (DA0) goes in first place, i=1 (DA1) in second place, and so on
    }

    // clean up
    config_010();
    config_000();
    return (result);
  }
};
