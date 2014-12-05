
#include "mruby.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/string.h"
#include "error.h"
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "uefi.h"

#define E_IO_ERROR E_RUNTIME_ERROR

struct mrb_dir {
  DIR *dir;
};

void
mrb_dir_free(mrb_state *mrb, void *ptr)
{
  struct mrb_dir *mdir = ptr;

  if (mdir->dir) {
    closedir(mdir->dir);
    mdir->dir = NULL;
  }
  mrb_free(mrb, mdir);
}

static struct mrb_data_type mrb_dir_type = { "DIR", mrb_dir_free };

mrb_value
mrb_dir_close(mrb_state *mrb, mrb_value self)
{
  struct mrb_dir *mdir;
  mdir = (struct mrb_dir *)mrb_get_datatype(mrb, self, &mrb_dir_type);
  if (!mdir) return mrb_nil_value();
  if (!mdir->dir) {
    mrb_raise(mrb, E_IO_ERROR, "closed directory");
  }
  if (closedir(mdir->dir) == -1) {
    mrb_sys_fail(mrb, "closedir");
  }
  mdir->dir = NULL;
  return mrb_nil_value();
}

mrb_value
mrb_dir_init(mrb_state *mrb, mrb_value self)
{
  DIR *dir;
  struct mrb_dir *mdir;
  mrb_value path;
  char *cpath;

  mdir = (struct mrb_dir *)mrb_get_datatype(mrb, self, &mrb_dir_type);
  if (mdir) {
    mrb_dir_free(mrb, mdir);
  }

  mrb_get_args(mrb, "S", &path);
  cpath = mrb_string_value_cstr(mrb, &path);
  if ((dir = opendir(cpath)) == NULL) {
    mrb_sys_fail(mrb, cpath);
  }

  mdir = (struct mrb_dir *)mrb_malloc(mrb, sizeof(*mdir));
  mdir->dir = dir;
  DATA_PTR(self) = mdir;
  DATA_TYPE(self) = &mrb_dir_type;

  return self;
}

mrb_value
mrb_dir_read(mrb_state *mrb, mrb_value self)
{
  struct mrb_dir *mdir;
  struct dirent *dp;

  mdir = (struct mrb_dir *)mrb_get_datatype(mrb, self, &mrb_dir_type);
  if (!mdir) return mrb_nil_value();
  if (!mdir->dir) {
    mrb_raise(mrb, E_IO_ERROR, "closed directory");
  }
  dp = readdir(mdir->dir);
  if (dp != NULL) {
    return uefi_utf16_to_ascii(mrb, dp->d_name);
  } else {
    return mrb_nil_value();
  }
}

void
mrb_init_dir(mrb_state *mrb)
{
  struct RClass *d;

  d = mrb_define_class(mrb, "Dir", mrb->object_class);
  MRB_SET_INSTANCE_TT(d, MRB_TT_DATA);
  mrb_define_method(mrb, d, "close",      mrb_dir_close,  ARGS_NONE());
  mrb_define_method(mrb, d, "initialize", mrb_dir_init,   ARGS_REQ(1));
  mrb_define_method(mrb, d, "read",       mrb_dir_read,   ARGS_NONE());
}

