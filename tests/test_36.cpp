#include <isl/set.h>
#include <isl/union_map.h>
#include <isl/union_set.h>
#include <isl/ast_build.h>
#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include <tiramisu/debug.h>
#include <tiramisu/core.h>

#include <string.h>
#include <Halide.h>

#include "wrapper_test_36.h"

using namespace tiramisu;

/**
 * Test .compute_at().
 */

void generate_function(std::string name, int size, int val0)
{
    tiramisu::global::set_default_tiramisu_options();
    

    // -------------------------------------------------------
    // Layer I
    // -------------------------------------------------------

    tiramisu::function function0(name);
    tiramisu::constant N("N", tiramisu::expr((int32_t) size), p_int32, true, NULL, 0, &function0);
    tiramisu::var i("i");
    tiramisu::var j("j");
    tiramisu::computation S0("[N]->{S0[i,j]: 0<=i<N and 0<=j<N}", tiramisu::expr((uint8_t) val0), true, p_uint8, &function0);
    tiramisu::computation S1("[N]->{S1[i,j]: 0<=i<N and 1<=j<N-1}", (S0(i,j-1) + S0(i,j) + S0(i,j+1))/((uint8_t) 3), true, p_uint8, &function0);
    tiramisu::computation S2("[N]->{S2[i,j]: 0<=i<N and j=0}", (S0(i,j) + S0(i,j+1))/((uint8_t) 2), true, p_uint8, &function0);
    tiramisu::computation S3("[N]->{S3[i,j]: 0<=i<N and j=N-1}", (S0(i,j-1) + S0(i,j))/((uint8_t) 2), true, p_uint8, &function0);

    // -------------------------------------------------------
    // Layer II
    // -------------------------------------------------------


    S0.compute_at(S1, i);

    S2.after(S1, i);

    S3.after(S2, i);

    // -------------------------------------------------------
    // Layer III
    // -------------------------------------------------------

    tiramisu::buffer buf0("buf0", {size, size}, tiramisu::p_uint8, a_temporary, &function0);
    tiramisu::buffer buf1("buf1", {size, size}, tiramisu::p_uint8, a_output, &function0);
    S0.set_access("[N,M]->{S0[i,j]->buf0[i,j]: 0<=i<N and 0<=j<N}");
    S1.set_access("[N,M]->{S1[i,j]->buf1[i,j]: 0<=i<N and 0<=j<N}");
    S2.set_access("[N,M]->{S2[i,j]->buf1[i,j]: 0<=i<N and 0<=j<N}");
    S3.set_access("[N,M]->{S3[i,j]->buf1[i,j]: 0<=i<N and 0<=j<N}");

    // -------------------------------------------------------
    // Code Generation
    // -------------------------------------------------------

    function0.set_arguments({&buf1});
    function0.gen_time_space_domain();
    function0.gen_isl_ast();
    function0.gen_halide_stmt();
    function0.gen_halide_obj("build/generated_fct_test_" + std::string(TEST_NUMBER_STR) + ".o");
}

int main(int argc, char **argv)
{
    generate_function("tiramisu_generated_code", SIZE1, 2);

    return 0;
}
