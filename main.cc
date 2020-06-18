/*************************************************************************
 *
 *  This file is part of the ACT library
 *
 *  Copyright (c) 2018-2019 Rajit Manohar
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *
 **************************************************************************
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <act/act.h>
#include <act/passes/netlist.h>
#include "config.h"
#include "check_chp.h"
#include "cartographer.h"
#include <act/chp-opt/optimize.h>

static void usage(char *name)
{
  fprintf(stderr, "Usage: %s <actfile> <process> <outfile> [--optimize]\n", name);
  exit(1);
}

int main(int argc, char **argv)
{
  Act *a;
  char *proc;
  bool chpopt = false;

  /* initialize ACT library */
  Act::Init(&argc, &argv);

  /* some usage check */
  if (argc < 4 || argc > 5)
  {
    usage(argv[0]);
  }
  
  /* check if optimize */
  if (argc == 5 && strcmp(argv[4], "--optimize") == 0) {
    chpopt = true;
    printf("> Optimization turned ON\n");
  }

  /* read in the ACT file */
  a = new Act(argv[1]);

  /* expand it */
  a->Expand();

  /* read configuration file, if any */
  config_read("prs2net.conf");

  /* find the process specified on the command line */
  Process *p = a->findProcess(argv[2]);

  if (!p)
  {
    fatal_error("Could not find process `%s' in file `%s'", argv[2], argv[1]);
  }

  if (!p->isExpanded())
  {
    fatal_error("Process `%s' is not expanded.", argv[2]);
  }

  /* extract the chp */
  if (p->lang == NULL || p->lang->getchp() == NULL)
  {
    fatal_error("Input file `%s' does not have any chp.", argv[2]);
  }

  if (chpopt) {
    ChpOpt::optimize(p, a->getTypeFactory());
    printf("> Optimized CHP:\n");
    chp_print(stdout, p->lang->getchp()->c);
    printf("\n");
  }
  
  struct Hashtable * chan_sends = check_chp(p);
  generate_act(p, argv[1], argv[3], false, 0, chpopt, chan_sends);
  hash_free(chan_sends);
  return 0;
}
