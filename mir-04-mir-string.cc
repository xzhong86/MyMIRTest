//
// refer to mir-test/api-loop.h loop-interp.c
//
// g++ -I.. -o test.exe mir-01-hello.cc -L.. -lmir
//

#include <string>

extern "C" {
#include "mir.h"
}

using std::string;
string get_mir_string() {
    // refer to mir-tests/test1.mir
//m1:    module
//cse:   func i64, i64:i
//       local i64:l, i64:j, i64:k
//       mov j, 0
//       mov k, 0
//       mul l, 2, i
//       bgt L1, i, 0
//       mul j, 2, i
//       jmp L2
//L1:
//       mul k, 2, i
//L2:
//       add i, l, j
//       add i, i, k
//       ret i
//       endfunc
//       endmodule
    string str("");
    str += "m1: module\n";
    str += "loop: func i64, i64:arg\n";
    str += "\t local i64:sum, i64:i\n";
    str += "\t mov sum, 0\n";
    str += "\t mov i, 0\n";
    str += "L1:\n";
    str += "\t add sum, sum, i\n";
    str += "\t add i, i, 1\n";
    str += "\t blt L1, i, arg\n";
    str += "\t ret sum\n";
    str += "\t endfunc\n";
    str += "\t endmodule\n";
    return str;
}

int main()
{
    MIR_context_t ctx = MIR_init();

    string str = get_mir_string();
    MIR_scan_string(ctx, str.c_str());

    // refer to mir-tests/readme-example.c
    MIR_module_t m = DLIST_HEAD (MIR_module_t, *MIR_get_module_list (ctx));
    MIR_item_t func = DLIST_TAIL (MIR_item_t, m->items);
    //MIR_item_t func = MIR_get_global_item(ctx, "loop"); // unusable

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
