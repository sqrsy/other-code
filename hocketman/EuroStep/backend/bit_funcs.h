/**
 * Extracts a specific bit from a byte at the given position.
 *
 * This function retrieves the bit value (0 or 1) from a character (byte) at the 
 * specified position. The position is counted from 0 (least significant bit) to 
 * 7 (most significant bit).
 *
 * @param byte The character (8-bit integer) from which to extract the bit.
 * @param pos The bit position to extract, where 0 represents the least 
 * significant bit and 7 represents the most significant bit.
 * @return The bit value at the specified position, either 0 or 1.
 */
int get_bit(char byte, int pos) {
  int result = byte >> pos & 1;
  return (result);
}
