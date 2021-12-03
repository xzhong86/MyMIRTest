

extern "C" {
#include "c2mir/c2mir.h"
#include "mir-gen.h"
}

#include <cstring> // memset

#include <string>
using std::string;

struct StrBuf {
    string str;
    int len;
    int pos;
    StrBuf(const string &str) : str(str), len(str.length()), pos(0) { }
};

int my_getc(void *data) {
    StrBuf *buf = (StrBuf*)data;
    return (buf->pos >= buf->len) ? EOF : buf->str[buf->pos++];
}

int main()
{
    const char *c_code = "int ar_add(int arr[], int rd, int rs1, int imm) { arr[rd] = arr[rs1] + imm; }";
    StrBuf sbuf(c_code);

    MIR_context_t ctx = MIR_init();
    c2mir_init(ctx);

    struct c2mir_options opt;
    memset(&opt, 0, sizeof(opt));

    int res = c2mir_compile(ctx, &opt, my_getc, &sbuf, "test-code", NULL);

    c2mir_finish(ctx);

    MIR_output (ctx, stdout);

    MIR_module_t m = DLIST_HEAD (MIR_module_t, *MIR_get_module_list (ctx));
    MIR_item_t func = DLIST_HEAD (MIR_item_t, m->items);

    printf("m=%p name=%s\n", m, m->name);
    //MIR_output_item(ctx, stdout, func);

#if 0
    // TBD: failed when using mir gen interface
    unsigned level = 0;
    MIR_gen_init (ctx, 1);
    MIR_gen_set_optimize_level (ctx, 0, level);
    //MIR_gen_set_debug_file (ctx, 0, stderr);
    MIR_link (ctx, MIR_set_gen_interface, NULL);

    typedef int (*fun_p) (int arr[], int rd, int rs, int imm);

    printf("debug: %s:%d\n", __FILE__, __LINE__);
    fun_p fun = (fun_p) MIR_gen (ctx, 0, func);
    printf("debug: %s:%d\n", __FILE__, __LINE__);
    printf("MIR_gen fun=%p\n", fun);


    int arr[] = { 1,2,3,4,5 };
    fun(arr, 0, 2, 9);

    printf("result arr[0]=%d\n", arr[0]);

    MIR_gen_finish (ctx);
#else

    MIR_link (ctx, MIR_set_interp_interface, NULL);

    int arr[] = { 1,2,3,4,5 };

    MIR_val_t v_res, v_arr, v_rd, v_rs, v_imm;
    v_arr.a = arr;
    v_rd.i  = 0;
    v_rs.i  = 2;
    v_imm.i = 9;
    MIR_interp (ctx, func, &v_res, 4, v_arr, v_rd, v_rs, v_imm);
    printf("interp result: arr[0]=%d\n", arr[0]);

#endif

    MIR_finish(ctx);
}
