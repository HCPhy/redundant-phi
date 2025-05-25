int foo(int x, int c) {
  int y = x + 0;      // ← add with zero
  if (c)
    y = y * 1;        // ← mul with one
  else
    y = y * 1;
  return y;
}
