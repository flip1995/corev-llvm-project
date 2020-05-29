// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py
// RUN: %clang_cc1 -fenable-matrix -triple x86_64-apple-darwin %s -emit-llvm -disable-llvm-passes -o - -std=c++11 | FileCheck %s

template <typename EltTy, unsigned Rows, unsigned Columns>
struct MyMatrix {
  using matrix_t = EltTy __attribute__((matrix_type(Rows, Columns)));

  matrix_t value;
};

template <typename EltTy0, unsigned R0, unsigned C0>
typename MyMatrix<EltTy0, R0, C0>::matrix_t add(MyMatrix<EltTy0, R0, C0> &A, MyMatrix<EltTy0, R0, C0> &B) {
  return A.value + B.value;
}

void test_add_template() {
  // CHECK-LABEL: define void @_Z17test_add_templatev()
  // CHECK:       %call = call <10 x float> @_Z3addIfLj2ELj5EEN8MyMatrixIT_XT0_EXT1_EE8matrix_tERS2_S4_(%struct.MyMatrix* nonnull align 4 dereferenceable(40) %Mat1, %struct.MyMatrix* nonnull align 4 dereferenceable(40) %Mat2)

  // CHECK-LABEL: define linkonce_odr <10 x float> @_Z3addIfLj2ELj5EEN8MyMatrixIT_XT0_EXT1_EE8matrix_tERS2_S4_(
  // CHECK:       [[MAT1:%.*]] = load <10 x float>, <10 x float>* {{.*}}, align 4
  // CHECK:       [[MAT2:%.*]] = load <10 x float>, <10 x float>* {{.*}}, align 4
  // CHECK-NEXT:  [[RES:%.*]] = fadd <10 x float> [[MAT1]], [[MAT2]]
  // CHECK-NEXT:  ret <10 x float> [[RES]]

  MyMatrix<float, 2, 5> Mat1;
  MyMatrix<float, 2, 5> Mat2;
  Mat1.value = add(Mat1, Mat2);
}

template <typename EltTy0, unsigned R0, unsigned C0>
typename MyMatrix<EltTy0, R0, C0>::matrix_t subtract(MyMatrix<EltTy0, R0, C0> &A, MyMatrix<EltTy0, R0, C0> &B) {
  return A.value - B.value;
}

void test_subtract_template() {
  // CHECK-LABEL: define void @_Z22test_subtract_templatev()
  // CHECK:       %call = call <10 x float> @_Z8subtractIfLj2ELj5EEN8MyMatrixIT_XT0_EXT1_EE8matrix_tERS2_S4_(%struct.MyMatrix* nonnull align 4 dereferenceable(40) %Mat1, %struct.MyMatrix* nonnull align 4 dereferenceable(40) %Mat2)

  // CHECK-LABEL: define linkonce_odr <10 x float> @_Z8subtractIfLj2ELj5EEN8MyMatrixIT_XT0_EXT1_EE8matrix_tERS2_S4_(
  // CHECK:       [[MAT1:%.*]] = load <10 x float>, <10 x float>* {{.*}}, align 4
  // CHECK:       [[MAT2:%.*]] = load <10 x float>, <10 x float>* {{.*}}, align 4
  // CHECK-NEXT:  [[RES:%.*]] = fsub <10 x float> [[MAT1]], [[MAT2]]
  // CHECK-NEXT:  ret <10 x float> [[RES]]

  MyMatrix<float, 2, 5> Mat1;
  MyMatrix<float, 2, 5> Mat2;
  Mat1.value = subtract(Mat1, Mat2);
}

struct DoubleWrapper1 {
  int x;
  operator double() {
    return x;
  }
};

void test_DoubleWrapper1_Sub1(MyMatrix<double, 10, 9> &m) {
  // CHECK-LABEL: define void @_Z24test_DoubleWrapper1_Sub1R8MyMatrixIdLj10ELj9EE(
  // CHECK:       [[MATRIX:%.*]] = load <90 x double>, <90 x double>* {{.*}}, align 8
  // CHECK:       [[SCALAR:%.*]] = call double @_ZN14DoubleWrapper1cvdEv(%struct.DoubleWrapper1* %w1)
  // CHECK-NEXT:  [[SCALAR_EMBED:%.*]] = insertelement <90 x double> undef, double [[SCALAR]], i32 0
  // CHECK-NEXT:  [[SCALAR_EMBED1:%.*]] = shufflevector <90 x double> [[SCALAR_EMBED]], <90 x double> undef, <90 x i32> zeroinitializer
  // CHECK-NEXT:  [[RES:%.*]] = fsub <90 x double> [[MATRIX]], [[SCALAR_EMBED1]]
  // CHECK:       store <90 x double> [[RES]], <90 x double>* {{.*}}, align 8

  DoubleWrapper1 w1;
  w1.x = 10;
  m.value = m.value - w1;
}

void test_DoubleWrapper1_Sub2(MyMatrix<double, 10, 9> &m) {
  // CHECK-LABEL: define void @_Z24test_DoubleWrapper1_Sub2R8MyMatrixIdLj10ELj9EE(
  // CHECK:       [[SCALAR:%.*]] = call double @_ZN14DoubleWrapper1cvdEv(%struct.DoubleWrapper1* %w1)
  // CHECK:       [[MATRIX:%.*]] = load <90 x double>, <90 x double>* {{.*}}, align 8
  // CHECK-NEXT:  [[SCALAR_EMBED:%.*]] = insertelement <90 x double> undef, double [[SCALAR]], i32 0
  // CHECK-NEXT:  [[SCALAR_EMBED1:%.*]] = shufflevector <90 x double> [[SCALAR_EMBED]], <90 x double> undef, <90 x i32> zeroinitializer
  // CHECK-NEXT:  [[RES:%.*]] = fsub <90 x double> [[SCALAR_EMBED1]], [[MATRIX]]
  // CHECK:       store <90 x double> [[RES]], <90 x double>* {{.*}}, align 8

  DoubleWrapper1 w1;
  w1.x = 10;
  m.value = w1 - m.value;
}

struct DoubleWrapper2 {
  int x;
  operator double() {
    return x;
  }
};

void test_DoubleWrapper2_Add1(MyMatrix<double, 10, 9> &m) {
  // CHECK-LABEL: define void @_Z24test_DoubleWrapper2_Add1R8MyMatrixIdLj10ELj9EE(
  // CHECK:       [[MATRIX:%.*]] = load <90 x double>, <90 x double>* %1, align 8
  // CHECK:       [[SCALAR:%.*]] = call double @_ZN14DoubleWrapper2cvdEv(%struct.DoubleWrapper2* %w2)
  // CHECK-NEXT:  [[SCALAR_EMBED:%.*]] = insertelement <90 x double> undef, double [[SCALAR]], i32 0
  // CHECK-NEXT:  [[SCALAR_EMBED1:%.*]] = shufflevector <90 x double> [[SCALAR_EMBED]], <90 x double> undef, <90 x i32> zeroinitializer
  // CHECK-NEXT:  [[RES:%.*]] = fadd <90 x double> [[MATRIX]], [[SCALAR_EMBED1]]
  // CHECK:       store <90 x double> [[RES]], <90 x double>* {{.*}}, align 8

  DoubleWrapper2 w2;
  w2.x = 20;
  m.value = m.value + w2;
}

void test_DoubleWrapper2_Add2(MyMatrix<double, 10, 9> &m) {
  // CHECK-LABEL: define void @_Z24test_DoubleWrapper2_Add2R8MyMatrixIdLj10ELj9EE(
  // CHECK:       [[SCALAR:%.*]] = call double @_ZN14DoubleWrapper2cvdEv(%struct.DoubleWrapper2* %w2)
  // CHECK:       [[MATRIX:%.*]] = load <90 x double>, <90 x double>* %1, align 8
  // CHECK-NEXT:  [[SCALAR_EMBED:%.*]] = insertelement <90 x double> undef, double [[SCALAR]], i32 0
  // CHECK-NEXT:  [[SCALAR_EMBED1:%.*]] = shufflevector <90 x double> [[SCALAR_EMBED]], <90 x double> undef, <90 x i32> zeroinitializer
  // CHECK-NEXT:  [[RES:%.*]] = fadd <90 x double> [[SCALAR_EMBED1]], [[MATRIX]]
  // CHECK:       store <90 x double> [[RES]], <90 x double>* {{.*}}, align 8

  DoubleWrapper2 w2;
  w2.x = 20;
  m.value = w2 + m.value;
}

struct IntWrapper {
  char x;
  operator int() {
    return x;
  }
};

void test_IntWrapper_Add(MyMatrix<double, 10, 9> &m) {
  // CHECK-LABEL: define void @_Z19test_IntWrapper_AddR8MyMatrixIdLj10ELj9EE(
  // CHECK:       [[MATRIX:%.*]] = load <90 x double>, <90 x double>* {{.*}}, align 8
  // CHECK:       [[SCALAR:%.*]] = call i32 @_ZN10IntWrappercviEv(%struct.IntWrapper* %w3)
  // CHECK:       [[SCALAR_FP:%.*]] = sitofp i32 %call to double
  // CHECK-NEXT:  [[SCALAR_EMBED:%.*]] = insertelement <90 x double> undef, double [[SCALAR_FP]], i32 0
  // CHECK-NEXT:  [[SCALAR_EMBED1:%.*]] = shufflevector <90 x double> [[SCALAR_EMBED]], <90 x double> undef, <90 x i32> zeroinitializer
  // CHECK-NEXT:  [[RES:%.*]] = fadd <90 x double> [[MATRIX]], [[SCALAR_EMBED1]]
  // CHECK:       store <90 x double> [[RES]], <90 x double>* {{.*}}, align 8

  IntWrapper w3;
  w3.x = 'c';
  m.value = m.value + w3;
}

void test_IntWrapper_Sub(MyMatrix<double, 10, 9> &m) {
  // CHECK-LABEL: define void @_Z19test_IntWrapper_SubR8MyMatrixIdLj10ELj9EE(
  // CHECK:       [[SCALAR:%.*]] = call i32 @_ZN10IntWrappercviEv(%struct.IntWrapper* %w3)
  // CHECK-NEXT:  [[SCALAR_FP:%.*]] = sitofp i32 %call to double
  // CHECK:       [[MATRIX:%.*]] = load <90 x double>, <90 x double>* {{.*}}, align 8
  // CHECK-NEXT:  [[SCALAR_EMBED:%.*]] = insertelement <90 x double> undef, double [[SCALAR_FP]], i32 0
  // CHECK-NEXT:  [[SCALAR_EMBED1:%.*]] = shufflevector <90 x double> [[SCALAR_EMBED]], <90 x double> undef, <90 x i32> zeroinitializer
  // CHECK-NEXT:  [[RES:%.*]] = fsub <90 x double> [[SCALAR_EMBED1]], [[MATRIX]]
  // CHECK:       store <90 x double> [[RES]], <90 x double>* {{.*}}, align 8

  IntWrapper w3;
  w3.x = 'c';
  m.value = w3 - m.value;
}