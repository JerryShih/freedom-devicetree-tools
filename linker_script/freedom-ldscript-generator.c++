/* Copyright 2018 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <fstream>
#include <iostream>
#include <list>
#include <regex>

#include <fdt.h++>

#include <memory.h>
#include <strategies/chosen_strategy.h>
#include <strategies/default_bullet_arty.h>
#include <strategies/default_bullet_strategy.h>
#include <strategies/default_e20_arty_strategy.h>
#include <strategies/default_e20_strategy.h>
#include <strategies/default_e21_arty_strategy.h>
#include <strategies/default_e21_strategy.h>
#include <strategies/default_rocket_arty.h>
#include <strategies/default_rocket_strategy.h>

using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

void show_usage(string name)
{
  std::cerr << "Usage: " << name << " <option(s)>\n"
            << "General Options:\n"
            << "    -h,--help                               Show this help message\n"
            << "Input/Output Options:\n"
            << "    -d,--dtb <eg. xxx.dtb>                  Specify fullpath to the DTB file\n"
            << "    -l,--linker <eg. zzz.lds>               Generate linker file to fullpath filename\n"
            << "Linker Script Generation Options:\n"
            << "    --scratchpad                            Execute codes from ram, -l option\n"
            << "    --ramrodata                             Rodata in ram data section, -l option\n"
            << endl;
}

int main (int argc, char* argv[])
{
  /*
   * Parse Arguments
   */

  string dtb_file;
  string linker_file;
  string release;

  bool ramrodata = false;
  bool scratchpad = false;

  if ((argc < 2) || (argc > 7)) {
    show_usage(argv[0]);
    return 1;
  } else {
    for (int i = 1; i < argc; ++i) {
      string arg = argv[i];

      if ((arg == "-d") || (arg == "--dtb")) {
        if (i + 1 < argc) {
          dtb_file = argv[++i];
        } else {
          std::cerr << "--dtb option requires file path." << std::endl;
          show_usage(argv[0]);
          return 1;
        }
      } else if ((arg == "-l") || (arg == "--linker")) {
        if (i + 1 < argc) {
          linker_file = argv[++i];
        } else {
          std::cerr << "--linker option requires file path." << std::endl;
          show_usage(argv[0]);
          return 1;
        }
      } else if (arg == "--ramrodata") {
        ramrodata = true;
      } else if (arg == "--scratchpad") {
        scratchpad = true;
      } else if (arg == "-r") {
        if (i + 1 < argc) {
          release = argv[++i];
        }
      } else {
        show_usage(argv[0]);
        return 1;
      }
    }
  }

  if (dtb_file.empty()) {
      std::cerr << "--dtb option requires file path." << std::endl;
      show_usage(argv[0]);
      return 1;
  }
  fdt dtb(dtb_file);

  /*
   * Find memory devices
   */

  auto extract_mem_map = [](Memory &m, const node &n) {
    if(n.field_exists("reg-names")) {
      n.named_tuples(
        "reg-names",
        "reg", "mem",
        tuple_t<target_addr, target_size>(),
        [&](target_addr b, target_size s) {
          m.base = b;
          m.size = s;
        });
    } else if (n.field_exists("ranges")) {
      n.maybe_tuple(
        "ranges",
        tuple_t<target_addr, target_addr, target_size>(),
        [&]() {},
        [&](target_addr src, target_addr dest, target_size len) {
          if (src != 0 && len != 0) {
            m.base = src;
            m.size = len;
          }
        });
    } else {
      n.maybe_tuple(
        "reg",
        tuple_t<target_addr, target_size>(),
        [&]() {},
        [&](target_addr b, target_size s) {
          m.base = b;
          m.size = s;
        });
    }
  };

  list<string> memory_devices = {
    "sifive,dtim0",
    "sifive,itim0",
    "sifive,spi0",
    "sifive,sram0",
    "sifive,testram0",
    "memory",
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
  list<Memory> memories;

  /* Get the node pointed to by metal,entry */
  string entry_handle;
  uint32_t entry_offset = 0;;
  dtb.chosen(
    "metal,entry",
    tuple_t<node, uint32_t>(),
    [&](node n, uint32_t offset) {
      entry_handle = n.handle();
      entry_offset = offset;
    });

  for (auto it = memory_devices.begin(); it != memory_devices.end(); it++) {
    dtb.match(
        std::regex(*it),
        [&](const node n) {
          Memory mem(*it);
          extract_mem_map(mem, n);

          /* If we've requested an offset, add it to the node */
          if(n.handle() == entry_handle) {
            mem.base += entry_offset;
            mem.size -= entry_offset;
          }

          memories.push_back(mem);
        });
  }

  /* Sort memories (by base address) and then reverse so that the last
   * matching memory for each type is used by the Strategy*/
  memories.sort();
  memories.reverse();

  /*
   * Mapping Strategies
   *
   * Strategies are ordered from most to least complicated, because simpler
   * memory maps are more likely to match erroneously on more complicated
   * designs
   */

  list<MapStrategy *> strategies;

  /* The chosen node supersedes all default strategies */
  strategies.push_back(new ChosenStrategy());

  /* E21 is idiosyncratic (mapping two srams to dtim and itim) */
  strategies.push_back(new DefaultE21ArtyStrategy());
  strategies.push_back(new DefaultE21Strategy());

  /* Rocket is pretty straightforward */
  strategies.push_back(new DefaultRocketArtyStrategy());
  strategies.push_back(new DefaultRocketStrategy());

  /* Bullet only uses memory node */
  strategies.push_back(new DefaultBulletArtyStrategy());
  strategies.push_back(new DefaultBulletStrategy());

  /* E20 strategy goes last because it only uses one sram */
  strategies.push_back(new DefaultE20ArtyStrategy());
  strategies.push_back(new DefaultE20Strategy());

  ofstream os;
  os.open(linker_file);

  /*
   * Generate Linker Script
   *
   * Use the first valid strategy
   */
  bool strategy_found = false;
  for (auto it = strategies.begin(); it != strategies.end(); it++) {
    if ((*it)->valid(dtb ,memories)) {
      strategy_found = true;

      if (ramrodata) {
        LinkerScript lds = (*it)->create_layout(dtb, memories, LINK_STRATEGY_RAMRODATA);
        os << lds.describe();
      } else if (scratchpad) {
        LinkerScript lds = (*it)->create_layout(dtb, memories, LINK_STRATEGY_SCRATCHPAD);
        os << lds.describe();
      } else {
        LinkerScript lds = (*it)->create_layout(dtb, memories, LINK_STRATEGY_DEFAULT);
        os << lds.describe();
      }

      break;
    }
  }

  os.close();

  if (!strategy_found) {
    cerr << "No valid strategies found!";
    return 1;
  }

  return 0;
}