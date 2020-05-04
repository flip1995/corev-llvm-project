; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+sse2 | FileCheck %s --check-prefixes=CHECK,SSE
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx  | FileCheck %s --check-prefixes=CHECK,AVX

define { i64, i64 } @PR46189(double %0, double %1, double %2, double %3, double %4) {
; SSE-LABEL: PR46189:
; SSE:       # %bb.0:
; SSE-NEXT:    movapd %xmm0, %xmm5
; SSE-NEXT:    subsd %xmm2, %xmm5
; SSE-NEXT:    addsd %xmm2, %xmm0
; SSE-NEXT:    unpcklpd {{.*#+}} xmm5 = xmm5[0],xmm0[0]
; SSE-NEXT:    unpcklpd {{.*#+}} xmm3 = xmm3[0,0]
; SSE-NEXT:    divpd %xmm3, %xmm5
; SSE-NEXT:    cvttpd2dq %xmm5, %xmm0
; SSE-NEXT:    movapd %xmm1, %xmm3
; SSE-NEXT:    subsd %xmm2, %xmm3
; SSE-NEXT:    addsd %xmm2, %xmm1
; SSE-NEXT:    unpcklpd {{.*#+}} xmm3 = xmm3[0],xmm1[0]
; SSE-NEXT:    unpcklpd {{.*#+}} xmm4 = xmm4[0,0]
; SSE-NEXT:    divpd %xmm4, %xmm3
; SSE-NEXT:    cvttpd2dq %xmm3, %xmm1
; SSE-NEXT:    unpcklps {{.*#+}} xmm0 = xmm0[0],xmm1[0],xmm0[1],xmm1[1]
; SSE-NEXT:    movq %xmm0, %rax
; SSE-NEXT:    pshufd {{.*#+}} xmm0 = xmm0[2,3,0,1]
; SSE-NEXT:    movq %xmm0, %rdx
; SSE-NEXT:    retq
;
; AVX-LABEL: PR46189:
; AVX:       # %bb.0:
; AVX-NEXT:    vsubsd %xmm2, %xmm0, %xmm5
; AVX-NEXT:    vaddsd %xmm2, %xmm0, %xmm0
; AVX-NEXT:    vunpcklpd {{.*#+}} xmm0 = xmm5[0],xmm0[0]
; AVX-NEXT:    vmovddup {{.*#+}} xmm3 = xmm3[0,0]
; AVX-NEXT:    vdivpd %xmm3, %xmm0, %xmm0
; AVX-NEXT:    vcvttpd2dq %xmm0, %xmm0
; AVX-NEXT:    vsubsd %xmm2, %xmm1, %xmm3
; AVX-NEXT:    vaddsd %xmm2, %xmm1, %xmm1
; AVX-NEXT:    vunpcklpd {{.*#+}} xmm1 = xmm3[0],xmm1[0]
; AVX-NEXT:    vmovddup {{.*#+}} xmm2 = xmm4[0,0]
; AVX-NEXT:    vdivpd %xmm2, %xmm1, %xmm1
; AVX-NEXT:    vcvttpd2dq %xmm1, %xmm1
; AVX-NEXT:    vunpcklps {{.*#+}} xmm0 = xmm0[0],xmm1[0],xmm0[1],xmm1[1]
; AVX-NEXT:    vmovq %xmm0, %rax
; AVX-NEXT:    vpextrq $1, %xmm0, %rdx
; AVX-NEXT:    retq
  %6 = insertelement <2 x double> undef, double %0, i32 0
  %7 = shufflevector <2 x double> %6, <2 x double> undef, <2 x i32> zeroinitializer
  %8 = insertelement <2 x double> undef, double %2, i32 0
  %9 = shufflevector <2 x double> %8, <2 x double> undef, <2 x i32> zeroinitializer
  %10 = fsub <2 x double> %7, %9
  %11 = fadd <2 x double> %7, %9
  %12 = shufflevector <2 x double> %10, <2 x double> %11, <2 x i32> <i32 0, i32 3>
  %13 = insertelement <2 x double> undef, double %3, i32 0
  %14 = shufflevector <2 x double> %13, <2 x double> undef, <2 x i32> zeroinitializer
  %15 = fdiv <2 x double> %12, %14
  %16 = fptosi <2 x double> %15 to <2 x i32>
  %17 = insertelement <2 x double> undef, double %1, i32 0
  %18 = shufflevector <2 x double> %17, <2 x double> undef, <2 x i32> zeroinitializer
  %19 = fsub <2 x double> %18, %9
  %20 = fadd <2 x double> %18, %9
  %21 = shufflevector <2 x double> %19, <2 x double> %20, <2 x i32> <i32 0, i32 3>
  %22 = insertelement <2 x double> undef, double %4, i32 0
  %23 = shufflevector <2 x double> %22, <2 x double> undef, <2 x i32> zeroinitializer
  %24 = fdiv <2 x double> %21, %23
  %25 = fptosi <2 x double> %24 to <2 x i32>
  %26 = zext <2 x i32> %25 to <2 x i64>
  %27 = shl nuw <2 x i64> %26, <i64 32, i64 32>
  %28 = zext <2 x i32> %16 to <2 x i64>
  %29 = or <2 x i64> %27, %28
  %30 = extractelement <2 x i64> %29, i32 0
  %31 = insertvalue { i64, i64 } undef, i64 %30, 0
  %32 = extractelement <2 x i64> %29, i32 1
  %33 = insertvalue { i64, i64 } %31, i64 %32, 1
  ret { i64, i64 } %33
}
