#pragma once

#include "../BBE/List.h"
#include "../BBE/String.h"
#include "../BBE/SimpleFile.h"
#include <ctime>

#define BBE_TEMPLATE_ESCAPE(...) __VA_ARGS__
#define BBE_SERIALIZABLE_DATA_IMPL_EXPAND(x) x
#define BBE_SERIALIZABLE_DATA_IMPL_CAT(a, ...) BBE_SERIALIZABLE_DATA_IMPL_CAT_2(a, __VA_ARGS__)
#define BBE_SERIALIZABLE_DATA_IMPL_CAT_2(a, ...) a##__VA_ARGS__
#define BBE_SERIALIZABLE_DATA_IMPL_SEQ() \
100, 99, 98, 97, 96, 95, 94, 93, 92, 91, \
 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, \
 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, \
 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, \
 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, \
 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, \
 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, \
 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, \
 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, \
 10,  9,  8,  7,  6,  5,  4,  3,  2,  1, \
  0

#define BBE_SERIALIZABLE_DATA_IMPL_ARG_FILL( \
 a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9, a10, \
a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, \
a31, a32, a33, a34, a35, a36, a37, a38, a39, a40, \
a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, \
a51, a52, a53, a54, a55, a56, a57, a58, a59, a60, \
a61, a62, a63, a64, a65, a66, a67, a68, a69, a70, \
a71, a72, a73, a74, a75, a76, a77, a78, a79, a80, \
a81, a82, a83, a84, a85, a86, a87, a88, a89, a90, \
a91, a92, a93, a94, a95, a96, a97, a98, a99, a100,\
N, ...) N

#define BBE_SERIALIZABLE_DATA_IMPL_NARG(...) BBE_SERIALIZABLE_DATA_IMPL_NARG_2(__VA_ARGS__, BBE_SERIALIZABLE_DATA_IMPL_SEQ())
#define BBE_SERIALIZABLE_DATA_IMPL_NARG_2(...) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_ARG_FILL(__VA_ARGS__))

#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_1(func, x, ...) func(x)
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_2(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_1(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_3(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_2(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_4(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_3(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_5(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_4(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_6(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_5(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_7(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_6(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_8(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_7(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_9(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_8(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_10(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_9(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_11(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_10(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_12(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_11(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_13(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_12(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_14(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_13(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_15(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_14(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_16(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_15(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_17(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_16(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_18(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_17(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_19(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_18(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_20(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_19(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_21(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_20(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_22(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_21(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_23(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_22(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_24(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_23(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_25(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_24(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_26(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_25(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_27(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_26(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_28(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_27(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_29(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_28(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_30(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_29(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_31(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_30(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_32(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_31(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_33(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_32(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_34(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_33(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_35(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_34(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_36(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_35(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_37(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_36(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_38(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_37(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_39(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_38(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_40(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_39(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_41(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_40(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_42(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_41(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_43(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_42(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_44(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_43(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_45(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_44(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_46(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_45(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_47(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_46(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_48(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_47(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_49(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_48(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_50(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_49(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_51(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_50(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_52(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_51(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_53(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_52(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_54(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_53(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_55(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_54(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_56(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_55(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_57(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_56(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_58(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_57(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_59(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_58(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_60(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_59(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_61(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_60(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_62(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_61(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_63(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_62(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_64(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_63(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_65(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_64(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_66(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_65(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_67(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_66(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_68(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_67(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_69(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_68(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_70(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_69(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_71(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_70(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_72(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_71(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_73(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_72(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_74(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_73(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_75(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_74(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_76(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_75(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_77(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_76(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_78(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_77(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_79(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_78(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_80(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_79(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_81(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_80(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_82(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_81(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_83(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_82(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_84(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_83(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_85(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_84(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_86(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_85(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_87(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_86(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_88(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_87(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_89(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_88(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_90(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_89(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_91(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_90(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_92(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_91(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_93(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_92(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_94(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_93(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_95(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_94(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_96(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_95(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_97(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_96(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_98(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_97(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_99(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_98(func, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_LOOP_100(func, x, ...) func(x) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_LOOP_99(func, __VA_ARGS__))

#define BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE_ARG_3(type, name, defaultVal) BBE_TEMPLATE_ESCAPE type name = defaultVal;
#define BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE_ARG_2(type, name) BBE_TEMPLATE_ESCAPE type name = {};
#define BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE_ARG_1(arg) arg
#define BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE(...) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_CAT(BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE_ARG_, BBE_SERIALIZABLE_DATA_IMPL_NARG(__VA_ARGS__))(__VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE_EXPAND(...) BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE __VA_ARGS__
#define BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_2(LOOP, ...) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(LOOP(BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_SINGLE_EXPAND, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_MEMBERS(NARGS, ...) BBE_SERIALIZABLE_DATA_IMPL_MEMBERS_2(BBE_SERIALIZABLE_DATA_IMPL_CAT(BBE_SERIALIZABLE_DATA_IMPL_LOOP_, NARGS), __VA_ARGS__)

#define BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE_ARG_3(type, name, defaultVal) desc.describe(name, defaultVal);
#define BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE_ARG_2(type, name) desc.describe(name);
#define BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE_ARG_1(arg)
#define BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE(...) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(BBE_SERIALIZABLE_DATA_IMPL_CAT(BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE_ARG_, BBE_SERIALIZABLE_DATA_IMPL_NARG(__VA_ARGS__))(__VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE_EXPAND(x) BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE x
#define BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_2(LOOP, ...) BBE_SERIALIZABLE_DATA_IMPL_EXPAND(LOOP(BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_SINGLE_EXPAND, __VA_ARGS__))
#define BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR(NARGS, ...) \
void serialDescription(bbe::SerializedDescription& desc) \
{ \
    BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR_2(BBE_SERIALIZABLE_DATA_IMPL_CAT(BBE_SERIALIZABLE_DATA_IMPL_LOOP_, NARGS), __VA_ARGS__) \
}

#define BBE_SERIALIZABLE_DATA_IMPL(NARGS, ...) BBE_SERIALIZABLE_DATA_IMPL_MEMBERS(NARGS, __VA_ARGS__) BBE_SERIALIZABLE_DATA_IMPL_DESCRIPTOR(NARGS, __VA_ARGS__)
#define BBE_SERIALIZABLE_DATA(...) BBE_SERIALIZABLE_DATA_IMPL(BBE_SERIALIZABLE_DATA_IMPL_NARG(__VA_ARGS__), __VA_ARGS__)

namespace bbe
{
	enum class Undoable
	{
		NO,
		YES,
	};

	template <typename T>
	class SerializableObject
	{
	private:
		bbe::String path;
		bbe::String paranoiaPath;
		T data;

		constexpr static bool hasSerialDescription = requires(T & t, bbe::SerializedDescription & desc) {
			t.serialDescription(desc);
		};

		void load()
		{
			if (bbe::simpleFile::doesFileExist(path))
			{
				bbe::ByteBuffer binary = bbe::simpleFile::readBinaryFile(path);
				auto span = binary.getSpan();

				if constexpr (hasSerialDescription)
				{
					bbe::SerializedDescription desc;
					data.serialDescription(desc);
					desc.writeFromSpan(span);
				}
				else
				{
					data = T::deserialize(span);
				}
			}
			else
			{
				writeToFile();
			}
		}

	public:
		SerializableObject(const bbe::String& path, const bbe::String& paranoiaPath = "") :
			path(path),
			paranoiaPath(paranoiaPath)
		{
			load();
		}

		T* operator->()
		{
			return &data;
		}

		void writeToFile()
		{
			bbe::ByteBuffer buffer;
			buffer.write(data);

			bbe::simpleFile::backup::async::writeBinaryToFile(path, buffer);
			if (paranoiaPath.getLength() != 0)
			{
				time_t t;
				time(&t);
				bbe::simpleFile::backup::async::createDirectory(paranoiaPath);
				bbe::simpleFile::backup::async::writeBinaryToFile(paranoiaPath + "/" + path + t + ".bak", buffer);
			}
		}
	};

	template <typename T>
	class SerializableList
	{
	private:
		bbe::String path;
		bbe::String paranoiaPath;
		bbe::List<T> data;

		bbe::List<bbe::List<T>> history;
		Undoable undoable = Undoable::NO;

		constexpr static bool hasSerialDescription = requires(T & t, bbe::SerializedDescription & desc) {
			t.serialDescription(desc);
		};

		void load(const bbe::String& path, const bbe::List<T>& data)
		{
			this->path = path;
			if (bbe::simpleFile::doesFileExist(path))
			{
				// TODO: Check if load is even required
				bbe::ByteBuffer binary = bbe::simpleFile::readBinaryFile(path);
				bbe::ByteBufferSpan span = binary.getSpan();
				while (span.hasMore())
				{
					int64_t size;
					span.read(size);
					auto subSpan = span.readSpan(size);

					if constexpr (hasSerialDescription)
					{
						T t;
						bbe::SerializedDescription desc;
						t.serialDescription(desc);
						desc.writeFromSpan(subSpan);
						this->data.add(t);
					}
					else
					{
						this->data.add(T::deserialize(subSpan));
					}
				}
			}
			else
			{
				this->data = data;
				writeToFile();
			}

			pushUndoable();
		}

		void pushUndoable()
		{
			if (undoable == Undoable::YES)
			{
				history.add(data);
			}
		}

		SerializableList()
		{
			// Do nothing;
		}

	public:
		SerializableList(const bbe::String& path, const bbe::String& paranoiaPath = "", Undoable undoable = Undoable::NO) :
			paranoiaPath(paranoiaPath),
			undoable(undoable)
		{
			load(path, {});
		}

		static SerializableList withDefault(const bbe::String& path, const bbe::List<T>& data, const bbe::String& paranoiaPath = "", Undoable undoable = Undoable::NO)
		{
			SerializableList sl;
			sl.paranoiaPath = paranoiaPath;
			sl.undoable = undoable;
			sl.load(path, data);
			return sl;
		}

		void add(T /*copy*/ t)
		{
			data.add(std::move(t));
			bbe::ByteBuffer buffer;
			auto token = buffer.reserveSizeToken();
			buffer.write(data.last());
			buffer.fillSizeToken(token);
			bbe::simpleFile::backup::async::appendBinaryToFile(path, buffer);
			pushUndoable();
		}

		bool removeIndex(size_t index)
		{
			if (data.removeIndex(index))
			{
				writeToFile();
				return true;
			}
			return false;
		}

		bool swap(size_t a, size_t b)
		{
			bool retVal = data.swap(a, b);
			if(retVal) writeToFile();
			return retVal;
		}

		size_t getLength() const
		{
			return data.getLength();
		}

		T& operator[](size_t index)
		{
			return data[index];
		}

		const T& operator[](size_t index) const
		{
			return data[index];
		}

		void writeToFile(bool updateHistory = true)
		{
			bbe::ByteBuffer buffer;
			for (size_t i = 0; i < data.getLength(); i++)
			{
				auto token = buffer.reserveSizeToken();
				buffer.write(data[i]);
				buffer.fillSizeToken(token);
			}
			bbe::simpleFile::backup::async::writeBinaryToFile(path, buffer);
			if (paranoiaPath.getLength() != 0)
			{
				time_t t;
				time(&t);
				bbe::simpleFile::backup::async::createDirectory(paranoiaPath);
				bbe::simpleFile::backup::async::writeBinaryToFile(paranoiaPath + "/" + path + t + ".bak", buffer);
			}
			
			if(updateHistory) pushUndoable();
		}

		bool canUndo() const
		{
			return history.getLength() > 1;
		}

		bool undo()
		{
			if (!canUndo()) return false;

			history.popBack();
			data = history.last();
			writeToFile(false);
			return true;
		}

		const bbe::List<T>& getList() const
		{
			return data;
		}
	};
}
