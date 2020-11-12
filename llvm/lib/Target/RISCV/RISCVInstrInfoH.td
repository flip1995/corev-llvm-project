//===-- RISCVInstrInfoA.td - RISC-V 'A' instructions -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the CORE-V instructions from the Hardware Loop
// extension.
//
//===----------------------------------------------------------------------===//


let Predicates = [HasStdExtHwlp] in {
  def STARTI  : RVInstHwlp_ii<0b000, (ins uimm1:$imm1, uimm12:$imm12),
                            "cv.starti", "$imm1, $imm12">;
  def ENDI    : RVInstHwlp_ii<0b001, (ins uimm1:$imm1, uimm12:$imm12),
                            "cv.endi", "$imm1, $imm12">;
  def COUNT   : RVInstHwlp_ri<0b010, (ins GPR:$rs1, uimm12:$imm12),
                            "cv.count", "$imm1, $rs1">;
  def COUNTI  : RVInstHwlp_ii<0b011, (ins uimm1:$imm1, uimm12:$imm12),
                            "cv.counti", "$imm1, $imm12">;
  def SETUP   : RVInstHwlp_rii<0b100, (ins uimm1:$imm1, GPR:$rs1, uimm12:$imm12),
                            "cv.setup", "$imm1, $rs1, $imm12">;
  def SETUPI  : RVInstHwlp_iii<0b101, (ins uimm1:$imm1, uimm5:$uimm5, uimm12:$imm12),
                            "cv.setupi", "$imm1, $imm5, $imm12">;
} // Predicates = [HasStdExtHwlp]
