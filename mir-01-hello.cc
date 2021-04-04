//
// refer to mir-test/api-loop.h loop-interp.c
//
// g++ -I.. -o test.exe mir-01-hello.cc -L.. -lmir
//

extern "C" {
#include "mir.h"
}

int main()
{
    MIR_context_t ctx = MIR_init();
    MIR_module_t m = MIR_new_module (ctx, "m");

    MIR_item_t func;
    MIR_label_t fin, cont;
    MIR_reg_t ARG1, R2;
    MIR_type_t res_type = MIR_T_I64;

    func = MIR_new_func (ctx, "loop", 1, &res_type, 1, MIR_T_I64, "arg1");
    R2 = MIR_new_func_reg (ctx, func->u.func, MIR_T_I64, "count");
    ARG1 = MIR_reg (ctx, "arg1", func->u.func);
    fin = MIR_new_label (ctx);
    cont = MIR_new_label (ctx);

    // MOV R2, 0
    MIR_append_insn (ctx, func,
                     MIR_new_insn (ctx, MIR_MOV, MIR_new_reg_op (ctx, R2), MIR_new_int_op (ctx, 0)));

    // BGE fin, R2, ARG1
    MIR_append_insn (ctx, func,
                     MIR_new_insn (ctx, MIR_BGE, MIR_new_label_op (ctx, fin),
                                   MIR_new_reg_op (ctx, R2), MIR_new_reg_op (ctx, ARG1)));

    // cont:   // add lable
    MIR_append_insn (ctx, func, cont);

    // ADD R2, R2, 1
    MIR_append_insn (ctx, func,
                     MIR_new_insn (ctx, MIR_ADD, MIR_new_reg_op (ctx, R2), MIR_new_reg_op (ctx, R2),
                                   MIR_new_int_op (ctx, 1)));

    // BLT cont, R2, ARG1
    MIR_append_insn (ctx, func,
                     MIR_new_insn (ctx, MIR_BLT, MIR_new_label_op (ctx, cont),
                                   MIR_new_reg_op (ctx, R2), MIR_new_reg_op (ctx, ARG1)));

    // fin:   // add lable
    MIR_append_insn (ctx, func, fin);

    // ret R2
    MIR_append_insn (ctx, func, MIR_new_ret_insn (ctx, 1, MIR_new_reg_op (ctx, R2)));
    MIR_finish_func (ctx);
    MIR_finish_module (ctx);

    printf("==== Before load_module and link\n");
    MIR_output (ctx, stdout);

    MIR_load_module (ctx, m);
    MIR_link (ctx, MIR_set_interp_interface, NULL);

    printf("==== After load_module and link\n");
    MIR_output (ctx, stdout);


    MIR_val_t val;
    val.i = 100;
    MIR_interp (ctx, func, &val, 1, val);
    printf("interp result: %ld\n", val.i);

    MIR_finish(ctx);
}
