/* ************************************************************************
 * Copyright 2016 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#include <gtest/gtest.h>
#include <math.h>
#include <stdexcept>
#include <vector>
#include "testing_gemm_strided_batched_ex.hpp"
#include "utility.h"

using ::testing::TestWithParam;
using ::testing::Values;
using ::testing::ValuesIn;
using ::testing::Combine;
using namespace std;
// {M, N, K, lda, ldb, ldc, ldd, stride_a, stride_b, stride_c, stride_d};
// {alpha,beta},{transA,transB},batch_count,{type_a,type_b,type_c,type_d}

typedef std::tuple<vector<int>, vector<double>, vector<char>, int, vector<rocblas_datatype>>
    gemm_strided_batched_ex_tuple;

/* =====================================================================
README: This file contains testers to verify the correctness of
        BLAS routines with google test

        It is supposed to be played/used by advance / expert users
        Normal users only need to get the library routines without testers
     =================================================================== */

/* =====================================================================
Advance users only: BrainStorm the parameters but do not make artificial one which invalidates the
matrix.
like lda pairs with M, and "lda must >= M". case "lda < M" will be guarded by argument-checkers
inside API of course.
Yet, the goal of this file is to verify result correctness not argument-checkers.

Representative sampling is sufficient, endless brute-force sampling is not necessary
=================================================================== */

// vector of vector, each vector is a {M, N, K, lda, ldb, ldc, ldd, stride_a, stride_b, stride_c,
// stride_d};
// clang-format off

const vector<vector<int>> known_bug_small_matrix_size_range= {
    {  8,   9,  10,     8,  10,   8,    8,       80,    90,     82,    82 },   // NT gives error
    {  4,   3,   4,     4,   4,   4,    4,       16,    12,     12,    12 },   // NT, TC gives error
    {  3,   3,   3,   3,   3,   3,   3,     0,      9,      9,      9}, // CI error after re-trained gfx900/gfx906
    {  3,   3,   3,   3,   3,   3,   3,     9,      0,      9,      9}, // CI error after re-trained gfx900/gfx906
};

const vector<vector<int>> small_matrix_size_range = {
    { -1,  -1,  -1,    -1,   1,   1,    1,        1,      1,     1,     1 },
    {  4,   3,   4,     4,   4,   4,    4,       16,    16,     16,    16 },
    {  4,   4,   4,     4,   4,   5,    5,       16,    16,     20,    20 },
    {  4,   4,   4,     4,   4,   5,    5,       17,    17,     20,    20 },
    {  4,   4,   4,     4,   4,   5,    5,       17,    16,     20,    20 },
    {  4,   4,   4,     4,   4,   5,    5,       16,    17,     20,    20 },
    {  8,   8,   8,     8,   8,   8,    8,       64,    64,     64,    64 },
    {  8,   9,  10,     8,  10,   8,    8,      100,   100,    100,   100 },
    { 15,  15,  15,    15,  15,  15,   15,      225,   225,    225,   225 },
    { 16,  16,  16,    16,  16,  16,   16,      256,   256,    256,   256 },
    { 17,  17,  17,    17,  17,  17,   17,      289,   289,    289,   289 },
    { 31,  33,  35,   101, 102, 103,  103,     3605,   3605,  3605,  3605 },
    { 59,  61,  63,   129, 131, 137,  137,     8631,   8631,  8631,  8631 },
    { 63,  63,  63,    63,  63,  63,   63,     3969,  3969,   3969,  3969 },
    { 64,  64,  64,    64,  64,  64,   64,     4096,  4096,   4096,  4096 },
    { 65,  65,  65,    65,  65,  65,   65,     4225,  4225,   4225,  4225 },
    {127, 127, 127,   127, 127, 127,  127,    16129, 16129,  16129, 16129 },
    {128, 128, 128,   128, 128, 128,  128,    16384, 16384,  16384, 16384 },
    {129, 129, 129,   129, 129, 129,  129,    16641, 16641,  16641, 16641 },
};

const vector<vector<int>> small_matrix_size_stride_a_range = {
    {  3,   3,   3,   3,   3,   3,   3,     9,      9,      9,      9},
    { 15,  15,  15,  15,  15,  15,  15,   225,      0,    225,    225},
    { 16,  16,  16,  16,  16,  16,  16,     0,    256,    256,    256},
    { 17,  17,  17,  17,  17,  17,  17,   289,      0,    289,    289},
    { 63,  63,  63,  63,  63,  63,  63,     0,   3969,   3969,   3969},
    { 64,  64,  64,  64,  64,  64,  64,  4096,      0,   4096,   4096},
    { 65,  65,  65,  65,  65,  65,  65,     0,   4225,   4225,   4225},
    {127, 127, 127, 127, 127, 127, 127, 16129,      0,  16129,  16129},
    {128, 128, 128, 128, 128, 128, 128,     0,  16384,  16384,  16384},
    {129, 129, 129, 129, 129, 129, 129, 16641,      0,  16641,  16641},
};

const vector<vector<int>> medium_matrix_size_range = {
    {255, 255, 255, 255, 255, 255, 255,  65025,  65025,  65025,  65025},
    {256, 256, 256, 256, 256, 256, 256,  65536,  65536,  65536,  65536},
    {257, 257, 257, 257, 257, 257, 257,  66049,  66049,  66049,  66049},
};

const vector<vector<int>> medium_matrix_size_stride_a_range = {
    {255, 255, 255, 255, 255, 255, 255, 65025,     0, 65025, 65025},
    {256, 256, 256, 256, 256, 256, 256,     0, 65536, 65536, 65536},
    {257, 257, 257, 257, 257, 257, 257, 66049,     0, 66049, 66049},
};

const vector<vector<int>> large_matrix_size_range = {
    {511, 511, 511,  511, 511, 511, 511, 261121, 261121, 261121, 261121},
    {512, 512, 512,  512, 512, 512, 512, 262144, 262144, 262144, 262144},
    {513, 513, 513,  513, 513, 513, 513, 263169, 263169, 263169, 263169},
    {513, 514, 515,  516, 517, 518, 518, 266771, 266772, 266773, 266773},
};
const vector<vector<int>> large_matrix_size_stride_a_range = {
    {511, 511, 511,  511, 511, 511, 511,      0, 261121, 261121, 261121},
    {512, 512, 512,  512, 512, 512, 512, 262144,      0, 262144, 262144},
    {513, 513, 513,  513, 513, 513, 513,      0, 263169, 263169, 263169},
    {513, 514, 515,  516, 517, 518, 518, 266771,      0, 266773, 266773},
};

// vector of vector, each pair is a {alpha, beta};
// add/delete this list in pairs, like {2.0, 4.0}

const vector<vector<double>> full_alpha_beta_range = { {1.0, 0.0}, {-2.0, -3.0}, {0.0, 1.0}, };

const vector<vector<double>> alpha_beta_2_3 = {{2.0, 3.0}};
// clang-format on

// vector of vector, each pair is a {transA, transB};
// add/delete this list in pairs, like {'N', 'T'}
// for single/double precision, 'C'(conjTranspose) will downgraded to 'T' (transpose) internally in
// sgemm_strided_batched_ex/dgemm_strided_batched_ex,
const vector<vector<char>> full_transA_transB_range = {
    {'N', 'N'}, {'N', 'T'}, {'C', 'N'}, {'T', 'C'}};
const vector<vector<char>> transA_transB_NT = {{'N', 'T'}};

// number of gemms in batched gemm
// clang-format off
const vector<int> batch_count_n1_0_1_3    = { -1,   0,   1,  3 };
const vector<int> batch_count_31_32_33    = { 31,  32,  33,    };
const vector<int> batch_count_63_64_65    = { 63,  64,  65,    };
const vector<int> batch_count_2           = {  2               };

//const vector<int> small_batch_count_stride_a_range  = {  1,   2,   3,    };
//const vector<int> small_batch_count_stride_a_range  = {  1,   2,         };
//const vector<int> small_batch_count_stride_a_range  = {  1,   2,   3,    };
//const vector<int> medium_batch_count_stride_a_range = { 31,  32,  33,    };

// a_type, b_type, c_type, d_type, compute_type
const vector<vector<rocblas_datatype>> precision_half = {{ rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r  }};

const vector<vector<rocblas_datatype>> precision_hpa_half = {{ rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f32_r  }};

const vector<vector<rocblas_datatype>> precision_single = {{ rocblas_datatype_f32_r,
rocblas_datatype_f32_r,
rocblas_datatype_f32_r,
rocblas_datatype_f32_r,
rocblas_datatype_f32_r  }};

const vector<vector<rocblas_datatype>> precision_double = {{ rocblas_datatype_f64_r,
rocblas_datatype_f64_r,
rocblas_datatype_f64_r,
rocblas_datatype_f64_r,
rocblas_datatype_f64_r  }};

const vector<vector<rocblas_datatype>> precision_type_range = {{rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r},
{rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f16_r,
rocblas_datatype_f32_r},
{rocblas_datatype_f32_r,
rocblas_datatype_f32_r,
rocblas_datatype_f32_r,
rocblas_datatype_f32_r,
rocblas_datatype_f32_r},
{rocblas_datatype_f64_r,
rocblas_datatype_f64_r,
rocblas_datatype_f64_r,
rocblas_datatype_f64_r,
rocblas_datatype_f64_r}};

// clang-format on

// clang-format off
// vector of vector, each vector is a {M, N, K, lda, ldb, ldc, stride_a, stride_b, stride_c},{alpha,beta},{transA,transB},batch_count,precision;
//gemm_strided_batched_ex_tuple db_sb_1 {{12544,  64,  64,12544,  64,12544,802816, 0, 802816},{1, 0},{'N','N'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_2 {{12544,  64,  64,12544,  64,12544,802816, 0, 802816},{1, 0},{'N','N'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_3 {{ 3136, 256,  64, 3136,  64, 3136,200704, 0, 802816},{1, 0},{'N','N'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_4 {{ 3136, 256,  64, 3136,  64, 3136,200704, 0, 802816},{1, 0},{'N','N'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_5 {{ 3136,  64, 256, 3136, 256, 3136,802816, 0, 200704},{1, 0},{'N','N'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_6 {{ 3136,  64, 256, 3136, 256, 3136,802816, 0, 200704},{1, 0},{'N','N'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_7 {{  784, 128, 512,  784, 512,  784,401408, 0, 100352},{1, 0},{'N','N'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_8 {{  784, 128, 512,  784, 512,  784,401408, 0, 100352},{1, 0},{'N','N'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_9 {{  784, 512, 128,  784, 128,  784,100352, 0, 401408},{1, 0},{'N','N'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_10{{  784, 512, 128,  784, 128,  784,100352, 0, 401408},{1, 0},{'N','N'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_11{{  784,  64, 192,  784, 192,  784,150528, 0,  50176},{1, 0},{'N','N'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_12{{12544,  64,  64,12544,  64,12544,802816, 0, 802816},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_13{{12544,  64,  64,12544,  64,12544,802816, 0, 802816},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_14{{  196,1024, 256,  196,1024,  196, 50176, 0, 200704},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_15{{  196,1024, 256,  196,1024,  196, 50176, 0, 200704},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_16{{  196, 256,1024,  196, 256,  196,200704, 0,  50176},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_17{{  196, 256,1024,  196, 256,  196,200704, 0,  50176},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_18{{  196, 256, 256,  196, 256,  196, 50176, 0,  50176},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_19{{  196, 256, 256,  196, 256,  196, 50176, 0,  50176},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_20{{  196, 512, 192,  196, 512,  196, 37632, 0, 100352},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_21{{ 3136, 256,  64, 3136, 256, 3136,200704, 0, 802816},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_22{{ 3136, 256,  64, 3136, 256, 3136,200704, 0, 802816},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_23{{ 3136,  64, 256, 3136,  64, 3136,802816, 0, 200704},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_24{{ 3136,  64, 256, 3136,  64, 3136,802816, 0, 200704},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_25{{   49,2048, 512,   49,2048,   49, 25088, 0, 100352},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_26{{   49,2048, 512,   49,2048,   49, 25088, 0, 100352},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_27{{   49, 512,2048,   49, 512,   49,100352, 0,  25088},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_28{{   49, 512,2048,   49, 512,   49,100352, 0,  25088},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_29{{   49, 512, 512,   49, 512,   49, 25088, 0,  25088},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_30{{   49, 512, 512,   49, 512,   49, 25088, 0,  25088},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_31{{   49, 832, 256,   49, 832,   49, 12544, 0,  40768},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_32{{  784, 128, 512,  784, 128,  784,401408, 0, 100352},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_33{{  784, 128, 512,  784, 128,  784,401408, 0, 100352},{1, 0},{'N','T'}, 8,precision_half};
//gemm_strided_batched_ex_tuple db_sb_34{{  784, 192,  64,  784, 192,  784, 50176, 0, 150528},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_35{{  784, 512, 128,  784, 512,  784,100352, 0, 401408},{1, 0},{'N','T'},16,precision_half};
//gemm_strided_batched_ex_tuple db_sb_36{{  784, 512, 128,  784, 512,  784,100352, 0, 401408},{1, 0},{'N','T'}, 8,precision_half};
//
//const vector<gemm_strided_batched_ex_tuple> deepbench_sb_vec = {
//    db_sb_1,  db_sb_2,  db_sb_3,  db_sb_4,  db_sb_5,  db_sb_6,  db_sb_7,  db_sb_8,  db_sb_9,
//    db_sb_10, db_sb_11, db_sb_12, db_sb_13, db_sb_14, db_sb_15, db_sb_16, db_sb_17, db_sb_18,
//    db_sb_19, db_sb_20, db_sb_21, db_sb_22, db_sb_23, db_sb_24, db_sb_25, db_sb_26, db_sb_27,
//    db_sb_28, db_sb_29, db_sb_30, db_sb_31, db_sb_32, db_sb_33, db_sb_34, db_sb_35, db_sb_36};

// clang-format on

/* ===============Google Unit Test==================================================== */

/* =====================================================================
     BLAS-3 gemm_strided_batched_ex:
=================================================================== */

/* ============================Setup Arguments======================================= */

// Please use "class Arguments" (see utility.hpp) to pass parameters to templated testers;
// Some routines may not touch/use certain "members" of objects "argus".
// like BLAS-1 Scal does not have lda, BLAS-2 GEMV does not have ldb, ldc;
// That is fine. These testers & routines will leave untouched members alone.
// Do not use std::tuple to directly pass parameters to testers
// by std:tuple, you have unpack it with extreme care for each one by like "std::get<0>" which is
// not intuitive and error-prone

Arguments setup_gemm_strided_batched_ex_arguments(gemm_strided_batched_ex_tuple tup)
{
    vector<int> matrix_size                  = std::get<0>(tup);
    vector<double> alpha_beta                = std::get<1>(tup);
    vector<char> transA_transB               = std::get<2>(tup);
    int batch_count                          = std::get<3>(tup);
    vector<rocblas_datatype> precision_types = std::get<4>(tup);

    Arguments arg;

    // see the comments about matrix_size_range above
    arg.M        = matrix_size[0];
    arg.N        = matrix_size[1];
    arg.K        = matrix_size[2];
    arg.lda      = matrix_size[3];
    arg.ldb      = matrix_size[4];
    arg.ldc      = matrix_size[5];
    arg.ldd      = matrix_size[6];
    arg.stride_a = matrix_size[7];
    arg.stride_b = matrix_size[8];
    arg.stride_c = matrix_size[9];
    arg.stride_d = matrix_size[10];

    // the first element of alpha_beta_2_3 is always alpha, and the second is always beta
    arg.alpha = alpha_beta[0];
    arg.beta  = alpha_beta[1];

    arg.transA_option = transA_transB[0];
    arg.transB_option = transA_transB[1];

    arg.batch_count = batch_count;

    arg.a_type       = precision_types[0];
    arg.b_type       = precision_types[1];
    arg.c_type       = precision_types[2];
    arg.d_type       = precision_types[3];
    arg.compute_type = precision_types[4];

    arg.timing = 0;

    return arg;
}

class gemm_strided_batched_ex : public ::TestWithParam<gemm_strided_batched_ex_tuple>
{
    protected:
    gemm_strided_batched_ex() {}
    virtual ~gemm_strided_batched_ex() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_P(gemm_strided_batched_ex, standard)
{
    // GetParam return a tuple. Tee setup routine unpack the tuple
    // and initializes arg(Arguments) which will be passed to testing routine
    // The Arguments data struture have physical meaning associated.
    // while the tuple is non-intuitive.

    Arguments arg = setup_gemm_strided_batched_ex_arguments(GetParam());

    //  std::cout << "gemm_strided_batched_ex, standard" << std::endl;

    rocblas_status status = testing_gemm_strided_batched_ex(arg);

    // if not success, then the input argument is problematic, so detect the error message
    if(status != rocblas_status_success)
    {
        if(arg.M < 0 || arg.N < 0 || arg.K < 0)
        {
            EXPECT_EQ(rocblas_status_invalid_size, status);
        }
        else if(arg.transA_option == 'N' ? arg.lda < arg.M : arg.lda < arg.K)
        {
            EXPECT_EQ(rocblas_status_invalid_size, status);
        }
        else if(arg.transB_option == 'N' ? arg.ldb < arg.K : arg.ldb < arg.N)
        {
            EXPECT_EQ(rocblas_status_invalid_size, status);
        }
        else if(arg.ldc < arg.M)
        {
            EXPECT_EQ(rocblas_status_invalid_size, status);
        }
        else if(arg.batch_count < 0)
        {
            EXPECT_EQ(rocblas_status_invalid_size, status);
        }
    }
}

// notice we are using vector of vector
// so each elment in xxx_range is a avector,
// ValuesIn take each element (a vector) and combine them and feed them to test_p
// The combinations are  { {M, N, K, lda, ldb, ldc}, {alpha, beta}, {transA, transB}, {batch_count}
// }

TEST(pre_checkin_gemm_strided_batched_ex_bad_arg, float)
{
    testing_gemm_strided_batched_ex_bad_arg();
}

//--- small
// tests with stride_a == 0
INSTANTIATE_TEST_CASE_P(quick_blas3_small_stride_zero,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(small_matrix_size_stride_a_range),
                                ValuesIn(full_alpha_beta_range),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_n1_0_1_3),
                                ValuesIn(precision_type_range)));

INSTANTIATE_TEST_CASE_P(quick_blas3_small_no_stride_zero,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(small_matrix_size_range),
                                ValuesIn(full_alpha_beta_range),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_n1_0_1_3),
                                ValuesIn(precision_type_range)));

INSTANTIATE_TEST_CASE_P(known_bug_blas3_small,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(known_bug_small_matrix_size_range),
                                ValuesIn(full_alpha_beta_range),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_n1_0_1_3),
                                ValuesIn(precision_type_range)));
// tests with stride_a == 0
INSTANTIATE_TEST_CASE_P(pre_checkin_blas3_small_stride_zero,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(small_matrix_size_stride_a_range),
                                ValuesIn(full_alpha_beta_range),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_n1_0_1_3),
                                ValuesIn(precision_type_range)));
//--- medium
INSTANTIATE_TEST_CASE_P(pre_checkin_blas3_medium_no_stride_zero,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(medium_matrix_size_range),
                                ValuesIn(alpha_beta_2_3),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_63_64_65),
                                ValuesIn(precision_single)));
// tests with stride_a == 0
INSTANTIATE_TEST_CASE_P(nightly_blas3_medium_stride_zero,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(medium_matrix_size_stride_a_range),
                                ValuesIn(alpha_beta_2_3),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_31_32_33),
                                ValuesIn(precision_type_range)));

INSTANTIATE_TEST_CASE_P(nightly_checkin_blas3_medium,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(medium_matrix_size_range),
                                ValuesIn(alpha_beta_2_3),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_31_32_33),
                                ValuesIn(precision_type_range)));
//--- large
INSTANTIATE_TEST_CASE_P(pre_checkin_blas3_large,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(large_matrix_size_range),
                                ValuesIn(alpha_beta_2_3),
                                ValuesIn(full_transA_transB_range),
                                ValuesIn(batch_count_2),
                                ValuesIn(precision_type_range)));
// tests with stride_a == 0
INSTANTIATE_TEST_CASE_P(pre_checkin_blas3_large_stride_zero,
                        gemm_strided_batched_ex,
                        Combine(ValuesIn(large_matrix_size_stride_a_range),
                                ValuesIn(alpha_beta_2_3),
                                ValuesIn(transA_transB_NT),
                                ValuesIn(batch_count_2),
                                ValuesIn(precision_type_range)));

// INSTANTIATE_TEST_CASE_P(nightly_blas3_deepbench_sizes,
//                        gemm_strided_batched_ex,
//                        ValuesIn(deepbench_sb_vec));
