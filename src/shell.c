#include "mruby.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/string.h"
#include "error.h"
#include  <stdlib.h>
#include "uefi.h"

mrb_value
mrb_shell_exec(mrb_state *mrb, mrb_value klass)
{
  mrb_value command;
  char *ccommand;
  int ret;

  mrb_get_args(mrb, "S", &command);
  ccommand = mrb_string_value_cstr(mrb, &command);
  ret = system(ccommand);
  return mrb_fixnum_value(ret);
}

void
mrb_init_shell(mrb_state *mrb)
{
  struct RClass *shell;

  shell = mrb_define_class(mrb, "Shell", mrb->object_class);
  MRB_SET_INSTANCE_TT(shell, MRB_TT_DATA);
  mrb_define_class_method(mrb, shell, "exec", mrb_shell_exec, ARGS_REQ(1));
}

