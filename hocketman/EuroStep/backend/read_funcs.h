/**
 * Reads an analog input pin and calculates the voltage in millivolts (mV), 
 * optionally accounting for a voltage divider.
 *
 * This function reads the analog input from the specified pin and converts 
 * it to a voltage value in millivolts. If resistor values for a voltage divider 
 * (r1 and r2) are provided, the function back-calculates the real voltage before 
 * division. If no resistor values are given, a default conversion factor of 4.9 
 * mV per unit is applied based on a 10-bit ADC (analog-to-digital converter) 
 * with a 5V reference.
 *
 * @param pin_in The analog input pin to read from.
 * @param r1 The resistance value (in ohms) of the resistor connected between 
 *           the voltage source and the analog pin. Defaults to 0 (no voltage divider).
 * @param r2 The resistance value (in ohms) of the resistor connected between 
 *           the analog pin and ground. Defaults to 0 (no voltage divider).
 * @param debug A boolean flag that, when true, prints the calculated mV value 
 *              to the serial monitor for debugging purposes. Defaults to false.
 * @return The calculated voltage in millivolts (mV).
 */
float read_analog_mV(int pin_in, int r1 = 0, int r2 = 0, bool debug = false) {

  int x = analogRead(pin_in);
  x = analogRead(pin_in);
  float mV = 0;
  if (r1 == 0 & r2 == 0) {
    mV = x * 4.9;
  } else {
    mV = x * 4.9 * (r1 + r2) / r2;
  }

  if (debug) {
    Serial.print("Current value (read_analog_mV): ");
    Serial.print(mV);
    Serial.println("");
  }

  return (mV);
}

/**
 * Reads an analog input pin, applies a smoothing filter using a history buffer, 
 * and returns the average voltage in millivolts (mV).
 *
 * This function smooths the voltage readings from the specified analog pin using 
 * an averaging filter. It maintains a history of the last 8 readings, updates the 
 * history with the latest reading, and calculates the average to reduce noise 
 * in the signal. The voltage is adjusted based on the provided resistor values 
 * if a voltage divider is used.
 *
 * @param pin_in The analog input pin to read from.
 * @param read_history An array of 8 integers to store the history of recent readings.
 *                     This array should be maintained between function calls to 
 *                     preserve the smoothing effect.
 * @param r1 The resistance value (in ohms) of the resistor connected between 
 *           the voltage source and the analog pin. Defaults to 0 (no voltage divider).
 * @param r2 The resistance value (in ohms) of the resistor connected between 
 *           the analog pin and ground. Defaults to 0 (no voltage divider).
 * @param debug A boolean flag that, when true, prints the smoothed mV value 
 *              to the serial monitor for debugging purposes. Defaults to false.
 * @return The smoothed voltage in millivolts (mV) as an integer.
 */
int read_analog_mV_smooth(int pin_in, int read_history[8], int r1 = 0, int r2 = 0, bool debug = false) {

  // track history of input
  for (int i = 0; i < 7; i++) {  // move history back one step
    read_history[i] = read_history[i + 1];
  }
  read_history[7] = read_analog_mV(pin_in, r1, r2);  // update history with new value

  // calculate average of history
  long incoming_cv = 0;
  for (int i = 0; i < 8; i++) {  // calculate grand sum of history
    incoming_cv = incoming_cv + read_history[i];
  }
  incoming_cv = incoming_cv / 8;

  if (debug) {
    Serial.print("Current value (read_analog_mV_smooth): ");
    Serial.print(incoming_cv);
    Serial.println("");
  }

  return (incoming_cv);
}
