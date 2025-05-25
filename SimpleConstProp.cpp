//===- SimpleConstProp.cpp - Tiny constant‑prop & phi‑cleanup pass -------===//
//
//  This out‑of‑tree LLVM pass shows the *minimum* machinery needed to
//  (1) constant‑fold trivial arithmetic (x+0, x*1, etc.)
//  (2) remove redundant phi‑nodes whose incoming values are identical.
//
//  Build (LLVM ≥ 17, new pass manager):
//    add_llvm_pass_plugin(SimpleConstProp)
//
//  Run:
//    opt -load-pass-plugin ./libSimpleConstProp.so -passes='simple-constprop' \
//        -S input.ll -o output.ll
//===----------------------------------------------------------------------===//
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"   

#define DEBUG_TYPE "simple-constprop"

using namespace llvm;
using namespace llvm::PatternMatch;

namespace {
struct SimpleConstPropPass : public PassInfoMixin<SimpleConstPropPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {


    llvm::errs() << "SimpleConstProp running on " << F.getName() << "\n";

    bool Changed = false;

    // --- 1. Trivial constant‑prop / inst‑simplifications --------------
    SmallVector<Instruction *, 8> ToErase;
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        // x + 0  -> x   ;  0 + x -> x
        // x * 1  -> x   ;  1 * x -> x
        Value *X;
        
        if (match(&I, m_Add(m_Value(X), m_Zero())) ||
            match(&I, m_Add(m_Zero(), m_Value(X))) ||
            match(&I, m_Mul(m_Value(X), m_One())) ||
            match(&I, m_Mul(m_One(), m_Value(X)))) {
          I.replaceAllUsesWith(X);
          ToErase.push_back(&I);
          Changed = true;
        }
      }
    }

    // Actually erase now to keep the iterator sane
    for (Instruction *Dead : ToErase)
      Dead->eraseFromParent();

    // --- 2. Remove redundant phi‑nodes --------------------------------

    
    for (BasicBlock &BB : F) {
      if (auto *PN = dyn_cast<PHINode>(BB.begin())) {
        // Only look at *simple* case: first instruction and all incoming
        // values identical.
        bool AllSame = true;
        Value *First = PN->getIncomingValue(0);
        for (unsigned i = 1, e = PN->getNumIncomingValues(); i < e; ++i)
          if (PN->getIncomingValue(i) != First) {
            AllSame = false;
            break;
          }
        if (AllSame) {
          PN->replaceAllUsesWith(First);
          PN->eraseFromParent();
          Changed = true;
        }
      }
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};
} // namespace

// --------- Pass registration glue (new PM) -----------------------------
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "SimpleConstProp", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "simple-constprop") {
                    FPM.addPass(SimpleConstPropPass());
                    return true;
                  }
                  return false;
                });
          }};
}
