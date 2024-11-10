void update_dac_code(bool data[16], long outVolt, bool channel, bool verbose = false) {

  // declare vars
  long gain = 1;

  // set ceiling and floor for voltage
  if (outVolt < 0) {
    outVolt = 0;
  }
  if (outVolt > 4096) {
    outVolt = 4096;
  }

  // set up chip options as bit code
  data[0] = channel;  // slot 0: channel A or B
  if (outVolt > 2048) {
    data[2] = 0;  // slot 2: turn on gain (~GAIN bit)
    gain = 2;     // compute volts less gain
  } else {
    data[2] = 1;
  }
  data[3] = 1;  // slot 3: turns on DAC

  // convert output voltage to DAC value
  long long_base = power_long(2, 12);
  long dacCode = long_base * outVolt / gain / 2048;
  long dacCode_old = dacCode;

  // write DAC value as bit code
  for (int i = 11; i > -1; i--) {
    int base = power_int(2, i);
    //Serial.println (base);
    //Serial.println ("-");
    int index = 11 - i + 4;
    data[index] = dacCode / base;
    dacCode = dacCode % base;
  }

  if (verbose) {
    Serial.print("The DAC value is: ");
    Serial.println(dacCode_old);

    Serial.print("The 12-bit code is: ");
    Serial.println("");
    for (int i = 0; i < 16; i++) {
      Serial.print(data[i]);
    }
    Serial.println("");
  }
}

void write_dac_code(bool data[16], int pin_cs, int pin_sck, int pin_sdi, int pin_ldac) {

  // get ready to write
  digitalWrite(pin_cs, LOW);

  // write 16 bits to DAC
  for (int i = 0; i < 16; i++) {
    if (data[i] == 1) {
      digitalWrite(pin_sdi, HIGH);
    } else {
      digitalWrite(pin_sdi, LOW);
    }
    digitalWrite(pin_sck, HIGH);
    digitalWrite(pin_sck, LOW);
  }

  // finish write
  digitalWrite(pin_cs, HIGH);
  if (pin_ldac > -1) {
    digitalWrite(pin_ldac, LOW);
    digitalWrite(pin_ldac, HIGH);
  }

  delayMicroseconds(1);  // make sure pin_cs stays high for a tiny bit
                         // this is to avoid trouble writing chan B right after chan A
}
