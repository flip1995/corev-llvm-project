int foo(int dest, int src1, int src2) {
  dest = __builtin_corev_mac(dest, src1, src2);
  return dest;
}
