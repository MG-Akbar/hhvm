/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <algorithm>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

Vlabel Vunit::makeBlock(AreaIndex area) {
  auto i = blocks.size();
  blocks.emplace_back(area);
  return Vlabel{i};
}

Vlabel Vunit::makeScratchBlock() {
  return makeBlock(AreaIndex::Main);
}

void Vunit::freeScratchBlock(Vlabel l) {
  // This will leak blocks if anything's been added since the corresponding
  // call to makeScratchBlock(), but it's harmless.
  if (l == blocks.size() - 1) blocks.pop_back();
}

Vtuple Vunit::makeTuple(VregList&& regs) {
  auto i = tuples.size();
  tuples.emplace_back(std::move(regs));
  return Vtuple{i};
}

Vtuple Vunit::makeTuple(const VregList& regs) {
  auto i = tuples.size();
  tuples.emplace_back(regs);
  return Vtuple{i};
}

VcallArgsId Vunit::makeVcallArgs(VcallArgs&& args) {
  VcallArgsId i(vcallArgs.size());
  vcallArgs.emplace_back(std::move(args));
  return i;
}

Vreg Vunit::makeConst(bool b) {
  auto it = constants.find(b);
  if (it != constants.end()) return it->second;
  return constants[b] = makeReg();
}

Vreg Vunit::makeConst(uint32_t v) {
  auto it = constants.find(v);
  if (it != constants.end()) return it->second;
  return constants[v] = makeReg();
}

Vreg Vunit::makeConst(uint64_t v) {
  auto it = constants.find(v);
  if (it != constants.end()) return it->second;
  return constants[v] = makeReg();
}

Vreg Vunit::makeConst(double d) {
  union { double d; uint64_t i; } u;
  u.d = d;
  return makeConst(u.i);
}

Vreg Vunit::makeConst(Vptr p) {
  auto it = constants.find(p);
  if (it != constants.end()) return it->second;
  return constants[p] = makeReg();
}

bool Vunit::needsRegAlloc() const {
  if (next_vr > Vreg::V0) return true;

  for (auto& block : blocks) {
    for (auto& inst : block.code) {
      if (inst.op == Vinstr::copyargs) return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
}}
