/* Copyright (c) 2019 SiFive Inc. */
/* SPDX-License-Identifier: Apache-2.0 */

#include "section.h"

Section::Section(Memory logical_memory, Memory virtual_memory, Phdr program_header)
  : logical_memory(logical_memory),
    virtual_memory(virtual_memory),
    program_header(program_header)
{
}

string Section::describe() {
  string description = "\t." + output_name + " : {\n";

  for (auto it = commands.begin(); it != commands.end(); it++) {
    description += "\t\t" + *it + "\n";
  }

  description += "\t} >" + virtual_memory.name + " ";
  description += "AT>" + logical_memory.name + " ";
  description += ":" + program_header.name + "\n";

  for (auto it = trailing_commands.begin(); it != trailing_commands.end(); it++) {
    description += "\t" + *it + "\n";
  }

  return description;
}

void Section::add_command(string command) {
  commands.push_back(command);
}
