//
// refer to mir-test/api-loop.h loop-interp.c
//
// g++ -I.. -o test.exe mir-01-hello.cc -L.. -lmir
//

extern "C" {
#include "mir.h"
}

// 1 2 3 5 8 13 21
int fun_ab(int n)
{
    int a = 1, b = 2;
    for (int i = 2; i < n; i++) {
	int t = b;
	b = a + b;
	a = t;
    }
    return b;
}

int main()
{
    MIR_context_t ctx = MIR_init();
    MIR_module_t m = MIR_new_module (ctx, "m");

    MIR_item_t func;
    MIR_label_t fin, cont;
    MIR_reg_t ARG1, R2;
    MIR_type_t res_type = MIR_T_I64;

    MIR_load_external (ctx, "fun_ab", (void*)fun_ab);

    // void fun(void)
    func = MIR_new_func (ctx, "fun", 1, &res_type, 0);
    R2 = MIR_new_func_reg (ctx, func->u.func, MIR_T_I64, "count");

    // MOV R2, 10
    MIR_append_insn (ctx, func, MIR_new_insn (ctx, MIR_MOV,
					      MIR_new_reg_op (ctx, R2),
					      MIR_new_int_op (ctx, 5)));

    // call funab
    MIR_item_t proto = MIR_new_proto(ctx, "p_funab", 1, &res_type, 1, MIR_T_I64, "arg1");
    MIR_item_t f_fun_ab = MIR_new_import (ctx, "fun_ab");

    MIR_append_insn (ctx, func, MIR_new_call_insn (ctx, 4,
						   MIR_new_ref_op (ctx, proto),
						   MIR_new_ref_op (ctx, f_fun_ab),
						   MIR_new_reg_op (ctx, R2),
						   MIR_new_reg_op (ctx, R2)));

    // ret R2
    MIR_append_insn (ctx, func, MIR_new_ret_insn (ctx, 1, MIR_new_reg_op (ctx, R2)));
    MIR_finish_func (ctx);
    MIR_finish_module (ctx);

    MIR_load_module (ctx, m);
    MIR_link (ctx, MIR_set_interp_interface, NULL);

    MIR_val_t val;
    MIR_interp (ctx, func, &val, 0);
    printf("interp result: %ld\n", val.i);

    MIR_finish(ctx);
}
