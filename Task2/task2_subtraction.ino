#include <avr/io.h>

void printSREG(uint8_t sreg) {
  Serial.print("  SREG = 0x");
  if (sreg < 16) Serial.print("0");
  Serial.print(sreg, HEX);
  Serial.print("  (0b");
  for (int i = 7; i >= 0; i--) Serial.print((sreg >> i) & 1);
  Serial.println(")");
  Serial.println("         I T H S V N Z C");

  bool C = (sreg >> 0) & 1;
  bool Z = (sreg >> 1) & 1;
  bool N = (sreg >> 2) & 1;
  bool V = (sreg >> 3) & 1;
  bool S = (sreg >> 4) & 1;
  bool H = (sreg >> 5) & 1;

  Serial.print("  C = "); Serial.print(C);
  Serial.println(C ? "  (borrow occurred, underflow)" : "  (no borrow)");

  Serial.print("  Z = "); Serial.print(Z);
  Serial.println(Z ? "  (result is zero)" : "  (result is not zero)");

  Serial.print("  N = "); Serial.print(N);
  Serial.println(N ? "  (bit 7 set, negative in signed view)" : "  (bit 7 clear)");

  Serial.print("  V = "); Serial.print(V);
  Serial.println(V ? "  (signed overflow)" : "  (no signed overflow)");

  Serial.print("  S = "); Serial.print(S);
  Serial.println("  (N XOR V)");

  Serial.print("  H = "); Serial.println(H);
}

void runTest(uint8_t a, uint8_t b, const char* label) {
  Serial.println("----------------------------------------");
  Serial.println(label);
  Serial.println("----------------------------------------");

  uint8_t result;
  uint8_t sreg_after;

  asm volatile (
    "mov  r16, %[valA]  \n\t"
    "mov  r17, %[valB]  \n\t"
    "sub  r16, r17      \n\t"
    "in   %[sreg], 0x3F \n\t"
    "mov  %[res], r16   \n\t"
    : [res]  "=r" (result),
      [sreg] "=r" (sreg_after)
    : [valA] "r"  (a),
      [valB] "r"  (b)
    : "r16", "r17"
  );

  Serial.print("  R16 (A) = "); Serial.print(a, DEC);
  Serial.print("  R17 (B) = "); Serial.println(b, DEC);
  Serial.print("  Result in R16 = "); Serial.print(result, DEC);
  Serial.print(" (0x");
  if (result < 16) Serial.print("0");
  Serial.print(result, HEX);
  Serial.print(", 0b");
  for (int i = 7; i >= 0; i--) Serial.print((result >> i) & 1);
  Serial.println(")");
  Serial.println();

  printSREG(sreg_after);

  Serial.println();
  bool carry = (sreg_after >> 0) & 1;
  Serial.print("  Summary: ");
  Serial.print(a); Serial.print(" - "); Serial.print(b); Serial.print(" = ");
  if (carry) {
    Serial.println(result);
    Serial.print("  (underflow: result = 256 + ");
    Serial.print(a); Serial.print(" - "); Serial.print(b);
    Serial.print(" = "); Serial.print(result); Serial.println(")");
  } else {
    Serial.println(result);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("========================================");
  Serial.println("  Task 2 - Register Subtraction & SREG");
  Serial.println("========================================");
  Serial.println();

  runTest(10, 5,  "CASE 1: 10 - 5  (positive result)");
  runTest(10, 10, "CASE 2: 10 - 10 (zero result)");
  runTest(5,  10, "CASE 3: 5  - 10 (underflow)");

  Serial.println("========================================");
  Serial.println("  Done.");
  Serial.println("========================================");
}

void loop() {}
