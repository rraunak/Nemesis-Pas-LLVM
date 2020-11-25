//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//


#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Compiler.h"
#include "llvm/ADT/Statistic.h"
#include <iostream>
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include "AArch64.h"
#include "AArch64InstrInfo.h"
#include "AArch64RegisterInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBundle.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm//MC/MCInst.h"
#include <map>
#include <unordered_map>
#include "llvm-c/Core.h" //HwiSoo

namespace llvm{
        static cl::opt<bool> EnableNEMESIS(
                        "enable-NEMESIS",
                        cl::init(false),
                        cl::desc("Implement NEMESIS."),
                        cl::Hidden);

        struct Nemesis : public MachineFunctionPass {

                public:
                        static char ID;

                        Nemesis() : MachineFunctionPass(ID) {
                          srand(777);
                        }

                std::unordered_map<unsigned int, unsigned int> registersMtoS = {{ AArch64::X0, AArch64::X8 }, { AArch64::W0, AArch64::W8 }, { AArch64::X1, AArch64::X9 }, { AArch64::W1, AArch64::W9 }, { AArch64::X2, AArch64::X10 }, { AArch64::W2, AArch64::W10 }, { AArch64::X3, AArch64::X11 }, { AArch64::W3, AArch64::W11 }, { AArch64::X4, AArch64::X12 }, { AArch64::W4, AArch64::W12 }, { AArch64::X5, AArch64::X13 }, { AArch64::W5, AArch64::W13 }, { AArch64::X6, AArch64::X14 }, { AArch64::W6, AArch64::W14 }, /*{ AArch64::X7, AArch64::X15 }, { AArch64::W7, AArch64::W15 },*/ { AArch64::SP, AArch64::X16}, { AArch64::LR, AArch64::X17}};

                std::unordered_map<unsigned int, unsigned int> registersMtoS2 = {{ AArch64::X0, AArch64::X15 }, { AArch64::W0, AArch64::W15 }, { AArch64::X1, AArch64::X19 }, { AArch64::W1, AArch64::W19 }, { AArch64::X2, AArch64::X20 }, { AArch64::W2, AArch64::W20 }, { AArch64::X3, AArch64::X21 }, { AArch64::W3, AArch64::W21 }, { AArch64::X4, AArch64::X22 }, { AArch64::W4, AArch64::W22 }, { AArch64::X5, AArch64::X23 }, { AArch64::W5, AArch64::W23 }, { AArch64::X6, AArch64::X24 }, { AArch64::W6, AArch64::W24 }, /*{ AArch64::X7, AArch64::X25 }, { AArch64::W7, AArch64::W25 },*/ { AArch64::SP, AArch64::X26}, { AArch64::LR, AArch64::X27}};

                std::vector<unsigned int> functionCallArgs = {AArch64::X0, AArch64::W0, AArch64::X1, AArch64::W1, AArch64::X2, AArch64::W2, AArch64::X3, AArch64::W3, AArch64::X4, AArch64::W4, AArch64::X5, AArch64::W5, AArch64::X6, AArch64::W6/*, AArch64::X7, AArch64::W7*/};

                std::vector<unsigned int> functionCallArgs1 = {AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6/*, AArch64::X7*/};

                //std::unordered_map<unsigned int> functionCallArg = {AArch64::X0, AArch64::W0, AArch64::X1, AArch64::W1, AArch64::X2, AArch64::W2, AArch64::X3, AArch64::W3, AArch64::X4, AArch64::W4, AArch64::X5, AArch64::W5, AArch64::X6, AArch64::W6, AArch64::X7, AArch64::W7};


                std::unordered_map<unsigned int, unsigned int> registers64shadow1 = {{ AArch64::X0, AArch64::X8 }, { AArch64::X1, AArch64::X9 }, { AArch64::X2, AArch64::X10 }, { AArch64::X3, AArch64::X11 }, { AArch64::X4, AArch64::X12 }, { AArch64::X5, AArch64::X13 }, { AArch64::X6, AArch64::X14 }, { AArch64::X7, AArch64::X15 }, { AArch64::SP, AArch64::X16 }, { AArch64::LR, AArch64::X17}};

                std::unordered_map<unsigned int, unsigned int> registers64voting = {{ AArch64::X0, AArch64::X8 }, { AArch64::W0, AArch64::X8 }, { AArch64::X1, AArch64::X9 }, { AArch64::W1, AArch64::X9 }, { AArch64::X2, AArch64::X10 }, { AArch64::W2, AArch64::X10 }, { AArch64::X3, AArch64::X11 }, { AArch64::W3, AArch64::X11 }, { AArch64::X4, AArch64::X12 }, { AArch64::W4, AArch64::X12 }, { AArch64::X5, AArch64::X13 }, { AArch64::W5, AArch64::X13 }, { AArch64::X6, AArch64::X14 }, { AArch64::W6, AArch64::X14 }, /*{ AArch64::X7, AArch64::X15 }, { AArch64::W7, AArch64::X15 },*/ { AArch64::SP, AArch64::X16}, { AArch64::LR, AArch64::X17}};

                std::unordered_map<unsigned int, unsigned int> registers32shadow1 = {{ AArch64::W0, AArch64::W8 }, { AArch64::W1, AArch64::W9 }, { AArch64::W2, AArch64::W10 }, { AArch64::W3, AArch64::W11 }, { AArch64::W4, AArch64::W12 }, { AArch64::W5, AArch64::W13 }, { AArch64::W6, AArch64::W14 }, { AArch64::W7, AArch64::W15 }};

                std::unordered_map<unsigned int, unsigned int> registers64shadow2 = {{ AArch64::X0, AArch64::X15 }, { AArch64::X1, AArch64::X19 }, { AArch64::X2, AArch64::X20 }, { AArch64::X3, AArch64::X21 }, { AArch64::X4, AArch64::X22 }, { AArch64::X5, AArch64::X23 }, { AArch64::X6, AArch64::X24 },/* { AArch64::X7, AArch64::X25 },*/ { AArch64::SP, AArch64::X26 }, { AArch64::LR, AArch64::X27}};

                std::unordered_map<unsigned int, unsigned int> registers64voting2 = {{ AArch64::X0, AArch64::X15 }, { AArch64::W0, AArch64::X15 }, { AArch64::X1, AArch64::X19 }, { AArch64::W1, AArch64::X19 }, { AArch64::X2, AArch64::X20 }, { AArch64::W2, AArch64::X20 }, { AArch64::X3, AArch64::X21 }, { AArch64::W3, AArch64::X21 }, { AArch64::X4, AArch64::X22 }, { AArch64::W4, AArch64::X22 }, { AArch64::X5, AArch64::X23 }, { AArch64::W5, AArch64::X23 }, { AArch64::X6, AArch64::X24 }, { AArch64::W6, AArch64::X24 },/* { AArch64::X7, AArch64::X25 }, { AArch64::W7, AArch64::X25 },*/ { AArch64::SP, AArch64::X26}, { AArch64::LR, AArch64::X27}};

                std::unordered_map<unsigned int, unsigned int> registers32shadow2 = {{ AArch64::W0, AArch64::W15 }, { AArch64::W1, AArch64::W19 }, { AArch64::W2, AArch64::W20 }, { AArch64::W3, AArch64::W21 }, { AArch64::W4, AArch64::W22 }, { AArch64::W5, AArch64::W23 }, { AArch64::W6, AArch64::W24 }, /*{ AArch64::W7, AArch64::W25 }*/};


                unsigned int getSlvReg(unsigned int reg){

                        std::unordered_map<unsigned int, unsigned int>::const_iterator num = registersMtoS.find(reg);

                        if(num == registersMtoS.end())
                        {
                          return reg;
                        }
                        else
                        {
                          return num->second;
                        }
                }

                unsigned int getSlvReg2(unsigned int reg){

                        std::unordered_map<unsigned int, unsigned int>::const_iterator num = registersMtoS2.find(reg);

                        if(num == registersMtoS2.end())
                        {
                          return reg;
                        }
                        else
                        {
                          return num->second;
                        }
                }

                bool isGPR(unsigned int regNUM) {
                  std::unordered_map<unsigned int, unsigned int>::const_iterator got = registers64shadow1.find(regNUM);
                  if (got == registers64voting.end())
                  {
                    return false;
                  }
                  std::unordered_map<unsigned int, unsigned int>::const_iterator got2 = registers64shadow2.find(regNUM);
                  if (got2 == registers64voting2.end())
                  {
                    return false;
                  }
                  return true;
                }

                bool isGPR64(unsigned int regNUM) {
                  std::unordered_map<unsigned int, unsigned int>::const_iterator got = registers64shadow1.find(regNUM);
                  if (got == registers64shadow1.end())
                  {
                    return false;
                  }
                  std::unordered_map<unsigned int, unsigned int>::const_iterator got2 = registers64shadow2.find(regNUM);
                  if (got2 == registers64shadow2.end())
                  {
                    return false;
                  }
                  return true;
                }

                bool isGPR32(unsigned int regNUM) {
                  std::unordered_map<unsigned int, unsigned int>::const_iterator got = registers32shadow1.find(regNUM);
                  if (got == registers32shadow1.end())
                  {
                    return false;
                  }
                  std::unordered_map<unsigned int, unsigned int>::const_iterator got2 = registers32shadow2.find(regNUM);
                  if (got2 == registers32shadow2.end())
                  {
                    return false;
                  }
                  return true;
                }

          void triplicateInstructions(MachineFunction &MF) {
            //const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
            for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); MBB != MBE; ++MBB) {
              for(MachineBasicBlock::iterator I=MBB->begin(), E=MBB->end(); I != E; ++I) {
                if(!(I->isBranch()) && !(I->mayStore()) && !(I->isCall()) && /*(I->getOperand(0).getReg() != AArch64::SP) &&**/  !(I->isReturn()) && !(I->isCompare()) && !(I->getOpcode() > 96 && I->getOpcode() < 129)) {
                  MachineInstr *slaveinst = MF.CloneMachineInstr (&*I);
                  MachineInstr *slaveinst2 = MF.CloneMachineInstr (&*I);

                  slaveinst->setFlags(0);
                  slaveinst2->setFlags(0);

                  for(unsigned int opcount=0; opcount<I->getNumOperands(); opcount++){
                    if(I->getOperand(opcount).isReg()){
                      slaveinst->getOperand(opcount).setReg(getSlvReg(I->getOperand(opcount).getReg()));
                    }
                  }

                  for(unsigned int opcount=0; opcount<I->getNumOperands(); opcount++){
                    if(I->getOperand(opcount).isReg()){
                      slaveinst2->getOperand(opcount).setReg(getSlvReg2(I->getOperand(opcount).getReg()));
                    }
                  }
                  if(I->getOperand(0).getReg() == AArch64::SP)
                  {
                    MBB->insertAfter(I, slaveinst);
                    MBB->insertAfter(I, slaveinst2);
                  }
                  else if(I->getOperand(0).getReg() == AArch64::X16 || I->getOperand(0).getReg() == AArch64::X26)
                  {
                    continue;
                  }
                  else {

                    MBB->insert(I, slaveinst);
                    MBB->insert(I, slaveinst2);
                  }
                }

              }
            }
          }

                void checkFunctionCalls(MachineFunction &MF){
                        const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                        int numBBs=0;
                        for(MachineFunction::iterator MB = MF.begin(), MBE = MF.end(); (MB != MBE) && (numBBs < FUNCSIZE); ++MB, ++numBBs) {
                          if(MF.getName() == "main" && MB == MF.begin()) {
                            DebugLoc DL3 = MB->begin()->getDebugLoc();
                            MachineInstr *MIcopyX0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X8).addReg(AArch64::X0).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX0);
                            MIcopyX0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X0).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX0);
                            MachineInstr *MIcopyX1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X9).addReg(AArch64::X1).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX1);
                            MIcopyX1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X19).addReg(AArch64::X1).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX1);
                            MachineInstr *MIcopyX2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X10).addReg(AArch64::X2).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX2);
                            MIcopyX2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X20).addReg(AArch64::X2).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX2);
                            MachineInstr *MIcopyX3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X11).addReg(AArch64::X3).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX3);
                            MIcopyX3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X21).addReg(AArch64::X3).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX3);
                            MachineInstr *MIcopyX4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X12).addReg(AArch64::X4).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX4);
                            MIcopyX4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X22).addReg(AArch64::X4).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX4);
                            MachineInstr *MIcopyX5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X13).addReg(AArch64::X5).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX5);
                            MIcopyX5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X23).addReg(AArch64::X5).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX5);
                            MachineInstr *MIcopyX6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X14).addReg(AArch64::X6).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX6);
                            MIcopyX6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X24).addReg(AArch64::X6).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX6);
                            //MachineInstr *MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X7).addImm(0).addImm(0);
                            //MB->insert(MB->begin(), MIcopyX7);
                            //MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X25).addReg(AArch64::X7).addImm(0).addImm(0);
                            //MB->insert(MB->begin(), MIcopyX7);
                            MachineInstr *MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX30);
                            MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX30);
                            MachineInstr *MIcopyX31 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X16).addReg(AArch64::SP).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX31);
                            MIcopyX31 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X26).addReg(AArch64::SP).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX31);
                          }

                         /* if(MF.getName() != "main" && MB == MF.begin() && MB != MBE) {
                            DebugLoc DL3 = MB->begin()->getDebugLoc();
                            MachineInstr *MIcopyM0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                            MIcopyM0->setFlags(0);
                            //MB->insert(MB->begin(), MIcopyM0);
                            MB->push_back(MIcopyM0);
                            MachineInstr *MIcopyM1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                            MIcopyM1->setFlags(0);
                            //MB->insert(MB->begin(), MIcopyM1);
                            MB->push_back(MIcopyM1);*/

                            /*MachineInstr *MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX30);
                            MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                            MB->insert(MB->begin(), MIcopyX30);*/

                            /*MachineInstr *MIcopyX0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X8).addReg(AArch64::X0).addImm(0).addImm(0);
                            MIcopyX0->setFlags(0);
                            MB->push_back(MIcopyX0);
                            MachineInstr *MIcopyY0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X0).addImm(0).addImm(0);
                            MIcopyY0->setFlags(0);
                            MB->push_back(MIcopyY0);
                            MachineInstr *MIcopyX1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X9).addReg(AArch64::X1).addImm(0).addImm(0);
                            MIcopyX1->setFlags(0);
                            MB->push_back(MIcopyX1);
                            MachineInstr *MIcopyY1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X19).addReg(AArch64::X1).addImm(0).addImm(0);
                            MIcopyY1->setFlags(0);
                            MB->push_back(MIcopyY1);
                            MachineInstr *MIcopyX2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X10).addReg(AArch64::X2).addImm(0).addImm(0);
                            MIcopyX2->setFlags(0);
                            MB->push_back(MIcopyX2);
                            MachineInstr *MIcopyY2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X20).addReg(AArch64::X2).addImm(0).addImm(0);
                            MIcopyY2->setFlags(0);
                            MB->push_back(MIcopyY2);
                            MachineInstr *MIcopyX3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X11).addReg(AArch64::X3).addImm(0).addImm(0);
                            MIcopyX3->setFlags(0);
                            MB->push_back(MIcopyX3);
                            MachineInstr *MIcopyY3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X21).addReg(AArch64::X3).addImm(0).addImm(0);
                            MIcopyY3->setFlags(0);
                            MB->push_back(MIcopyY3);
                            MachineInstr *MIcopyX4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X12).addReg(AArch64::X4).addImm(0).addImm(0);
                            MIcopyX4->setFlags(0);
                            MB->push_back(MIcopyX4);
                            MachineInstr *MIcopyY4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X22).addReg(AArch64::X4).addImm(0).addImm(0);
                            MIcopyY4->setFlags(0);
                            MB->push_back(MIcopyY4);
                            MachineInstr *MIcopyX5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X13).addReg(AArch64::X5).addImm(0).addImm(0);
                            MIcopyX5->setFlags(0);
                            MB->push_back(MIcopyX5);
                            MachineInstr *MIcopyY5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X23).addReg(AArch64::X5).addImm(0).addImm(0);
                            MIcopyY5->setFlags(0);
                            MB->push_back(MIcopyY5);
                            MachineInstr *MIcopyX6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X14).addReg(AArch64::X6).addImm(0).addImm(0);
                            MIcopyX6->setFlags(0);
                            MB->push_back(MIcopyX6);
                            MachineInstr *MIcopyY6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X24).addReg(AArch64::X6).addImm(0).addImm(0);
                            MIcopyY6->setFlags(0);
                            MB->push_back(MIcopyY6);
                            //MachineInstr *MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X7).addImm(0).addImm(0);
                            //MB->insert(MB->begin(), MIcopyX7);
                            //MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X25).addReg(AArch64::X7).addImm(0).addImm(0);
                            //MB->insert(MB->begin(), MIcopyX7);

                            MachineInstr *MIcopyX31 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X16).addReg(AArch64::SP).addImm(0).addImm(0);
                            MIcopyX31->setFlags(0);
                            MB->push_back(MIcopyX31);
                            MachineInstr *MIcopyY31 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X26).addReg(AArch64::SP).addImm(0).addImm(0);
                            MIcopyY31->setFlags(0);
                            MB->push_back(MIcopyY31);


                          }*/
                          if(MB == MBE) {
                            continue;
                          }
                          
                          for (MachineBasicBlock::iterator I=MB->begin(), E=MB->end(); I !=E; ++I) {
                              if(I->isCall() /*&& std::next(I) != E*/) {
                                DebugLoc DL3 = I->getDebugLoc();
                                const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

                                checkFunctionCallArguments(I, MF);

                                MachineInstr *MIcopyC0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X8).addReg(AArch64::X0).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyC0);
                                MIcopyC0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X0).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyC0);
                                MachineInstr *MIcopyC1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X9).addReg(AArch64::X1).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyC1);
                                MIcopyC1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X19).addReg(AArch64::X1).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyC1);
                                MachineInstr *MIcopyC2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X16).addReg(AArch64::SP).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyC2);
                                MIcopyC2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X26).addReg(AArch64::SP).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyC2);

                                MachineInstr *MIcopyX2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X10).addReg(AArch64::X2).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX2);
                                MIcopyX2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X20).addReg(AArch64::X2).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX2);
                                MachineInstr *MIcopyX3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X11).addReg(AArch64::X3).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX3);
                                MIcopyX3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X21).addReg(AArch64::X3).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX3);
                                MachineInstr *MIcopyX4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X12).addReg(AArch64::X4).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX4);
                                MIcopyX4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X22).addReg(AArch64::X4).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX4);
                                MachineInstr *MIcopyX5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X13).addReg(AArch64::X5).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX5);
                                MIcopyX5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X23).addReg(AArch64::X5).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX5);
                                MachineInstr *MIcopyX6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X14).addReg(AArch64::X6).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX6);
                                MIcopyX6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X24).addReg(AArch64::X6).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX6);
                                //MachineInstr *MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X7).addImm(0).addImm(0);
                                //MB->insert(MB->begin(), MIcopyX7);
                                //MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X25).addReg(AArch64::X7).addImm(0).addImm(0);
                                //MB->insert(MB->begin(), MIcopyX7);
                                MachineInstr *MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX30);
                                MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                                MB->insertAfter(I, MIcopyX30);
                              }

                            if(I->getOpcode() != AArch64::INLINEASM && I->mayStore() && std::next(I) != E){
                              /*
                              errs()<< "HWISOO_DEBUG\n";
                              I->dump();
                              errs() << "Check\n F: "<< MF.getName() << "BB: " << I->getParent()->getName() << "Instr: " << *I << "\n";
                              errs()<<I->getOpcode()<<"/"<<OR1K::NOP<<"\n";
                              */
                              DebugLoc DL3= I->getDebugLoc();
                              const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

                              for (unsigned int opcount=0 ; opcount < I->getNumOperands() ;opcount++){ //
                                if(I->getOperand(opcount).isReg()) {
                                  int reg = I->getOperand(opcount).getReg();
                                  if(((isGPR64(reg)) || (isGPR32(reg))) && (I->getOpcode() == AArch64::STRXpre || I->getOpcode() == AArch64::STPXpre) && (I->getOperand(opcount).getReg() == AArch64::SP) && (I->getOperand(opcount).getReg() != AArch64::WZR) && (I->getOperand(opcount).getReg() != AArch64::XZR)) {
                                    MachineInstr *copyMoveM = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(getSlvReg(I->getOperand(opcount).getReg())).addReg(I->getOperand(opcount).getReg()).addImm(0).addImm(0);
                                    MB->insertAfter(I,copyMoveM);
                                    MachineInstr *copyMoveD = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(getSlvReg2(I->getOperand(opcount).getReg())).addReg(I->getOperand(opcount).getReg()).addImm(0).addImm(0);
                                    MB->insertAfter(I,copyMoveD);
                                    break;
                                  }

                                }
                              } //end of for

                            }

                          }
                        }

                }

                void checkFunctionCalls2(MachineFunction &MF){
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  //int numBBs=0;
                  for(MachineFunction::iterator MB = MF.begin(), MBE = MF.end(); (MB != MBE); ++MB) {
                    if(MF.getName() != "main" && MB == MF.begin() && MB != MBE) {
                      DebugLoc DL3 = MB->begin()->getDebugLoc();
                      /*MachineInstr *MIcopyM0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                      MB->insert(MB->begin(), MIcopyM0);
                      MIcopyM0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                      MB->insert(MB->begin(), MIcopyM0);*/

                      MachineInstr *MIcopyM0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                      MIcopyM0->setFlags(0);
                      //MB->insert(MB->begin(), MIcopyM0);
                      MB->push_back(MIcopyM0);
                      MachineInstr *MIcopyM1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                      MIcopyM1->setFlags(0);
                      //MB->insert(MB->begin(), MIcopyM1);
                      MB->push_back(MIcopyM1);

                      /*MachineInstr *MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                      MB->insert(MB->begin(), MIcopyX30);
                      MIcopyX30 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                      MB->insert(MB->begin(), MIcopyX30);*/

                      MachineInstr *MIcopyX0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X8).addReg(AArch64::X0).addImm(0).addImm(0);
                      MIcopyX0->setFlags(0);
                      MB->push_back(MIcopyX0);
                      MachineInstr *MIcopyY0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X0).addImm(0).addImm(0);
                      MIcopyY0->setFlags(0);
                      MB->push_back(MIcopyY0);
                      MachineInstr *MIcopyX1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X9).addReg(AArch64::X1).addImm(0).addImm(0);
                      MIcopyX1->setFlags(0);
                      MB->push_back(MIcopyX1);
                      MachineInstr *MIcopyY1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X19).addReg(AArch64::X1).addImm(0).addImm(0);
                      MIcopyY1->setFlags(0);
                      MB->push_back(MIcopyY1);
                      MachineInstr *MIcopyX2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X10).addReg(AArch64::X2).addImm(0).addImm(0);
                      MIcopyX2->setFlags(0);
                      MB->push_back(MIcopyX2);
                      MachineInstr *MIcopyY2 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X20).addReg(AArch64::X2).addImm(0).addImm(0);
                      MIcopyY2->setFlags(0);
                      MB->push_back(MIcopyY2);
                      MachineInstr *MIcopyX3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X11).addReg(AArch64::X3).addImm(0).addImm(0);
                      MIcopyX3->setFlags(0);
                      MB->push_back(MIcopyX3);
                      MachineInstr *MIcopyY3 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X21).addReg(AArch64::X3).addImm(0).addImm(0);
                      MIcopyY3->setFlags(0);
                      MB->push_back(MIcopyY3);
                      MachineInstr *MIcopyX4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X12).addReg(AArch64::X4).addImm(0).addImm(0);
                      MIcopyX4->setFlags(0);
                      MB->push_back(MIcopyX4);
                      MachineInstr *MIcopyY4 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X22).addReg(AArch64::X4).addImm(0).addImm(0);
                      MIcopyY4->setFlags(0);
                      MB->push_back(MIcopyY4);
                      MachineInstr *MIcopyX5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X13).addReg(AArch64::X5).addImm(0).addImm(0);
                      MIcopyX5->setFlags(0);
                      MB->push_back(MIcopyX5);
                      MachineInstr *MIcopyY5 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X23).addReg(AArch64::X5).addImm(0).addImm(0);
                      MIcopyY5->setFlags(0);
                      MB->push_back(MIcopyY5);
                      MachineInstr *MIcopyX6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X14).addReg(AArch64::X6).addImm(0).addImm(0);
                      MIcopyX6->setFlags(0);
                      MB->push_back(MIcopyX6);
                      MachineInstr *MIcopyY6 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X24).addReg(AArch64::X6).addImm(0).addImm(0);
                      MIcopyY6->setFlags(0);
                      MB->push_back(MIcopyY6);
                      //MachineInstr *MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X15).addReg(AArch64::X7).addImm(0).addImm(0);
                      //MB->insert(MB->begin(), MIcopyX7);
                      //MIcopyX7 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X25).addReg(AArch64::X7).addImm(0).addImm(0);
                      //MB->insert(MB->begin(), MIcopyX7);

                      MachineInstr *MIcopyX31 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X16).addReg(AArch64::SP).addImm(0).addImm(0);
                      MIcopyX31->setFlags(0);
                      MB->push_back(MIcopyX31);
                      MachineInstr *MIcopyY31 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X26).addReg(AArch64::SP).addImm(0).addImm(0);
                      MIcopyY31->setFlags(0);
                      MB->push_back(MIcopyY31);

                      }
                      }
                      }

                bool isMasterReg(unsigned int reg) {
                  for(unsigned int i = 0; i<functionCallArgs.size(); ++i) {
                    if (functionCallArgs[i] == reg) {
                      return true;
                    }
                  }
                  return false;
                }

                int diagnosisBBNumber;
                int FUNCSIZE;
                int MFSIZE;

                bool runOnMachineFunction(MachineFunction &MF) {
                  FUNCSIZE=100000;
                  if(EnableNEMESIS)
                  {

                    FUNCSIZE = countNumberofStores(MF);



                    MachineBasicBlock* UnrecoverableERROR = makeErrorBB(MF);
                    MachineBasicBlock* UnrecoverableERRORInter = makeErrorBB(MF);
                    MachineBasicBlock* RecoverableErrorBB = makeErrorBB(MF);

                    triplicateInstructions(MF);
                    checkFunctionCalls(MF);
                    //checkFunctionCalls2(MF);

                    changeLabelsStore(MF);
                    changeLabelsCMP(MF);

                    MFSIZE = countNumberofMF(MF);
                    changeCF(MF);

                    storeCheckNAIVE2(MF, UnrecoverableERROR, UnrecoverableERRORInter, RecoverableErrorBB);

                    changeLabelsCALL(MF);

                    checkFunctionCalls2(MF);




                    changeLabelsStore(MF);
                    changeLabelsAfterStore(MF);

                    storeSilenceCheck(MF);


                  }
                  return true;
                }
/*
                void fixCompares(MachineFunction &MF) {
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); MBB != MBE; ++MBB) {
                    llvm::MachineBasicBlock::iterator cmpInst=NULL;
                    llvm::MachineBasicBlock::iterator nextInst=NULL;
                    unsigned int reg1=0;
                    unsigned int reg2=0;
                    for(llvm::MachineBasicBlock::iterator I = MBB->begin(), E=MBB->end(); I!=E; ++I) {
                      if(I->getOpcode() > 96 && I->getOpcode() < 129) {
                        cmpInst = I;
                        nextInst = std::next(cmpInst);
                        if (cmpInst->getOperand(0).isReg()) reg1=cmpInst->getOperand(0).getReg();
                        if (cmpInst->getOperand(1).isReg()) reg2=cmpInst->getOperand(1).getReg();
                        bool reg1IsSpilled=false;
                        bool reg2IsSpilled=false;
                        while(!(nextInst->isConditionalBranch())) {
                          nextInst->dump();
                        }

                      }
                    }
                  }
                }**/

                int countNumberofStores(MachineFunction &MF) {
                  int numStores=0;
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); MBB != MBE; ++MBB) {
                    for(llvm::MachineBasicBlock::iterator I=MBB->begin(), E=MBB->end(); I!=E; ++I) {
                      if(I->mayStore())
                        ++numStores;
                    }
                  }
                  return numStores;
                }

                int countNumberofMF(MachineFunction &MF) {
                  int numMF=0;
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); MBB != MBE; ++MBB) {
                    ++numMF;
                  }
                  return numMF;
                }

                void storeSilenceCheck(MachineFunction &MF) {
                    const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                    int numBBs=0;
                    for(MachineFunction::iterator MBB = ++(MF.begin()), MBE = MF.end(); MBB != MBE && numBBs < FUNCSIZE; ++MBB) {
                      MachineFunction::iterator silentStoreCheckBB = MBB;
                      MachineFunction::iterator errorCheckBB = MBB;
                      llvm::MachineBasicBlock::iterator storeInst = NULL;
                      DebugLoc DL3 = MBB->begin()->getDebugLoc();

                      /*if(MF.getName() != "main" && MBB == MF.begin()) {
                       // DebugLoc DL3 = MB->begin()->getDebugLoc();
                        MachineInstr *MIcopyM0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                        MBB->insert(MBB->begin(), MIcopyM0);
                        MIcopyM0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                        MBB->insert(MBB->begin(), MIcopyM0);
                      } */


                      for(llvm::MachineBasicBlock::iterator I = MBB->begin(), E=MBB->end(); I != E && numBBs < FUNCSIZE; ++I) {
                        if(I->mayStore()) {
                          if(I->getOperand(0).getReg() != AArch64::X28 && I->getOperand(0).getReg() != AArch64::W28 /*&& isGPR64(I->getOperand(0).getReg()) && isGPR64(I->getOperand(1).getReg())*/) {
                            errorCheckBB++;
                            silentStoreCheckBB--;
                            storeInst=I;

                            ++numBBs;
                            int storeOpcode=storeInst->getOpcode();
                            int loadOpcode=0;
                            int cmpOpcode=0;
                            //int loadOpcodeW=0;
                            //int cmpOpcodeW=0;
                            switch(storeOpcode) {
                              case AArch64::STRXui: {
                              loadOpcode = AArch64::LDRXui;
                              cmpOpcode = AArch64::SUBSXrs;

                              break;}
                            case AArch64::STRWui: {   loadOpcode=AArch64::LDRWui;
                                cmpOpcode=AArch64::SUBSWrs;
                              break;}

                            case AArch64::STRBBui: { loadOpcode=AArch64::LDRBBui;
                              cmpOpcode=AArch64::SUBSWrs;


                              break;}

                               // break;
                              //}
                              default: {
                                //storeInst->dump();
                                //errs()<<"Error";
                              }

                            }
                                /*
                            MachineInstr *MIcopyM0 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X17).addReg(AArch64::LR).addImm(0).addImm(0);
                            MIcopyM0->setFlags(0);
                            silentStoreCheckBB->push_back(MIcopyM0);
                            MachineInstr *MIcopyM1 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X27).addReg(AArch64::LR).addImm(0).addImm(0);
                            MIcopyM1->setFlags(0);
                            silentStoreCheckBB->push_back(MIcopyM1);

                            MachineInstr *MIcopyX31 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X16).addReg(AArch64::SP).addImm(0).addImm(0);
                            MIcopyX31->setFlags(0);
                            silentStoreCheckBB->push_back(MIcopyX31);
                            MachineInstr *MIcopyX32 = BuildMI(MF, DL3 , TII->get(AArch64::ADDXri)).addReg(AArch64::X26).addReg(AArch64::SP).addImm(0).addImm(0);
                            MIcopyX32->setFlags(0);
                            silentStoreCheckBB->push_back(MIcopyX32);*/

                            assert(loadOpcode != 0);
                            assert(cmpOpcode != 0);
                            //assert(loadOpcodeW != 0);
                            //assert(cmpOpcodeW != 0);

                            if(isGPR32(storeInst->getOperand(0).getReg()) || (storeInst->getOperand(0).getReg() == AArch64::WZR))
                            {
                              MachineInstr *MIloadVCR = BuildMI(MF, DL3, TII->get(loadOpcode))/*.addReg(AArch64::X18).addReg(storeInst->getOperand(1).getReg()).addImm(0)*/;
                              MIloadVCR->setFlags(0);
                              for(unsigned int opcount=0;opcount<storeInst->getNumOperands();opcount++) {
                                MIloadVCR->addOperand(MF, storeInst->getOperand(opcount));
                              }


                                MIloadVCR->getOperand(0).setReg(AArch64::W18);


                              silentStoreCheckBB->push_back(MIloadVCR);

                              MachineInstr *MIload = BuildMI(MF, DL3, TII->get(loadOpcode))/*.addReg(AArch64::X28).addReg(getSlvReg(storeInst->getOperand(1).getReg())).addImm(0)*/;
                              MIload->setFlags(0);
                              for(unsigned int opcount=0;opcount<storeInst->getNumOperands();opcount++) {
                                MIload->addOperand(MF, storeInst->getOperand(opcount));
                              }

                                MIload->getOperand(0).setReg(AArch64::W28);



                                MIload->getOperand(1).setReg(getSlvReg(storeInst->getOperand(1).getReg()));

                              silentStoreCheckBB->push_back(MIload);


                                MachineInstr *MIcmp = BuildMI(MF, DL3, TII->get(cmpOpcode)).addReg(AArch64::WZR).addReg(AArch64::W28).addReg(storeInst->getOperand(0).getReg()).addImm(0);
                                silentStoreCheckBB->push_back(MIcmp);



                              MachineInstr *MItest = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::EQ).addMBB(errorCheckBB->instr_begin()->getParent());
                              silentStoreCheckBB->addSuccessor(errorCheckBB->instr_begin()->getParent());
                              silentStoreCheckBB->push_back(MItest);


                                MachineInstr *MImove = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(AArch64::W28).addReg(AArch64::W18).addImm(0).addImm(0);
                                silentStoreCheckBB->push_back(MImove);


                            }
                            else {

                            MachineInstr *MIloadVCR = BuildMI(MF, DL3, TII->get(loadOpcode))/*.addReg(AArch64::X18).addReg(storeInst->getOperand(1).getReg()).addImm(0)*/;
                            MIloadVCR->setFlags(0);
                            for(unsigned int opcount=0;opcount<storeInst->getNumOperands();opcount++) {
                              MIloadVCR->addOperand(MF, storeInst->getOperand(opcount));
                            }
                            //if(isGPR32(storeInst->getOperand(0).getReg()) || (storeInst->getOperand(0).getReg() == AArch64::WZR))
                            //{
                              //MIloadVCR->getOperand(0).setReg(AArch64::W18);
                            //}
                            //else {

                              MIloadVCR->getOperand(0).setReg(AArch64::X18);
                            //}
                            silentStoreCheckBB->push_back(MIloadVCR);

                            MachineInstr *MIload = BuildMI(MF, DL3, TII->get(loadOpcode))/*.addReg(AArch64::X28).addReg(getSlvReg(storeInst->getOperand(1).getReg())).addImm(0)*/;
                            MIload->setFlags(0);
                            for(unsigned int opcount=0;opcount<storeInst->getNumOperands();opcount++) {
                              MIload->addOperand(MF, storeInst->getOperand(opcount));
                            }
                            //if(isGPR32(storeInst->getOperand(0).getReg()) || (storeInst->getOperand(0).getReg() == AArch64::WZR))
                            //{
                              //MIload->getOperand(0).setReg(AArch64::W28);
                            //}
                            //else {
                              MIload->getOperand(0).setReg(AArch64::X28);
                            //}
                            //if(isGPR32(storeInst->getOperand(1).getReg()) || (storeInst->getOperand(1).getReg() == AArch64::WZR))
                            //{
                              //MIload->getOperand(1).setReg(getSlvReg(storeInst->getOperand(1).getReg()));
                            //}
                            //else {
                            MIload->getOperand(1).setReg(getSlvReg(storeInst->getOperand(1).getReg()));
                            //}
                            silentStoreCheckBB->push_back(MIload);

                            //if(isGPR32(I->getOperand(0).getReg()) || (I->getOperand(0).getReg() == AArch64::WZR))
                            //{
                              //MachineInstr *MIcmp = BuildMI(MF, DL3, TII->get(cmpOpcode)).addReg(AArch64::WZR).addReg(AArch64::W28).addReg(storeInst->getOperand(0).getReg()).addImm(0);
                              //silentStoreCheckBB->push_back(MIcmp);
                            //}
                            //else {
                            MachineInstr *MIcmp = BuildMI(MF, DL3, TII->get(cmpOpcode)).addReg(AArch64::XZR).addReg(AArch64::X28).addReg(storeInst->getOperand(0).getReg()).addImm(0);
                              silentStoreCheckBB->push_back(MIcmp);
                            //}


                            MachineInstr *MItest = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::EQ).addMBB(errorCheckBB->instr_begin()->getParent());
                            silentStoreCheckBB->addSuccessor(errorCheckBB->instr_begin()->getParent());
                            silentStoreCheckBB->push_back(MItest);

                            //if(isGPR32(I->getOperand(0).getReg()) || (I->getOperand(0).getReg() == AArch64::WZR))
                            //{
                              //MachineInstr *MImove = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(AArch64::W28).addReg(AArch64::W18).addImm(0).addImm(0);
                              //silentStoreCheckBB->push_back(MImove);
                            //}
                            //else {

                            MachineInstr *MImove = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(AArch64::X28).addReg(AArch64::X18).addImm(0).addImm(0);
                              silentStoreCheckBB->push_back(MImove);
                            //}
                            }


                          }

                          }
                      }
                    }
                }
                MachineBasicBlock* makeErrorBB(MachineFunction &MF) {
                  MachineBasicBlock *errorMBB = MF.CreateMachineBasicBlock();
                  MachineFunction::iterator It = (MF.end())->getIterator();
                  MF.insert(It, errorMBB);
                  errorMBB->addSuccessor(errorMBB);

                  return errorMBB;
                }
                MachineBasicBlock* makeDiagnosisBBNaive(MachineFunction &MF, MachineBasicBlock *sourceMB, MachineBasicBlock *UnrecoverableERRORBB, MachineBasicBlock *UnrecoverableERRORInter, MachineBasicBlock *RecoverableErrorBB, MachineBasicBlock::iterator strInst ) {

                  DebugLoc DL3 = sourceMB->instr_begin()->getDebugLoc();
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  MachineBasicBlock *recovery = MF.CreateMachineBasicBlock();
                  MachineFunction::iterator It = (MF.end())->getIterator();
                  MF.insert(It, recovery);
                  sourceMB->addSuccessor(recovery);
                  sourceMB->addSuccessor(recovery);
                  recovery->addSuccessor(sourceMB);

                  MachineInstr *memRestoreInst = BuildMI(MF, DL3, TII->get(strInst->getOpcode()));
                  for(unsigned int opcount = 0; opcount < strInst->getNumOperands(); opcount++) {
                    memRestoreInst->addOperand(MF, strInst->getOperand(opcount));
                  }

                  if(isGPR32(strInst->getOperand(0).getReg()) || (strInst->getOperand(0).getReg() == AArch64::WZR))
                  {
                    memRestoreInst->getOperand(0).setReg(AArch64::W28);
                    recovery->push_back(memRestoreInst);
                  }
                  else {
                    memRestoreInst->getOperand(0).setReg(AArch64::X28);
                    recovery->push_back(memRestoreInst);
                  }

                  unsigned int valReg=strInst->getOperand(0).getReg();
                  unsigned int memReg=strInst->getOperand(1).getReg();
                  /*
                  if((isGPR32(strInst->getOperand(0).getReg())) && (isGPR32(strInst->getOperand(1).getReg() || isGPR64(strInst->getOperand(1).getReg())) ))
                  {

                    if(isGPR32(strInst->getOperand(1).getReg()))
                    {
                      MachineInstr *MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(valReg).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(getSlvReg(valReg)).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(memReg).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(getSlvReg(memReg)).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);

                      recovery->push_back(MoveValRegM);
                      recovery->push_back(MoveValRegD);
                      recovery->push_back(MoveMemRegM);
                      recovery->push_back(MoveMemRegD);
                    }
                    else if(isGPR64(strInst->getOperand(1).getReg())){
                      MachineInstr *MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(valReg).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(getSlvReg(valReg)).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(memReg).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(memReg)).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);

                      recovery->push_back(MoveValRegM);
                      recovery->push_back(MoveValRegD);
                      recovery->push_back(MoveMemRegM);
                      recovery->push_back(MoveMemRegD);

                    }

                  }
                  else if((isGPR64(strInst->getOperand(0).getReg())) && (isGPR32(strInst->getOperand(1).getReg() || isGPR64(strInst->getOperand(1).getReg())) )){
                    if(isGPR32(strInst->getOperand(1).getReg()) )
                    {
                      MachineInstr *MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(valReg).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(valReg)).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(memReg).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDWri)).addReg(getSlvReg(memReg)).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                      recovery->push_back(MoveValRegM);
                      recovery->push_back(MoveValRegD);
                      recovery->push_back(MoveMemRegM);
                      recovery->push_back(MoveMemRegD);
                    }
                    else if(isGPR64(strInst->getOperand(1).getReg())) {
                      MachineInstr *MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(valReg).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(valReg)).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(memReg).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                      MachineInstr *MoveMemRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(memReg)).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                      recovery->push_back(MoveValRegM);
                      recovery->push_back(MoveValRegD);
                      recovery->push_back(MoveMemRegM);
                      recovery->push_back(MoveMemRegD);
                  }
                  }*/
                  if(strInst->getOperand(0).getReg() == AArch64::WZR)
                  {

                  //MachineInstr *MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(valReg).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                  //MachineInstr *MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(valReg)).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                  MachineInstr *MoveMemRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(memReg).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                  MachineInstr *MoveMemRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(memReg)).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                  //recovery->push_back(MoveValRegM);
                  //recovery->push_back(MoveValRegD);
                  recovery->push_back(MoveMemRegM);
                  recovery->push_back(MoveMemRegD);
                  }
                  else {
                    MachineInstr *MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(valReg).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                    MachineInstr *MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(valReg)).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                    MachineInstr *MoveMemRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(memReg).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                    MachineInstr *MoveMemRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(memReg)).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                    recovery->push_back(MoveValRegM);
                    recovery->push_back(MoveValRegD);
                    recovery->push_back(MoveMemRegM);
                    recovery->push_back(MoveMemRegD);
                  }

                  MachineInstr *jumpBack = BuildMI(MF, DL3, TII->get(AArch64::B)).addMBB(sourceMB);
                  recovery->push_back(jumpBack);
                  return recovery;
                }

                MachineBasicBlock* makeDiagnosisBB(MachineFunction &MF, MachineBasicBlock *sourceMB, MachineBasicBlock *UnrecoverableERRORBB, MachineBasicBlock *UnrecoverableERRORInter, MachineBasicBlock *RecoverableErrorBB, MachineBasicBlock::iterator strInst) {
                  DebugLoc DL3 = sourceMB->instr_begin()->getDebugLoc();
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  MachineBasicBlock *recovery = MF.CreateMachineBasicBlock();
                  MachineFunction::iterator It = (MF.end())->getIterator();
                  MF.insert(It, recovery);
                  sourceMB->addSuccessor(recovery);
                  recovery->addSuccessor(sourceMB);

                  MachineInstr *memRestoreInst = BuildMI(MF, DL3, TII->get(strInst->getOpcode()));
                  for (unsigned int opcount=0; opcount < strInst->getNumOperands(); opcount++) {
                    memRestoreInst->addOperand(MF, strInst->getOperand(opcount));
                  }
                  if(isGPR32(strInst->getOperand(0).getReg()) || (strInst->getOperand(0).getReg() == AArch64::WZR))
                  {
                    memRestoreInst->getOperand(0).setReg(AArch64::W28);
                  }
                  else {
                    memRestoreInst->getOperand(0).setReg(AArch64::X28);
                  }
                  recovery->push_back(memRestoreInst);

                  unsigned int valReg = strInst->getOperand(0).getReg();
                  unsigned int memReg = strInst->getOperand(1).getReg();
                  MachineInstr *MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(valReg).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                  MachineInstr *MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(valReg)).addReg(getSlvReg2(valReg)).addImm(0).addImm(0);
                  MachineInstr *MoveMemRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(memReg).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                  MachineInstr *MoveMemRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(memReg)).addReg(getSlvReg2(memReg)).addImm(0).addImm(0);
                  recovery->push_back(MoveValRegM);
                  recovery->push_back(MoveValRegD);
                  recovery->push_back(MoveMemRegM);
                  recovery->push_back(MoveMemRegD);

                  MachineInstr *jumpBack = BuildMI(MF, DL3, TII->get(AArch64::B)).addMBB(sourceMB);
                  recovery->push_back(jumpBack);
                  return recovery;
                }

                void storeCheckNAIVE(MachineFunction &MF, MachineBasicBlock* UnrecoverableERRORBB, MachineBasicBlock* UnrecoverableERRORInter, MachineBasicBlock* RecoverableErrorBB) {
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); MBB != MBE; ++MBB) {
                    llvm::MachineBasicBlock::iterator storeInst = NULL;
                    DebugLoc DL3 = MBB->begin()->getDebugLoc();
                    for(MachineBasicBlock::instr_iterator I = MBB->instr_begin(), I1=MBB->instr_begin(), E=MBB->instr_end(); I != E; ++I, ++I1) {
                      bool check=true;
                      if(I->mayStore())
                        if(I->getOperand(0).getReg() != AArch64::X28) {
                          storeInst=I;
                         // DebugLoc DL3 = MBB->findDebugLoc(I);
                          int storeOpcode = I->getOpcode();
                          int loadOpcode=0;
                          int cmpOpcode=0;
                          //switch(storeOpcode) {
                          //case AArch64::STRXui: {
                            loadOpcode=AArch64::LDRXui;
                            cmpOpcode=AArch64::SUBSXrs;
                            //break;
                          //}
                          //default:{
                            //I->dump();
                            //errs()<<"Error here";
                          //}
                          //}
                          if(check) {
                            assert(loadOpcode != 0);
                            assert(cmpOpcode != 0);
                            MachineInstr *MIload = BuildMI(MF, DL3 , TII->get(loadOpcode))/*.addReg(AArch64::X18).addReg(getSlvReg(storeInst->getOperand(1).getReg())).addImm(0)*/;
                            MIload->setFlags(0);

                            for(unsigned int opcount=0; opcount<storeInst->getNumOperands(); opcount++) {
                              MIload->addOperand(MF, storeInst->getOperand(opcount));
                            }
                            MIload->getOperand(0).setReg(AArch64::X15);
                            MIload->getOperand(1).setReg(getSlvReg(storeInst->getOperand(1).getReg()));


                            //MIload->getOperand(1).setReg(getSlvReg(I->getOperand(1).getReg()));
                            //MIload->getOperand(2).setReg(0);
                            /*
                            for(unsigned int opcount=1; opcount<MIload->getNumOperands(); opcount++) {
                              if(MIload->getOperand(opcount).isReg()) {
                                MIload->getOperand(opcount).setReg(getSlvReg(storeInst->getOperand(opcount).getReg()));
                              }
                            }*/

                            MachineBasicBlock* errorBB = makeDiagnosisBBNaive(MF, I->getParent(), UnrecoverableERRORBB, UnrecoverableERRORInter, RecoverableErrorBB, I);
                            MBB->addSuccessor(errorBB);
                            insertBr(I, MF, errorBB);

                            MachineInstr *MIcmp = BuildMI(MF, DL3, TII->get(cmpOpcode)).addReg(AArch64::XZR).addReg(AArch64::X15).addReg(getSlvReg(I->getOperand(0).getReg())).addImm(0);
                            MBB->insertAfter(I, MIcmp);
                            MBB->insertAfter(I, MIload);
                          }
                        }
                    }
                  }
                }

          void storeCheckNAIVE2(MachineFunction &MF, MachineBasicBlock* UnrecoverableERRORBB, MachineBasicBlock* UnrecoverableERRORInter, MachineBasicBlock* RecoverableErrorBB) {
            const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
            //int numBBs=0;
            for(MachineFunction::iterator MBB = ++(MF.begin()), MBE = MF.end(); MBB != MBE; /*&& numBBs < FUNCSIZE;*/ ++MBB) {
              //MachineFunction::iterator silentStoreCheckBB = MBB;
              //MachineFunction::iterator errorCheckBB = MBB;
              llvm::MachineBasicBlock::iterator storeInst = NULL;
              DebugLoc DL3 = MBB->begin()->getDebugLoc();
              for(llvm::MachineBasicBlock::iterator I = MBB->begin(), E=MBB->end(); I != E ;/*&& numBBs < FUNCSIZE;*/ ++I) {
                if(I->mayStore()) {
                  if(I->getOperand(0).getReg() != AArch64::X28 && I->getOperand(0).getReg() != AArch64::W28/*&& isGPR64(I->getOperand(0).getReg()) && isGPR64(I->getOperand(1).getReg())*/) {
                   // errorCheckBB++;
                    //silentStoreCheckBB--;
                    storeInst=I;

                   // ++numBBs;
                    int storeOpcode=storeInst->getOpcode();
                    int loadOpcode=0;
                    int cmpOpcode=0;
                    //int loadOpcodeW=0;
                    //int cmpOpcodeW=0;
                    switch(storeOpcode) {
                    case AArch64::STRXui: {
                      loadOpcode = AArch64::LDRXui;
                      cmpOpcode = AArch64::SUBSXrs;

                      break;}
                    case AArch64::STRWui: {   loadOpcode=AArch64::LDRWui;
                      cmpOpcode=AArch64::SUBSWrs;
                      break;}

                    case AArch64::STRBBui: { loadOpcode=AArch64::LDRBBui;
                      cmpOpcode=AArch64::SUBSWrs;


                      break;}

                      // break;
                      //}
                    default: {
                      //storeInst->dump();
                      //errs()<<"Error";
                    }

                    }
                    assert(loadOpcode != 0);
                    assert(cmpOpcode != 0);
                    //assert(loadOpcodeW != 0);
                    //assert(cmpOpcodeW != 0);
                    if(isGPR32(I->getOperand(0).getReg()) || (I->getOperand(0).getReg() == AArch64::WZR))
                    {
                      MachineInstr *MIloadVCR = BuildMI(MF, DL3, TII->get(loadOpcode))/*.addReg(AArch64::X18).addReg(storeInst->getOperand(1).getReg()).addImm(0)*/;
                      MIloadVCR->setFlags(0);
                      for(unsigned int opcount=0;opcount<storeInst->getNumOperands();opcount++) {
                        MIloadVCR->addOperand(MF, storeInst->getOperand(opcount));
                      }
                      MIloadVCR->getOperand(0).setReg(AArch64::W18);
                      MIloadVCR->getOperand(1).setReg(getSlvReg(storeInst->getOperand(1).getReg()));

                      MachineBasicBlock* errorBB = makeDiagnosisBBNaive(MF, I->getParent(), UnrecoverableERRORBB, UnrecoverableERRORInter, RecoverableErrorBB, I);
                      MBB->addSuccessor(errorBB);
                      insertBr(I, MF, errorBB);


                      MachineInstr *MIcmp = BuildMI(MF, DL3, TII->get(cmpOpcode)).addReg(AArch64::WZR).addReg(AArch64::W18).addReg(getSlvReg(I->getOperand(0).getReg())).addImm(0);
                      MBB->insertAfter(I, MIcmp);



                      MBB->insertAfter(I, MIloadVCR);
                    }
                    else {
                    MachineInstr *MIloadVCR = BuildMI(MF, DL3, TII->get(loadOpcode))/*.addReg(AArch64::X18).addReg(storeInst->getOperand(1).getReg()).addImm(0)*/;

                    MIloadVCR->setFlags(0);

                    for(unsigned int opcount=0;opcount<storeInst->getNumOperands();opcount++) {
                      MIloadVCR->addOperand(MF, storeInst->getOperand(opcount));
                    }
                    //if(isGPR32(I->getOperand(0).getReg()) || (I->getOperand(0).getReg() == AArch64::WZR))
                    //{
                      //MIloadVCR->getOperand(0).setReg(AArch64::W18);
                    //}
                    //else {
                      MIloadVCR->getOperand(0).setReg(AArch64::X18);
                    //}
                    MIloadVCR->getOperand(1).setReg(getSlvReg(storeInst->getOperand(1).getReg()));



                    MachineBasicBlock* errorBB = makeDiagnosisBBNaive(MF, I->getParent(), UnrecoverableERRORBB, UnrecoverableERRORInter, RecoverableErrorBB, I);
                    MBB->addSuccessor(errorBB);
                    insertBr(I, MF, errorBB);

                    //if(isGPR32(I->getOperand(0).getReg()) || (I->getOperand(0).getReg() == AArch64::WZR))
                    //{
                      //MachineInstr *MIcmp = BuildMI(MF, DL3, TII->get(cmpOpcode)).addReg(AArch64::WZR).addReg(AArch64::W18).addReg(getSlvReg(I->getOperand(0).getReg())).addImm(0);
                      //MBB->insertAfter(I, MIcmp);
                    //}
                    //else{
                    MachineInstr *MIcmp = BuildMI(MF, DL3, TII->get(cmpOpcode)).addReg(AArch64::XZR).addReg(AArch64::X18).addReg(getSlvReg(I->getOperand(0).getReg())).addImm(0);
                      MBB->insertAfter(I, MIcmp);
                    //}

                    MBB->insertAfter(I, MIloadVCR);
                    }
                    }


                    }
                }
                }
                }


                void checkFunctionCallArguments(MachineBasicBlock::iterator MI, MachineFunction &MF) {

                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  DebugLoc DL3 = MI->getDebugLoc();
                  MachineFunction::iterator MBB;

                  MachineBasicBlock *diagnosisBB = MF.CreateMachineBasicBlock();
                  MachineFunction::iterator It = (MF.end())->getIterator();
                  MF.insert(It, diagnosisBB);
                  MI->getParent()->addSuccessor(diagnosisBB);

                  MachineInstr *MIcmp = NULL;
                  MachineInstr *MItest = NULL;

                  for(auto& Reg : functionCallArgs1 ) {

                    MIcmp = BuildMI(MF, DL3, TII->get(AArch64::SUBSXrs)).addReg(AArch64::XZR).addReg(Reg).addReg(getSlvReg(Reg)).addImm(0);
                    MI->getParent()->insert(MI, MIcmp);
                    MItest = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::NE).addMBB(diagnosisBB);
                    MI->getParent()->insert(MI, MItest);
                  }

                  for(auto& Reg : functionCallArgs1) {
                    MachineInstr *checkArgM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(Reg).addReg(getSlvReg2(Reg)).addImm(0).addImm(0);
                    MachineInstr *checkArgD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(Reg)).addReg(getSlvReg2(Reg)).addImm(0).addImm(0);
                    diagnosisBB->push_back(checkArgM);
                    diagnosisBB->push_back(checkArgD);
                  }

                  MachineInstr *jumpBack = BuildMI(MF, DL3, TII->get(AArch64::B)).addMBB(MI->getParent());
                  diagnosisBB->push_back(jumpBack);
                  diagnosisBB->addSuccessor(MI->getParent());

                  MachineBasicBlock *diagnosisBB1 = MF.CreateMachineBasicBlock();
                  It = (MF.end())->getIterator();
                  MF.insert(It, diagnosisBB1);
                  MI->getParent()->addSuccessor(diagnosisBB1);

                  MIcmp = NULL;
                  MItest = NULL;

                  for(auto&Reg : functionCallArgs1) {

                    MIcmp = BuildMI(MF, DL3, TII->get(AArch64::SUBSXrs)).addReg(AArch64::XZR).addReg(Reg).addReg(getSlvReg2(Reg)).addImm(0);
                    MI->getParent()->insert(MI, MIcmp);
                    MItest = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::NE).addMBB(diagnosisBB1);
                    MI->getParent()->insert(MI, MItest);
                  }

                  for(auto& Reg : functionCallArgs1) {
                    MachineInstr *checkArgM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(Reg).addReg(getSlvReg(Reg)).addImm(0).addImm(0);
                    MachineInstr *checkArgD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg2(Reg)).addReg(getSlvReg(Reg)).addImm(0).addImm(0);
                    diagnosisBB1->push_back(checkArgM);
                    diagnosisBB1->push_back(checkArgD);
                  }

                  jumpBack = BuildMI(MF, DL3, TII->get(AArch64::B)).addMBB(MI->getParent());
                  diagnosisBB1->push_back(jumpBack);
                  diagnosisBB1->addSuccessor(MI->getParent());

                }

                void insertBr (MachineBasicBlock::iterator I, MachineFunction &MF, MachineBasicBlock* errorBB )
                {
                  MachineBasicBlock *MB=I->getParent();
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  DebugLoc DL3= I->getDebugLoc();
                  MachineInstr *MItest = BuildMI(MF, DL3 , TII->get(AArch64::Bcc)).addImm(AArch64CC::NE).addMBB(errorBB);
                  MB->addSuccessor(errorBB);
                  MB->insertAfter(I, MItest);
                  //MB->updateTerminator();
                }

                MachineBasicBlock* splitBlockAfterInstr(MachineBasicBlock::iterator MI, MachineFunction &MF) {
                  // Create a new MBB for the code after the OrigBB.
                  MachineBasicBlock *NewBB =
                      MF.CreateMachineBasicBlock( (MI->getParent())->getBasicBlock());
                  MachineFunction::iterator MBBI = (MI->getParent())->getIterator();

                  //(MI->getParent())->addSuccessor(NewBB);
                  MBBI->addSuccessor(NewBB);

                  for (MachineBasicBlock::succ_iterator succ = MBBI->succ_begin(), succ_E = MBBI->succ_end(); succ != succ_E;/*++succ*/) {
                    MachineBasicBlock::succ_iterator target = succ;

                    if((*target) == NewBB) {
                      ++succ;
                      continue;
                    }

                    MachineBasicBlock *targetBeforeRemove = *target;
                    NewBB->addSuccessor(targetBeforeRemove);

                    MBBI->removeSuccessor(target);
                  }

                  //++MBBI;
                  MF.insert(std::next(MBBI), NewBB);

                  NewBB->splice(NewBB->end(), &(*MBBI), std::next(MI), MBBI->end());

                  /*
                  // Splice the instructions starting with MI over to NewBB.
                  MachineBasicBlock::iterator T= (MI->getParent())->instr_end(), E= (MI->getParent())->instr_end();
                  if ( T !=  (MI->getParent())->instr_begin()) T--;
                  if (MI !=E && MI != T)
                    NewBB->splice(NewBB->end(),  (MI->getParent()), MI,  (MI->getParent())->end());
                    //else  errs() << "FIX it by hand. I CAN NOT SPLIT THIS BASICBLOCK\n F: "<< MF.getName() << "BB: " << MI->getParent()->getName() << "Instr: " << *MI << "\n";
                  else
                  {
                    errs() << "FIX it by hand. I CAN NOT SPLIT THIS BASICBLOCK\n F: "<< MF.getName() << "BB: " << MI->getParent()->getName() << "Instr: " << *MI << "\n";
                    NewBB->splice(NewBB->end(),  (MI->getParent()), MI,  (MI->getParent())->end());//HWISOO_DEBG
                  }
                  //HWISOO: else means that it is end or end-1
                  //In that case, how can we split?

                  **/

                  return NewBB;
                }

                void changeLabelsStore(MachineFunction &MF){
                  int numBBs=0;
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); (MBB != MBE) /*&& (numBBs < FUNCSIZE)*/; ++MBB, ++numBBs) {
                    for ( llvm::MachineBasicBlock::iterator I=MBB->begin(), E=MBB->end(); I !=E ; ++I){
                      if ( (I->mayStore() ) /*&& std::prev(I) != E*/ && I != MBB->begin()){
                        MachineBasicBlock* newBB=splitBlockAfterInstr(std::prev(I), MF);
                        newBB->setHasAddressTaken();

                        break;
                      }
                      /*else if ( (I->mayStore() ) && std::next(I) == E && I != MBB->begin()){
                        MachineBasicBlock* newBB=MF.CreateMachineBasicBlock((I->getParent())->getBasicBlock());
                        MachineFunction::iterator It = MBB->getIterator();
                        //It++;
                        if (It != MF.end())
                          MF.insert(It,newBB);
                        else MF.push_back(newBB);
                        newBB->splice(newBB->end(), I->getParent(), I);
                        MBB->addSuccessor(newBB);
                        newBB->setHasAddressTaken();
                        break;
                      }*/
                    }
                  }
                }// end of function
                void changeLabelsAfterStore(MachineFunction &MF){
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); MBB != MBE; ++MBB) {
                    for ( llvm::MachineBasicBlock::iterator I=MBB->begin(), E=MBB->end(); I !=E ; ++I){
                      if ( /*(I->getOpcode() == AArch64::SUBSXrs )*/ (I->getOperand(0).getReg() == AArch64::XZR || I->getOperand(0).getReg() == AArch64::WZR) /*&& std::next(I) != E*/ && I != MBB->begin()) { // error detectors
                        if (I->getOperand(1).getReg() == AArch64::X18 || I->getOperand(1).getReg() == AArch64::W18 ) {
                          MachineBasicBlock *newBB = splitBlockAfterInstr(std::prev(I), MF);
                          newBB->setHasAddressTaken();
                          break;
                        }
                      }
                      //if ( /*(I->getOpcode()==AArch64::SUBSXrs )*/(I->getOperand(0).getReg() == AArch64::XZR || I->getOperand(0).getReg() == AArch64::WZR) && std::next(I) == E && I != MBB->begin()) { // error detectors
                        /*if (I->getOperand(1).getReg() == AArch64::X18 || I->getOperand(1).getReg() == AArch64::W18 ) {
                          MachineBasicBlock *newBB = MF.CreateMachineBasicBlock(
                              (I->getParent())->getBasicBlock());
                          MachineFunction::iterator It = MBB->getIterator();
                          // It++;
                          if (It != MF.end())
                            MF.insert(It, newBB);
                          else
                            MF.push_back(newBB);
                          newBB->splice(newBB->end(), I->getParent(), I);
                          MBB->addSuccessor(newBB);
                          newBB->setHasAddressTaken();
                          break;
                        }
                      }*/
                    }
                  }
                }// end of function
                void changeLabelsCMP(MachineFunction &MF){
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); MBB != MBE; ++MBB) {
                    for ( llvm::MachineBasicBlock::iterator I=MBB->begin(), E=MBB->end(); I !=E ; ++I){
                      if ( /*( I->getOpcode() > 96 && I->getOpcode() < 129) isCompare*/I->isCompare() /*&& std::next(I) != E*/ && I != MBB->begin()){
                        MachineBasicBlock* newBB=splitBlockAfterInstr(std::prev(I), MF);
                        newBB->setHasAddressTaken();
                        break;
                      }
                      //if (/* ( I->getOpcode() > 96 && I->getOpcode() < 129)*/I->isCompare() && std::next(I) == E && I != MBB->begin()){
                        /*MachineBasicBlock* newBB=MF.CreateMachineBasicBlock((I->getParent())->getBasicBlock());
                        MachineFunction::iterator It = MBB->getIterator();
                        if (It != MF.end())
                          MF.insert(It,newBB);
                        else MF.push_back(newBB);
                        newBB->splice(newBB->end(), I->getParent(), I);
                        MBB->addSuccessor(newBB);
                        newBB->setHasAddressTaken();
                        break;
                      }*/

                    }
                  }
                }// end of function

                void changeLabelsCALL(MachineFunction &MF){
                  int numBBs=0;
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); (MBB != MBE) && (numBBs < FUNCSIZE); ++MBB, ++numBBs) {
                    for ( llvm::MachineBasicBlock::iterator I=MBB->begin(), E=MBB->end(); I !=E ; ++I){
                      if ( ( I->isCall())  && /*std::next(I) != E &&*/ I != MBB->begin()){
                        MachineBasicBlock* newBB=splitBlockAfterInstr(std::prev(I), MF);
                        newBB->setHasAddressTaken();
                        break;
                      }
                      /*if ( I->isCall() && std::next(I) == E && I != MBB->begin()){
                        MachineBasicBlock* newBB=MF.CreateMachineBasicBlock((I->getParent())->getBasicBlock());
                        MachineFunction::iterator It = MBB->getIterator();

                        if (It != MF.end())
                          MF.insert(It,newBB);
                        else MF.push_back(newBB);
                        MBB->splice(newBB->end(), I->getParent(), I);
                        MBB->addSuccessor(newBB);
                        newBB->setHasAddressTaken();
                        break;
                      }*/

                    }
                  }
                }// end of function
                //int diagnosisBBNumber;
                void changeCF(MachineFunction &MF) {
                  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
                  int BBcounters=0,flag=0;
                  for(MachineFunction::iterator MBB = MF.begin(), MBE = MF.end(); (MBB != MBE) && (BBcounters < MFSIZE); ++MBB, ++BBcounters) {
                    DebugLoc DL3 = MBB->begin()->getDebugLoc();
                    llvm::MachineBasicBlock::iterator cmpInst=NULL;
                    llvm::MachineBasicBlock::iterator brInst=NULL;
                    for(llvm::MachineBasicBlock::iterator I=MBB->begin(), E=MBB->end(); I!=E; ++I) {
                      if(I->isCompare()) {
                        //if(isMasterReg(I->getOperand(0).getReg())) {
                          cmpInst = I;
                          flag++;
                        //}
                      }
                        //if(isMasterReg(I->getOperand(0).getReg()))
                          //cmpInst=I;

                      if(I->isConditionalBranch() && flag>0 && cmpInst != NULL){

                        assert(cmpInst);
                        flag=0;
                        MachineInstr *shadowCMP = MF.CloneMachineInstr(&*cmpInst);
                        shadowCMP->setFlags(0);
                        for(unsigned int opcount=0; opcount < cmpInst->getNumOperands(); opcount++) {
                          if (cmpInst->getOperand(opcount).isReg())
                            shadowCMP->getOperand(opcount).setReg(getSlvReg(cmpInst->getOperand(opcount).getReg()));
                          }
                        MachineInstr *shadowCMP1 = MF.CloneMachineInstr(&*cmpInst);
                        shadowCMP1->setFlags(0);
                        for(unsigned int opcount=0; opcount < cmpInst->getNumOperands(); opcount++) {
                          if (cmpInst->getOperand(opcount).isReg())
                            shadowCMP1->getOperand(opcount).setReg(getSlvReg(cmpInst->getOperand(opcount).getReg()));
                          }

                        MachineBasicBlock *diagnosisBB = MF.CreateMachineBasicBlock();
                        MBB->addSuccessor(diagnosisBB);
                        MachineFunction::iterator pos = (MF.end())->getIterator();
                        MF.insert(pos, diagnosisBB);

                        /*MachineInstr *MICFDETECTION = BuildMI(MF, DL3, TII->get(I->getOpcode()));
                        MICFDETECTION->setFlags(0);
                        for(unsigned int opcount=0;opcount<I->getNumOperands();opcount++) {
                          MICFDETECTION->addOperand(MF, I->getOperand(opcount));
                        }*/
                        MachineInstr *MICFDETECTION = MF.CloneMachineInstr(&*I);
                        MICFDETECTION->getOperand(1).setMBB(diagnosisBB);

                        AArch64CC::CondCode CC = (AArch64CC::CondCode)MICFDETECTION->getOperand(0).getImm();
                        MICFDETECTION->getOperand(0).setImm(AArch64CC::getInvertedCondCode(CC));

                        MachineBasicBlock *NewBB = MF.CreateMachineBasicBlock();
                        MBB->addSuccessor(NewBB);
                        MachineFunction::iterator It = (MF.end())->getIterator();
                        MF.insert(It, NewBB);

                        //NewBB->addSuccessor(MICFDETECTION->getOperand(1).getMBB());
                        //NewBB->addSuccessor(diagnosisBB);

                        NewBB->insert(NewBB->instr_begin(),MICFDETECTION);
                        NewBB->insert(NewBB->instr_begin(),shadowCMP);


                        /*MachineInstr *MICFDETECTION = MF.CloneMachineInstr(&*I);
                        MICFDETECTION->getOperand(1).setMBB(diagnosisBB);

                        AArch64CC::CondCode CC = (AArch64CC::CondCode)MICFDETECTION->getOperand(0).getImm();
                        MICFDETECTION->getOperand(0).setImm(AArch64CC::getInvertedCondCode(CC));

                        NewBB->addSuccessor(MICFDETECTION->getOperand(1).getMBB());
                        //NewBB->addSuccessor(diagnosisBB);
                        NewBB->push_back(MICFDETECTION);*/






                        unsigned int regOperand=0;
                        MachineInstr *MoveValRegM=NULL, *MoveValRegD=NULL;
                        for (unsigned int opcount=0; opcount < cmpInst->getNumOperands(); opcount++) {
                          if(cmpInst->getOperand(opcount).isReg())  {
                            regOperand = cmpInst->getOperand(opcount).getReg();
                            if(registersMtoS.find(regOperand) != registersMtoS.end()) {
                              MoveValRegM = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(regOperand).addReg(getSlvReg2(regOperand)).addImm(0).addImm(0);
                              MoveValRegD = BuildMI(MF, DL3, TII->get(AArch64::ADDXri)).addReg(getSlvReg(regOperand)).addReg(getSlvReg2(regOperand)).addImm(0).addImm(0);
                              diagnosisBB->push_back(MoveValRegM);
                              diagnosisBB->push_back(MoveValRegD);
                            }
                          }
                        }

                        MachineInstr *jumpBack = BuildMI(MF, DL3, TII->get(AArch64::B)).addMBB(I->getParent());
                        diagnosisBB->addSuccessor(I->getParent());
                        diagnosisBB->push_back(jumpBack);


                        //MachineInstr *MICFDETECTION=NULL;



                        //AArch64CC::CondCode CC = (AArch64CC::CondCode)MICFDETECTION->getOperand(0).getImm();
                        //MICFDETECTION->getOperand(0).setImm(AArch64CC::getInvertedCondCode(CC));

                        /*for(unsigned int opcount=0; opcount < I->getNumOperands(); opcount++)
                        {
                        if (I->getOpcode() == AArch64::Bcc && AArch64CC::getImm() == AArch64CC::NE)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::EQ).addMBB(diagnosisBB);
                        else if (I->getOpcode() == AArch64::Bcc && AArch64CC::getCondCodeName(I) == AArch64CC::EQ)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::NE).addMBB(diagnosisBB);
                        /*else if (I->getOpcode() == AArch64::Bcc && I->getOperand(opcount).getPredicate() == AArch64CC::GE)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::LE).addMBB(diagnosisBB);
                        else if (I->getOpcode() == AArch64::Bcc && I->getOperand(opcount).getPredicate() == AArch64CC::LE)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::GE).addMBB(diagnosisBB);
                        else if (I->getOpcode() == AArch64::Bcc && I->getOperand(opcount).getPredicate() == AArch64CC::HS)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::LS).addMBB(diagnosisBB);
                        else if (I->getOpcode() == AArch64::Bcc && I->getOperand(opcount).getPredicate() == AArch64CC::LS)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::HS).addMBB(diagnosisBB);
                        else if (I->getOpcode() == AArch64::Bcc && I->getOperand(opcount).getPredicate() == AArch64CC::GT)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::LT).addMBB(diagnosisBB);
                        else if (I->getOpcode() == AArch64::Bcc && I->getOperand(opcount).getPredicate() == AArch64CC::LT)
                          MICFDETECTION = BuildMI(MF, DL3, TII->get(AArch64::Bcc)).addImm(AArch64CC::GT).addMBB(diagnosisBB);
                        else
                          continue;
                        }*/


                        //assert (MICFDETECTION != NULL);


                        //brInst = I++;
                        //NewBB->push_back(MICFDETECTION);
                        //NewBB->addSuccessor(MICFDETECTION->getOperand(1).getMBB());
                        MachineInstr *MIjump = BuildMI(MF, DL3, TII->get(AArch64::B)).addMBB(I->getOperand(1).getMBB());
                        NewBB->push_back(MIjump);
                        NewBB->addSuccessor(I->getOperand(1).getMBB());
                        MachineInstr *MIjump1 = BuildMI(MF, DL3, TII->get(AArch64::B)).addMBB(I->getOperand(1).getMBB());
                        NewBB->push_back(MIjump1);


                        I->getOperand(1).setMBB(NewBB);


                        //MachineInstr *MIcfErrorDetection1 = BuildMI(MF, DL3, TII->get(I->getOpcode())).addImm(AArch64CC::NE).addMBB(diagnosisBB);
                        //MBB->push_back(shadowCMP1);
                        //MBB->push_back(MIcfErrorDetection1);

                        MBB++;
                        cmpInst=NULL;
                        break;
                      }
                    }
                  }
                }




        };

char Nemesis::ID = 0;
FunctionPass *createNemesisPass()
{

  return new Nemesis();
}
}
