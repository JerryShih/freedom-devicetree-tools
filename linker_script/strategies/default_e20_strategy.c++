/* Copyright (c) 2019 SiFive Inc. */
/* SPDX-License-Identifier: Apache-2.0 */

#include "default_e20_strategy.h"

#include <layouts/default_layout.h>
#include <layouts/scratchpad_layout.h>
#include <layouts/ramrodata_layout.h>

bool DefaultE20Strategy::valid(const fdt &dtb, list<Memory> available_memories)
{
  bool has_testram = false;

  /* Look through the available memories to determine if this is a valid strategy */
  for (auto it = available_memories.begin(); it != available_memories.end(); it++) {
    if ((*it).compatible == "sifive,testram0") {
      has_testram = true;
    }
  }

  return has_testram;
}

LinkerScript DefaultE20Strategy::create_layout(const fdt &dtb, list<Memory> available_memories,
                                             LinkStrategy link_strategy)
{
  Memory rom_memory;

  /* Map the available memories to the ROM, RAM, and ITIM */

  for (auto it = available_memories.begin(); it != available_memories.end(); it++) {
    if ((*it).compatible == "sifive,testram0") {
      rom_memory = *it;
      rom_memory.name = "ram";
      rom_memory.attributes = "wxa!ri";
    }
  }

  /* Generate the layouts */
  print_chosen_strategy("DefaultE20Strategy", link_strategy, rom_memory, rom_memory, rom_memory);

  switch (link_strategy) {
  default:
  case LINK_STRATEGY_DEFAULT:
    return DefaultLayout(dtb, rom_memory, rom_memory, rom_memory, rom_memory);
    break;

  case LINK_STRATEGY_SCRATCHPAD:
    return ScratchpadLayout(dtb, rom_memory, rom_memory, rom_memory, rom_memory);
    break;

  case LINK_STRATEGY_RAMRODATA:
    return RamrodataLayout(dtb, rom_memory, rom_memory, rom_memory, rom_memory);
    break;
  }
}

