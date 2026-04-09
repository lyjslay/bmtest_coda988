#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"

#define BM_SHELL_MAX_CMDS	128

int do_help(int argc, char **argv);

static struct cmd_entry cmd_list[BM_SHELL_MAX_CMDS] = {
  {"help", do_help, 0, ""},
  {NULL, NULL, 0, NULL}
};

int do_help(int argc, char **argv)
{
  struct cmd_entry *ptr = cmd_list;

  printf("Command List:\n");
  for (; ptr->name != NULL; ptr++) {
    printf("  %s %s\n", ptr->name, ptr->usage);
  }
  return 0;
}

static struct cmd_entry * find_cmd(char *cmd)
{
  struct cmd_entry *ptr = cmd_list;
  for (; ptr->name != NULL; ptr++) {
    if (strcmp(cmd, ptr->name) == 0)
      return ptr;
  }
  return NULL;
}

int command_main(int argc, char **argv)
{
  struct cmd_entry *ptr = find_cmd(argv[0]);
  if (ptr == NULL) {
     printf("unknown command %s\n", argv[0]);
     return -1;
  }
  if (argc != (ptr->arg_count + 1)) {
    printf("%s with wrong argument, usage:\n", ptr->name);
    printf("  %s %s\n", ptr->name, ptr->usage);
    return -1;
  }
  return ptr->func(argc, argv);
}

int command_add(struct cmd_entry *cmd)
{
    struct cmd_entry *cptr;
    int i;

    if( ( !cmd ) || ( !cmd->name ) || ( !cmd->func ) )
        return -1;

    for (cptr = cmd_list, i = 0; i < BM_SHELL_MAX_CMDS; cptr++, i++)
    {
      if (!cptr->name)
      {
          memcpy((char*)cptr, (char*)cmd, sizeof(struct cmd_entry));
          return 0;
      }
    }

    return -1;
}

