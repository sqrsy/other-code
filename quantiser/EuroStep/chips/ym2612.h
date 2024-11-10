class YM2612 {

public:

  bool verbose = true;
  int data_pin[8] = { PIN_DA0, PIN_DA1, PIN_DA2, PIN_DA3, PIN_DA4, PIN_DA5, PIN_DA6, PIN_DA7 };

  void set_byte(char byte) {

    // pass data to chip
    digitalWrite(PIN_CS, 0);
    for (int i = 0; i < 8; i++) {
      digitalWrite(data_pin[i], get_bit(byte, i));  // write i=0 (DA0) then i=1 (DA1) and so on
    }
    delayMicroseconds(1);

    // enable write mode
    digitalWrite(PIN_WR, 0);
    digitalWrite(PIN_RD, 1);
    delayMicroseconds(5);

    // stop write
    digitalWrite(PIN_WR, 1);
    digitalWrite(PIN_RD, 0);
    delayMicroseconds(5);

    // finish
    digitalWrite(PIN_CS, 1);
  }

  void set_reg_to_val(char reg, char val) {

    // write to register
    digitalWrite(PIN_A0, 0);
    digitalWrite(PIN_A1, 0);  // set to 1 for chan 4-6
    set_byte(reg);

    // write value
    digitalWrite(PIN_A0, 1);
    digitalWrite(PIN_A1, 0);  // set to 1 for chan 4-6
    set_byte(val);
  }
};
