/**
 * Clamps an integer value to ensure it falls within a specified range [min, max].
 *
 * This function checks if the provided value is outside the range and adjusts it 
 * to fit within the range. If the value is less than min, it returns min. If 
 * the value is greater than max, it returns max. Otherwise, it returns the 
 * original value.
 *
 * @param value The integer value to be clamped.
 * @param min The minimum integer value of the desired range.
 * @param max The maximum integer value of the desired range.
 * @return An integer between min and max, adjusted as necessary.
 */
int transfer_value_to_range(int value, int min, int max) {
  int new_val;
  if (value < min) {
    new_val = min;
  } else if (value > max) {
    new_val = max;
  } else {
    new_val = value;
  }
  return (new_val);
}
