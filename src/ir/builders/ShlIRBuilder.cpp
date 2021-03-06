#include <iostream>
#include <sstream>
#include <stdexcept>

#include <ShlIRBuilder.h>
#include <Registers.h>
#include <SMT2Lib.h>
#include <SymbolicElement.h>


ShlIRBuilder::ShlIRBuilder(uint64_t address, const std::string &disassembly):
  BaseIRBuilder(address, disassembly) {
}


void ShlIRBuilder::regImm(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint64_t          reg     = this->operands[0].getValue();
  uint64_t          imm     = this->operands[1].getValue();
  uint32_t          regSize = this->operands[0].getSize();

  /* Create the SMT semantic */
  op1 << ap.buildSymbolicRegOperand(reg, regSize);
  op2 << smt2lib::bv(imm, regSize * REG_SIZE);

  /* Finale expr */
  expr << smt2lib::bvshl(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createRegSE(inst, expr, reg, regSize);

  /* Apply the taint */
  ap.aluSpreadTaintRegReg(se, reg, reg);

  /* Add the symbolic flags element to the current inst */
  EflagsBuilder::cfShl(inst, se, ap, regSize, op1, op2);
  EflagsBuilder::ofShl(inst, se, ap, regSize, op1, op2);
  EflagsBuilder::pfShl(inst, se, ap, regSize, op2);
  EflagsBuilder::sfShl(inst, se, ap, regSize, op2);
  EflagsBuilder::zfShl(inst, se, ap, regSize, op2);
}


void ShlIRBuilder::regReg(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint64_t          reg     = this->operands[0].getValue();
  uint32_t          regSize = this->operands[0].getSize();

  /* Create the SMT semantic */
  op1 << ap.buildSymbolicRegOperand(reg, regSize);
  op2 << smt2lib::zx(ap.buildSymbolicRegOperand(ID_RCX, 1), (regSize - 1) * REG_SIZE);

  /* Finale expr */
  expr << smt2lib::bvshl(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createRegSE(inst, expr, reg, regSize);

  /* Apply the taint */
  ap.aluSpreadTaintRegReg(se, reg, reg);

  /* Add the symbolic flags element to the current inst */
  EflagsBuilder::cfShl(inst, se, ap, regSize, op1, op2);
  EflagsBuilder::ofShl(inst, se, ap, regSize, op1, op2);
  EflagsBuilder::pfShl(inst, se, ap, regSize, op2);
  EflagsBuilder::sfShl(inst, se, ap, regSize, op2);
  EflagsBuilder::zfShl(inst, se, ap, regSize, op2);
}


void ShlIRBuilder::regMem(AnalysisProcessor &ap, Inst &inst) const {
  TwoOperandsTemplate::stop(this->disas);
}


void ShlIRBuilder::memImm(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1, op2;
  uint32_t          writeSize = this->operands[0].getSize();
  uint64_t          mem       = this->operands[0].getValue();
  uint64_t          imm       = this->operands[1].getValue();

  /* Create the SMT semantic */
  op1 << ap.buildSymbolicMemOperand(mem, writeSize);
  op2 << smt2lib::bv(imm, writeSize * REG_SIZE);

  /* Final expr */
  expr << smt2lib::bvshl(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createMemSE(inst, expr, mem, writeSize);

  /* Apply the taint */
  ap.aluSpreadTaintMemMem(se, mem, mem);

  /* Add the symbolic flags element to the current inst */
  EflagsBuilder::cfShl(inst, se, ap, writeSize, op1, op2);
  EflagsBuilder::ofShl(inst, se, ap, writeSize, op1, op2);
  EflagsBuilder::pfShl(inst, se, ap, writeSize, op2);
  EflagsBuilder::sfShl(inst, se, ap, writeSize, op2);
  EflagsBuilder::zfShl(inst, se, ap, writeSize, op2);
}


void ShlIRBuilder::memReg(AnalysisProcessor &ap, Inst &inst) const {
  TwoOperandsTemplate::stop(this->disas);
}


Inst *ShlIRBuilder::process(AnalysisProcessor &ap) const {
  this->checkSetup();

  Inst *inst = new Inst(ap.getThreadID(), this->address, this->disas);

  try {
    this->templateMethod(ap, *inst, this->operands, "SHL");
    ap.incNumberOfExpressions(inst->numberOfElements()); /* Used for statistics */
    ControlFlow::rip(*inst, ap, this->nextAddress);
  }
  catch (std::exception &e) {
    delete inst;
    throw;
  }

  return inst;
}

