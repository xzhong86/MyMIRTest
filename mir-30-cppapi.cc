//
// refer to mir-test/api-loop.h loop-interp.c
//
// g++ -std=c++11 -I.. -o test.exe mir-02-speed.cc -L.. -lmir
//

extern "C" {
#include "mir.h"
#include "mir-gen.h"
#include "../real-time.h"
}

#include <functional>

namespace mir {

using String = std::string;
using Vector = std::vector;



class Context {
    MIR_context ctx_;
    std::map<String, MIR_module_t> modules_;
    using Proc = std::function<void(void)>;
public:
    Context() : ctx_(MIR_init()) { }

    void module_begin(String name) {
        MIR_module_t m = MIR_new_module(ctx_, name.c_str());
        modules_[name] = m;
    }
    void module_end() {
        MIR_finish_module(ctx_);
    }
    void module(String name, Proc proc) {
        module_begin(name);
        proc();
        module_end();
    }

    void func_begin(String name, Vector<MIR_type_t> res_type, Vector<MIR_var_t> args) {
        MIR_item_t func;
        func = MIR_new_func(ctx_, name.c_str(),
                            res_type.size(), res_type.data(),
                            args.size(), args.data());
        funcs_[name] = func;
        curr_func_ = func;
    }
    void func_end() {
        MIR_finish_func(ctx_);
    }
    void func(String name, Vector<MIR_type_t> res_type, Vector<MIR_var_t> args,
              Proc proc) {
        func_begin(name);
        proc();
        func_end();
    }

    void append(MIR_insn_t insn) {
        MIR_append_insn(ctx_, curr_func_, insn);
    }

    using Reg = MIR_reg_t;
    using InsnCode = MIR_insn_code_t;

    template <typename IMM>
    void _mov(MIR_insn_code_t op, Reg reg, IMM imm) {
        append(MIR_new_insn(ctx_, op,
                            MIR_new_reg_op(ctx_, reg),
                            MIR_new_int_op(ctx, imm)));
    }

    // c.mov(R2, 1)
    void mov(Reg reg, int64_t imm)  { _mov(MIR_MOV, reg, imm); }
    void mov(Reg reg, double imm)   { _mov(MIR_MOVD, reg, imm); }

    void proto() {
        MIR_new_proto_arr;
    }
    void call() {
        append(MIR_new_call_insn(
                   ctx_, nops, ...));
    }
};
}

static void time_it(std::string msg, std::function<void(void)> proc)
{
    double start_time = real_sec_time ();
    proc();
    double elapse = real_sec_time() - start_time;
    printf("%s => %.3f sec.\n", msg.c_str(), elapse);
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
    printf("debug: %s:%d\n", __FILE__, __LINE__);
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

    printf("debug: %s:%d\n", __FILE__, __LINE__);
    // ret R2
    MIR_append_insn (ctx, func, MIR_new_ret_insn (ctx, 1, MIR_new_reg_op (ctx, R2)));
    MIR_finish_func (ctx);
    MIR_finish_module (ctx);

    printf("==== Before load_module and link\n");
    MIR_output (ctx, stdout);

    MIR_load_module (ctx, m);
#if 1
    MIR_link (ctx, MIR_set_interp_interface, NULL);

    printf("==== After load_module and link\n");
    MIR_output (ctx, stdout);

    MIR_val_t val;
    val.i = 100;
    MIR_interp (ctx, func, &val, 1, val);
    printf("interp result: %ld\n", val.i);

    time_it("loop 1000000000 with interp", [&](){
        val.i =   1000000000;
        MIR_interp (ctx, func, &val, 1, val);
    });
#else

    unsigned level = 3;
    MIR_gen_init (ctx, 1);
    MIR_gen_set_optimize_level (ctx, 0, level);
    //MIR_gen_set_debug_file (ctx, 0, stderr);
    MIR_link (ctx, MIR_set_gen_interface, NULL);
    uint64_t (*fun) (uint64_t n_iter);

    printf("debug: %s:%d\n", __FILE__, __LINE__);
    fun = (uint64_t (*)(uint64_t)) MIR_gen (ctx, 0, func);

    printf("debug: %s:%d\n", __FILE__, __LINE__);
    time_it("loop 1000000000 with gen", [&](){  // 7 times faster?
        uint64_t i = 1000000000;
        fun(i);
    });
    MIR_gen_finish (ctx);
#endif

    MIR_finish(ctx);
}
