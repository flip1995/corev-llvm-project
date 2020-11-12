//===-- RISCVInstrInfoCOREV.td - CORE-V instructions -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the CORE-V instructions.
//
//===----------------------------------------------------------------------===//


let Predicates = [HasStdExtHwlp] in {
  def STARTI  : RVInstHwl_ii<0b000, (ins uimm1:$imm1, uimm12:$imm12),
                            "cv.starti", "$imm1, $imm12">;
  def ENDI    : RVInstHwl_ii<0b001, (ins uimm1:$imm1, uimm12:$imm12),
                            "cv.endi", "$imm1, $imm12">;
  def COUNT   : RVInstHwl_ri<0b010, (ins GPR:$rs1, uimm12:$imm12),
                            "cv.count", "$imm1, $rs1">;
  def COUNTI  : RVInstHwl_ii<0b011, (ins uimm1:$imm1, uimm12:$imm12),
                            "cv.counti", "$imm1, $imm12">;
  def SETUP   : RVInstHwl_rii<0b100, (ins uimm1:$imm1, GPR:$rs1, uimm12:$imm12),
                            "cv.setup", "$imm1, $rs1, $imm12">;
  def SETUPI  : RVInstHwl_iii<0b101, (ins uimm1:$imm1, uimm5:$uimm5, uimm12:$imm12),
                            "cv.setupi", "$imm1, $imm5, $imm12">;
} // Predicates = [HasStdExtHwlp]
