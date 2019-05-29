/* Copyright (c) 2019 SiFive Inc. */
/* SPDX-License-Identifier: Apache-2.0 */

#include <iostream>
#include <iomanip>
#include <string>

#include "map_strategy.h"

using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::setw;
using std::string;

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

