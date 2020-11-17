# RUN: llvm-mc -triple riscv32 --mattr=+xcorevhwlp -show-encoding %s \
# RUN:     | FileCheck -check-prefix=CHECK-FIXUP %s

.LBB0:
cv.starti 0, .LBB1
# CHECK-FIXUP: fixup A - offset: 0, value: .LBB1, kind: fixup_riscv_cvpcrel_ui12
cv.endi 0, .LBB1
# CHECK-FIXUP: fixup A - offset: 0, value: .LBB1, kind: fixup_riscv_cvpcrel_ui12
cv.setup 0, zero, .LBB1
# CHECK-FIXUP: fixup A - offset: 0, value: .LBB1, kind: fixup_riscv_cvpcrel_ui12
cv.setupi 0, .LBB1, 0
# CHECK-FIXUP: fixup A - offset: 0, value: .LBB1, kind: fixup_riscv_cvpcrel_urs1

.LBB1: nop
