
/* Same as Handle */

#include <Uefi/UefiBaseType.h>
#include <stdio.h>
#include <string.h>

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/data.h"
#include "mruby/value.h"

#include "uefi.h"


struct MRB_UEFI_POINTER_DATA
{
    VOID *pointer;
};

static struct MRB_UEFI_POINTER_DATA*
mrb_uefi_pointer_alloc(mrb_state *mrb, VOID *pointer)
{
    struct MRB_UEFI_POINTER_DATA *pd;
    pd = (struct MRB_UEFI_POINTER_DATA *)mrb_malloc(mrb, sizeof(*pd));
    pd->pointer = pointer;
    return pd;
}

static void
mrb_uefi_pointer_free(mrb_state *mrb, void *ptr)
{
    mrb_free(mrb, ptr);
}

static struct mrb_data_type mrb_uefi_pointer_type = {
    "UEFI::Pointer", mrb_uefi_pointer_free
};

static mrb_value
mrb_uefi_pointer_wrap(mrb_state *mrb, struct RClass *p_cls, struct MRB_UEFI_POINTER_DATA *pd)
{
    return mrb_obj_value(Data_Wrap_Struct(mrb, p_cls, &mrb_uefi_pointer_type, pd));
}

static mrb_value
mrb_uefi_pointer_make_helper(mrb_state *mrb, struct RClass *p_cls, VOID *pointer)
{
    struct MRB_UEFI_POINTER_DATA *pd = mrb_uefi_pointer_alloc(mrb, pointer);
    return mrb_uefi_pointer_wrap(mrb, p_cls, pd);
}

mrb_value
mrb_uefi_pointer_make(mrb_state *mrb, VOID *pointer)
{
    mrb_value p_cls = uefi_const_get_under_uefi(mrb, "Pointer");
    return mrb_uefi_pointer_make_helper(mrb, mrb_class_ptr(p_cls), pointer);
}

static mrb_value
mrb_uefi_pointer_cmp(mrb_state *mrb, mrb_value self)
{
    struct MRB_UEFI_POINTER_DATA *pd1, *pd2;
    mrb_value other;
    mrb_get_args(mrb, "o", &other);
    pd1 = (struct MRB_UEFI_POINTER_DATA *)mrb_get_datatype(mrb, self, &mrb_uefi_pointer_type);
    pd2 = (struct MRB_UEFI_POINTER_DATA *)mrb_get_datatype(mrb, other, &mrb_uefi_pointer_type);
    if (!pd1 || !pd2){
        return mrb_nil_value();
    }

    if (pd1->pointer < pd2->pointer){
        return mrb_fixnum_value(-1);
    }else if (pd1->pointer == pd2->pointer){
        return mrb_fixnum_value(0);
    }else{
        return mrb_fixnum_value(1);
    }
}

static mrb_value
mrb_uefi_pointer_to_s(mrb_state *mrb, mrb_value self)
{
    struct MRB_UEFI_POINTER_DATA *pd;
    char buf[32];

    pd = (struct MRB_UEFI_POINTER_DATA *)mrb_get_datatype(mrb, self, &mrb_uefi_pointer_type);
#if defined(MDE_CPU_X64)
    snprintf(buf, sizeof(buf), "0x%016llX", (UINTN)(pd->pointer));
#elif defined(MDE_CPU_IA32)
    snprintf(buf, sizeof(buf), "0x%08lX", (UINTN)(pd->pointer));
#else
#error Not Supported
#endif
    return mrb_str_new_cstr(mrb, buf);
}

static mrb_value
mrb_uefi_pointer_inspect(mrb_state *mrb, mrb_value self)
{
    struct MRB_UEFI_POINTER_DATA *pd;
    char buf[32];

    pd = (struct MRB_UEFI_POINTER_DATA *)mrb_get_datatype(mrb, self, &mrb_uefi_pointer_type);
#if defined(MDE_CPU_X64)
    snprintf(buf, sizeof(buf), "<Pointer 0x%016llX>", (UINTN)(pd->pointer));
#elif defined(MDE_CPU_IA32)
    snprintf(buf, sizeof(buf), "<Pointer 0x%08lX>", (UINTN)(pd->pointer));
#else
#error Not Supported
#endif
    return mrb_str_new_cstr(mrb, buf);
}

static mrb_value
mrb_uefi_pointer_value(mrb_state *mrb, mrb_value self)
{
    struct MRB_UEFI_POINTER_DATA *pd;
    mrb_value value = mrb_str_new(mrb, NULL, sizeof(VOID *));

    pd = (struct MRB_UEFI_POINTER_DATA *)mrb_get_datatype(mrb, self, &mrb_uefi_pointer_type);
    memcpy(RSTRING_PTR(value), &pd->pointer, sizeof(pd->pointer));
    return value;
}

void
mrb_init_uefi_pointer(mrb_state *mrb, struct RClass *mrb_uefi)
{
    struct RClass *p_cls;

    p_cls = mrb_define_class_under(mrb, mrb_uefi, "Pointer", mrb->object_class);
    MRB_SET_INSTANCE_TT(p_cls, MRB_TT_DATA);

    mrb_include_module(mrb, p_cls, mrb_class_get(mrb, "Comparable"));

    mrb_define_method(mrb, p_cls, "<=>", mrb_uefi_pointer_cmp, ARGS_REQ(1));
    mrb_define_method(mrb, p_cls, "to_s", mrb_uefi_pointer_to_s, ARGS_NONE());
    mrb_define_method(mrb, p_cls, "inspect", mrb_uefi_pointer_inspect, ARGS_NONE());
    mrb_define_method(mrb, p_cls, "value", mrb_uefi_pointer_value, ARGS_NONE());

    mrb_const_set(mrb, mrb_obj_value(p_cls), mrb_intern(mrb, "NULL"),
                  mrb_uefi_pointer_make_helper(mrb, p_cls, NULL));
}
