//===-- RISCVAsmBackend.cpp - RISCV Assembler Backend ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "RISCVAsmBackend.h"
#include "RISCVMCExpr.h"
#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

Optional<MCFixupKind> RISCVAsmBackend::getFixupKind(StringRef Name) const {
  if (STI.getTargetTriple().isOSBinFormatELF()) {
    unsigned Type;
    Type = llvm::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "llvm/BinaryFormat/ELFRelocs/RISCV.def"
#undef ELF_RELOC
               .Default(-1u);
    if (Type != -1u)
      return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  }
  return None;
}

const MCFixupKindInfo &
RISCVAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[] = {
      // This table *must* be in the order that the fixup_* kinds are defined in
      // RISCVFixupKinds.h.
      //
      // name                      offset bits  flags
      {"fixup_riscv_hi20", 12, 20, 0},
      {"fixup_riscv_lo12_i", 20, 12, 0},
      {"fixup_riscv_lo12_s", 0, 32, 0},
      {"fixup_riscv_pcrel_hi20", 12, 20,
       MCFixupKindInfo::FKF_IsPCRel | MCFixupKindInfo::FKF_IsTarget},
      {"fixup_riscv_pcrel_lo12_i", 20, 12,
       MCFixupKindInfo::FKF_IsPCRel | MCFixupKindInfo::FKF_IsTarget},
      {"fixup_riscv_pcrel_lo12_s", 0, 32,
       MCFixupKindInfo::FKF_IsPCRel | MCFixupKindInfo::FKF_IsTarget},
      {"fixup_riscv_got_hi20", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_tprel_hi20", 12, 20, 0},
      {"fixup_riscv_tprel_lo12_i", 20, 12, 0},
      {"fixup_riscv_tprel_lo12_s", 0, 32, 0},
      {"fixup_riscv_tprel_add", 0, 0, 0},
      {"fixup_riscv_tls_got_hi20", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_tls_gd_hi20", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_jal", 12, 20, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_branch", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_rvc_jump", 2, 11, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_rvc_branch", 0, 16, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_call", 0, 64, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_call_plt", 0, 64, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_relax", 0, 0, 0},
      {"fixup_riscv_align", 0, 0, 0},
      {"fixup_riscv_cvpcrel_ui12", 20, 12, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_riscv_cvpcrel_urs1", 15, 5, MCFixupKindInfo::FKF_IsPCRel}};
  static_assert((array_lengthof(Infos)) == RISCV::NumTargetFixupKinds,
                "Not all fixup kinds added to Infos array");

  // Fixup kinds from .reloc directive are like R_RISCV_NONE. They
  // do not require any extra processing.
  if (Kind >= FirstLiteralRelocationKind)
    return MCAsmBackend::getFixupKindInfo(FK_NONE);

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

// If linker relaxation is enabled, or the relax option had previously been
// enabled, always emit relocations even if the fixup can be resolved. This is
// necessary for correctness as offsets may change during relaxation.
bool RISCVAsmBackend::shouldForceRelocation(const MCAssembler &Asm,
                                            const MCFixup &Fixup,
                                            const MCValue &Target) {
  if (Fixup.getKind() >= FirstLiteralRelocationKind)
    return true;
  switch (Fixup.getTargetKind()) {
  default:
    break;
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    if (Target.isAbsolute())
      return false;
    break;
  case RISCV::fixup_riscv_got_hi20:
  case RISCV::fixup_riscv_tls_got_hi20:
  case RISCV::fixup_riscv_tls_gd_hi20:
    return true;
  }

  return STI.getFeatureBits()[RISCV::FeatureRelax] || ForceRelocs;
}

bool RISCVAsmBackend::fixupNeedsRelaxationAdvanced(const MCFixup &Fixup,
                                                   bool Resolved,
                                                   uint64_t Value,
                                                   const MCRelaxableFragment *DF,
                                                   const MCAsmLayout &Layout,
                                                   const bool WasForced) const {
  // Return true if the symbol is actually unresolved.
  // Resolved could be always false when shouldForceRelocation return true.
  // We use !WasForced to indicate that the symbol is unresolved and not forced
  // by shouldForceRelocation.
  if (!Resolved && !WasForced)
    return true;

  int64_t Offset = int64_t(Value);
  switch (Fixup.getTargetKind()) {
  default:
    return false;
  case RISCV::fixup_riscv_rvc_branch:
    // For compressed branch instructions the immediate must be
    // in the range [-256, 254].
    return Offset > 254 || Offset < -256;
  case RISCV::fixup_riscv_rvc_jump:
    // For compressed jump instructions the immediate must be
    // in the range [-2048, 2046].
    return Offset > 2046 || Offset < -2048;
  }
}

void RISCVAsmBackend::relaxInstruction(MCInst &Inst,
                                       const MCSubtargetInfo &STI) const {
  // TODO: replace this with call to auto generated uncompressinstr() function.
  MCInst Res;
  switch (Inst.getOpcode()) {
  default:
    llvm_unreachable("Opcode not expected!");
  case RISCV::C_BEQZ:
    // c.beqz $rs1, $imm -> beq $rs1, X0, $imm.
    Res.setOpcode(RISCV::BEQ);
    Res.addOperand(Inst.getOperand(0));
    Res.addOperand(MCOperand::createReg(RISCV::X0));
    Res.addOperand(Inst.getOperand(1));
    break;
  case RISCV::C_BNEZ:
    // c.bnez $rs1, $imm -> bne $rs1, X0, $imm.
    Res.setOpcode(RISCV::BNE);
    Res.addOperand(Inst.getOperand(0));
    Res.addOperand(MCOperand::createReg(RISCV::X0));
    Res.addOperand(Inst.getOperand(1));
    break;
  case RISCV::C_J:
    // c.j $imm -> jal X0, $imm.
    Res.setOpcode(RISCV::JAL);
    Res.addOperand(MCOperand::createReg(RISCV::X0));
    Res.addOperand(Inst.getOperand(0));
    break;
  case RISCV::C_JAL:
    // c.jal $imm -> jal X1, $imm.
    Res.setOpcode(RISCV::JAL);
    Res.addOperand(MCOperand::createReg(RISCV::X1));
    Res.addOperand(Inst.getOperand(0));
    break;
  }
  Inst = std::move(Res);
}

// Given a compressed control flow instruction this function returns
// the expanded instruction.
unsigned RISCVAsmBackend::getRelaxedOpcode(unsigned Op) const {
  switch (Op) {
  default:
    return Op;
  case RISCV::C_BEQZ:
    return RISCV::BEQ;
  case RISCV::C_BNEZ:
    return RISCV::BNE;
  case RISCV::C_J:
  case RISCV::C_JAL: // fall through.
    return RISCV::JAL;
  }
}

bool RISCVAsmBackend::mayNeedRelaxation(const MCInst &Inst,
                                        const MCSubtargetInfo &STI) const {
  return getRelaxedOpcode(Inst.getOpcode()) != Inst.getOpcode();
}

bool RISCVAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
  bool HasStdExtC = STI.getFeatureBits()[RISCV::FeatureStdExtC];
  unsigned MinNopLen = HasStdExtC ? 2 : 4;

  if ((Count % MinNopLen) != 0)
    return false;

  // The canonical nop on RISC-V is addi x0, x0, 0.
  for (; Count >= 4; Count -= 4)
    OS.write("\x13\0\0\0", 4);

  // The canonical nop on RVC is c.nop.
  if (Count && HasStdExtC)
    OS.write("\x01\0", 2);

  return true;
}

static uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {
  switch (Fixup.getTargetKind()) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case RISCV::fixup_riscv_got_hi20:
  case RISCV::fixup_riscv_tls_got_hi20:
  case RISCV::fixup_riscv_tls_gd_hi20:
    llvm_unreachable("Relocation should be unconditionally forced\n");
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
  case FK_Data_6b:
    return Value;
  case RISCV::fixup_riscv_lo12_i:
  case RISCV::fixup_riscv_pcrel_lo12_i:
  case RISCV::fixup_riscv_tprel_lo12_i:
    return Value & 0xfff;
  case RISCV::fixup_riscv_lo12_s:
  case RISCV::fixup_riscv_pcrel_lo12_s:
  case RISCV::fixup_riscv_tprel_lo12_s:
    return (((Value >> 5) & 0x7f) << 25) | ((Value & 0x1f) << 7);
  case RISCV::fixup_riscv_hi20:
  case RISCV::fixup_riscv_pcrel_hi20:
  case RISCV::fixup_riscv_tprel_hi20:
    // Add 1 if bit 11 is 1, to compensate for low 12 bits being negative.
    return ((Value + 0x800) >> 12) & 0xfffff;
  case RISCV::fixup_riscv_jal: {
    if (!isInt<21>(Value))
      Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
    if (Value & 0x1)
      Ctx.reportError(Fixup.getLoc(), "fixup value must be 2-byte aligned");
    // Need to produce imm[19|10:1|11|19:12] from the 21-bit Value.
    unsigned Sbit = (Value >> 20) & 0x1;
    unsigned Hi8 = (Value >> 12) & 0xff;
    unsigned Mid1 = (Value >> 11) & 0x1;
    unsigned Lo10 = (Value >> 1) & 0x3ff;
    // Inst{31} = Sbit;
    // Inst{30-21} = Lo10;
    // Inst{20} = Mid1;
    // Inst{19-12} = Hi8;
    Value = (Sbit << 19) | (Lo10 << 9) | (Mid1 << 8) | Hi8;
    return Value;
  }
  case RISCV::fixup_riscv_branch: {
    if (!isInt<13>(Value))
      Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
    if (Value & 0x1)
      Ctx.reportError(Fixup.getLoc(), "fixup value must be 2-byte aligned");
    // Need to extract imm[12], imm[10:5], imm[4:1], imm[11] from the 13-bit
    // Value.
    unsigned Sbit = (Value >> 12) & 0x1;
    unsigned Hi1 = (Value >> 11) & 0x1;
    unsigned Mid6 = (Value >> 5) & 0x3f;
    unsigned Lo4 = (Value >> 1) & 0xf;
    // Inst{31} = Sbit;
    // Inst{30-25} = Mid6;
    // Inst{11-8} = Lo4;
    // Inst{7} = Hi1;
    Value = (Sbit << 31) | (Mid6 << 25) | (Lo4 << 8) | (Hi1 << 7);
    return Value;
  }
  case RISCV::fixup_riscv_call:
  case RISCV::fixup_riscv_call_plt: {
    // Jalr will add UpperImm with the sign-extended 12-bit LowerImm,
    // we need to add 0x800ULL before extract upper bits to reflect the
    // effect of the sign extension.
    uint64_t UpperImm = (Value + 0x800ULL) & 0xfffff000ULL;
    uint64_t LowerImm = Value & 0xfffULL;
    return UpperImm | ((LowerImm << 20) << 32);
  }
  case RISCV::fixup_riscv_rvc_jump: {
    // Need to produce offset[11|4|9:8|10|6|7|3:1|5] from the 11-bit Value.
    unsigned Bit11  = (Value >> 11) & 0x1;
    unsigned Bit4   = (Value >> 4) & 0x1;
    unsigned Bit9_8 = (Value >> 8) & 0x3;
    unsigned Bit10  = (Value >> 10) & 0x1;
    unsigned Bit6   = (Value >> 6) & 0x1;
    unsigned Bit7   = (Value >> 7) & 0x1;
    unsigned Bit3_1 = (Value >> 1) & 0x7;
    unsigned Bit5   = (Value >> 5) & 0x1;
    Value = (Bit11 << 10) | (Bit4 << 9) | (Bit9_8 << 7) | (Bit10 << 6) |
            (Bit6 << 5) | (Bit7 << 4) | (Bit3_1 << 1) | Bit5;
    return Value;
  }
  case RISCV::fixup_riscv_rvc_branch: {
    // Need to produce offset[8|4:3], [reg 3 bit], offset[7:6|2:1|5]
    unsigned Bit8   = (Value >> 8) & 0x1;
    unsigned Bit7_6 = (Value >> 6) & 0x3;
    unsigned Bit5   = (Value >> 5) & 0x1;
    unsigned Bit4_3 = (Value >> 3) & 0x3;
    unsigned Bit2_1 = (Value >> 1) & 0x3;
    Value = (Bit8 << 12) | (Bit4_3 << 10) | (Bit7_6 << 5) | (Bit2_1 << 3) |
            (Bit5 << 2);
    return Value;
  }
  case RISCV::fixup_riscv_cvpcrel_ui12:
    return (Value >> 1) & 0xfff;
  case RISCV::fixup_riscv_cvpcrel_urs1:
    return (Value >> 1) & 0x1f;
  }
}

bool RISCVAsmBackend::evaluateTargetFixup(
    const MCAssembler &Asm, const MCAsmLayout &Layout, const MCFixup &Fixup,
    const MCFragment *DF, const MCValue &Target, uint64_t &Value,
    bool &WasForced) {
  const MCFixup *AUIPCFixup;
  const MCFragment *AUIPCDF;
  MCValue AUIPCTarget;
  switch (Fixup.getTargetKind()) {
  default:
    llvm_unreachable("Unexpected fixup kind!");
  case RISCV::fixup_riscv_pcrel_hi20:
    AUIPCFixup = &Fixup;
    AUIPCDF = DF;
    AUIPCTarget = Target;
    break;
  case RISCV::fixup_riscv_pcrel_lo12_i:
  case RISCV::fixup_riscv_pcrel_lo12_s: {
    AUIPCFixup = cast<RISCVMCExpr>(Fixup.getValue())->getPCRelHiFixup(&AUIPCDF);
    if (!AUIPCFixup) {
      Asm.getContext().reportError(Fixup.getLoc(),
                                   "could not find corresponding %pcrel_hi");
      return true;
    }

    // MCAssembler::evaluateFixup will emit an error for this case when it sees
    // the %pcrel_hi, so don't duplicate it when also seeing the %pcrel_lo.
    const MCExpr *AUIPCExpr = AUIPCFixup->getValue();
    if (!AUIPCExpr->evaluateAsRelocatable(AUIPCTarget, &Layout, AUIPCFixup))
      return true;
    break;
  }
  }

  if (!AUIPCTarget.getSymA() || AUIPCTarget.getSymB())
    return false;

  const MCSymbolRefExpr *A = AUIPCTarget.getSymA();
  const MCSymbol &SA = A->getSymbol();
  if (A->getKind() != MCSymbolRefExpr::VK_None || SA.isUndefined())
    return false;

  auto *Writer = Asm.getWriterPtr();
  if (!Writer)
    return false;

  bool IsResolved = Writer->isSymbolRefDifferenceFullyResolvedImpl(
      Asm, SA, *AUIPCDF, false, true);
  if (!IsResolved)
    return false;

  Value = Layout.getSymbolOffset(SA) + AUIPCTarget.getConstant();
  Value -= Layout.getFragmentOffset(AUIPCDF) + AUIPCFixup->getOffset();

  if (shouldForceRelocation(Asm, *AUIPCFixup, AUIPCTarget)) {
    WasForced = true;
    return false;
  }

  return true;
}

void RISCVAsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                 const MCValue &Target,
                                 MutableArrayRef<char> Data, uint64_t Value,
                                 bool IsResolved,
                                 const MCSubtargetInfo *STI) const {
  MCFixupKind Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return;
  MCContext &Ctx = Asm.getContext();
  MCFixupKindInfo Info = getFixupKindInfo(Kind);
  if (!Value)
    return; // Doesn't change encoding.
  // Apply any target-specific value adjustments.
  Value = adjustFixupValue(Fixup, Value, Ctx);

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  unsigned Offset = Fixup.getOffset();
  unsigned NumBytes = alignTo(Info.TargetSize + Info.TargetOffset, 8) / 8;

  assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned i = 0; i != NumBytes; ++i) {
    Data[Offset + i] |= uint8_t((Value >> (i * 8)) & 0xff);
  }
}

// Linker relaxation may change code size. We have to insert Nops
// for .align directive when linker relaxation enabled. So then Linker
// could satisfy alignment by removing Nops.
// The function return the total Nops Size we need to insert.
bool RISCVAsmBackend::shouldInsertExtraNopBytesForCodeAlign(
    const MCAlignFragment &AF, unsigned &Size) {
  // Calculate Nops Size only when linker relaxation enabled.
  if (!STI.getFeatureBits()[RISCV::FeatureRelax])
    return false;

  bool HasStdExtC = STI.getFeatureBits()[RISCV::FeatureStdExtC];
  unsigned MinNopLen = HasStdExtC ? 2 : 4;

  if (AF.getAlignment() <= MinNopLen) {
    return false;
  } else {
    Size = AF.getAlignment() - MinNopLen;
    return true;
  }
}

// We need to insert R_RISCV_ALIGN relocation type to indicate the
// position of Nops and the total bytes of the Nops have been inserted
// when linker relaxation enabled.
// The function insert fixup_riscv_align fixup which eventually will
// transfer to R_RISCV_ALIGN relocation type.
bool RISCVAsmBackend::shouldInsertFixupForCodeAlign(MCAssembler &Asm,
                                                    const MCAsmLayout &Layout,
                                                    MCAlignFragment &AF) {
  // Insert the fixup only when linker relaxation enabled.
  if (!STI.getFeatureBits()[RISCV::FeatureRelax])
    return false;

  // Calculate total Nops we need to insert. If there are none to insert
  // then simply return.
  unsigned Count;
  if (!shouldInsertExtraNopBytesForCodeAlign(AF, Count) || (Count == 0))
    return false;

  MCContext &Ctx = Asm.getContext();
  const MCExpr *Dummy = MCConstantExpr::create(0, Ctx);
  // Create fixup_riscv_align fixup.
  MCFixup Fixup =
      MCFixup::create(0, Dummy, MCFixupKind(RISCV::fixup_riscv_align), SMLoc());

  uint64_t FixedValue = 0;
  MCValue NopBytes = MCValue::get(Count);

  Asm.getWriter().recordRelocation(Asm, Layout, &AF, Fixup, NopBytes,
                                   FixedValue);

  return true;
}

std::unique_ptr<MCObjectTargetWriter>
RISCVAsmBackend::createObjectTargetWriter() const {
  return createRISCVELFObjectWriter(OSABI, Is64Bit);
}

MCAsmBackend *llvm::createRISCVAsmBackend(const Target &T,
                                          const MCSubtargetInfo &STI,
                                          const MCRegisterInfo &MRI,
                                          const MCTargetOptions &Options) {
  const Triple &TT = STI.getTargetTriple();
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  return new RISCVAsmBackend(STI, OSABI, TT.isArch64Bit(), Options);
}
