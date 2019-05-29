/* Copyright (c) 2019 SiFive Inc. */
/* SPDX-License-Identifier: Apache-2.0 */

#include <iostream>
#include <iomanip>
#include <string>
#include <regex>

#include "map_strategy.h"

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::regex;
using std::setw;
using std::string;

static list<string> testram_compats = {
  "sifive,testram0",
  "sifive,axi4-periph-port",
  "sifive,ahb-periph-port",
  "sifive,tl-periph-port",
  "sifive,axi4-sys-port",
  "sifive,ahb-sys-port",
  "sifive,tl-sys-port",
  "sifive,axi4-mem-port",
  "sifive,ahb-mem-port",
  "sifive,tl-mem-port",
  "sifive,periph-port",
  "sifive,sys-port",
  "sifive,mem-port",
};

void MapStrategy::print_chosen_strategy(string name, LinkStrategy layout,
                                        Memory ram, Memory rom, Memory itim)
{
  cout << hex;
  cout << "Using strategy " << name;
  cout << " with ";
  switch(layout) {
  default:
  case LINK_STRATEGY_DEFAULT:
    cout << "default";
    break;
  case LINK_STRATEGY_SCRATCHPAD:
    cout << "scratchpad";
    break;
  case LINK_STRATEGY_RAMRODATA:
    cout << "ramrodata";
    break;
  }
  cout << " layout" << endl;
  cout << "\tRAM:  " << setw(18) << ram.compatible << " - 0x" << ram.base << endl;
  cout << "\tROM:  " << setw(18) << rom.compatible << " - 0x" << rom.base << endl;
  cout << "\tITIM: " << setw(18) << itim.compatible << " - 0x" << itim.base << endl;
  cout << dec;
}

bool MapStrategy::has_testram(list<Memory> memories) {
  memories.sort();

  for (auto mem = memories.begin(); mem != memories.end(); mem++) {
    for (auto c = testram_compats.begin(); c != testram_compats.end(); c++) {
      if (mem->compatible == *c) {
        return true;
      }
    }
  }

  return false;
}

Memory MapStrategy::find_testram(list<Memory> memories) {
  Memory testram;

  memories.sort();

  for (auto mem = memories.begin(); mem != memories.end(); mem++) {
    for (auto c = testram_compats.begin(); c != testram_compats.end(); c++) {
      if (mem->compatible == *c) {
        return *mem;
      }
    }
  }
}

