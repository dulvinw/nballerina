; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --function-signature --scrub-attributes --check-attributes
; RUN: opt -attributor -attributor-manifest-internal  -attributor-max-iterations-verify -attributor-annotate-decl-cs -attributor-max-iterations=2 -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_CGSCC_NPM,NOT_CGSCC_OPM,NOT_TUNIT_NPM,IS__TUNIT____,IS________OPM,IS__TUNIT_OPM
; RUN: opt -aa-pipeline=basic-aa -passes=attributor -attributor-manifest-internal  -attributor-max-iterations-verify -attributor-annotate-decl-cs -attributor-max-iterations=2 -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_CGSCC_OPM,NOT_CGSCC_NPM,NOT_TUNIT_OPM,IS__TUNIT____,IS________NPM,IS__TUNIT_NPM
; RUN: opt -attributor-cgscc -attributor-manifest-internal  -attributor-annotate-decl-cs -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_TUNIT_NPM,NOT_TUNIT_OPM,NOT_CGSCC_NPM,IS__CGSCC____,IS________OPM,IS__CGSCC_OPM
; RUN: opt -aa-pipeline=basic-aa -passes=attributor-cgscc -attributor-manifest-internal  -attributor-annotate-decl-cs -S < %s | FileCheck %s --check-prefixes=CHECK,NOT_TUNIT_NPM,NOT_TUNIT_OPM,NOT_CGSCC_OPM,IS__CGSCC____,IS________NPM,IS__CGSCC_NPM
;
; When a function is marked `alwaysinline` and is able to be inlined,
; we can IPO its boundaries

; the function is not exactly defined, and marked alwaysinline and can be inlined,
; so the function can be analyzed
define linkonce void @inner1() alwaysinline {
; IS__TUNIT____: Function Attrs: alwaysinline nofree nosync nounwind readnone willreturn
; IS__TUNIT____-LABEL: define {{[^@]+}}@inner1()
; IS__TUNIT____-NEXT:  entry:
; IS__TUNIT____-NEXT:    ret void
;
; IS__CGSCC____: Function Attrs: alwaysinline nofree norecurse nosync nounwind readnone willreturn
; IS__CGSCC____-LABEL: define {{[^@]+}}@inner1()
; IS__CGSCC____-NEXT:  entry:
; IS__CGSCC____-NEXT:    ret void
;
entry:
  ret void
}

define void @outer1() {
; IS__TUNIT____: Function Attrs: nofree nosync nounwind readnone willreturn
; IS__TUNIT____-LABEL: define {{[^@]+}}@outer1()
; IS__TUNIT____-NEXT:  entry:
; IS__TUNIT____-NEXT:    ret void
;
; IS__CGSCC____: Function Attrs: nofree norecurse nosync nounwind readnone willreturn
; IS__CGSCC____-LABEL: define {{[^@]+}}@outer1()
; IS__CGSCC____-NEXT:  entry:
; IS__CGSCC____-NEXT:    ret void
;
entry:
  call void @inner1()
  ret void
}

; The function is not alwaysinline and is not exactly defined
; so it will not be analyzed
define linkonce i32 @inner2() {
; CHECK-LABEL: define {{[^@]+}}@inner2()
; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret i32 1
;
entry:
  ret i32 1
}

; CHECK-NOT: Function Attrs
define i32 @outer2() {
; CHECK-LABEL: define {{[^@]+}}@outer2()
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[R:%.*]] = call i32 @inner2()
; CHECK-NEXT:    ret i32 [[R]]
;
entry:
  %r = call i32 @inner2() alwaysinline
  ret i32 %r
}

; This function cannot be inlined although it is marked alwaysinline
; it is `unexactly defined` and alwaysinline but cannot be inlined.
; so it will not be analyzed
define linkonce i32 @inner3(i8* %addr) alwaysinline {
; CHECK: Function Attrs: alwaysinline
; CHECK-LABEL: define {{[^@]+}}@inner3
; CHECK-SAME: (i8* [[ADDR:%.*]])
; CHECK-NEXT:  entry:
; CHECK-NEXT:    indirectbr i8* [[ADDR]], [label [[ONE:%.*]], label %two]
; CHECK:       one:
; CHECK-NEXT:    ret i32 42
; CHECK:       two:
; CHECK-NEXT:    ret i32 44
;
entry:
  indirectbr i8* %addr, [ label %one, label %two ]

one:
  ret i32 42

two:
  ret i32 44
}

define i32 @outer3(i32 %x) {
; CHECK-LABEL: define {{[^@]+}}@outer3
; CHECK-SAME: (i32 [[X:%.*]])
; CHECK-NEXT:    [[CMP:%.*]] = icmp slt i32 [[X]], 42
; CHECK-NEXT:    [[ADDR:%.*]] = select i1 [[CMP]], i8* blockaddress(@inner3, [[ONE:%.*]]), i8* blockaddress(@inner3, [[TWO:%.*]])
; CHECK-NEXT:    [[CALL:%.*]] = call i32 @inner3(i8* [[ADDR]])
; CHECK-NEXT:    ret i32 [[CALL]]
;
  %cmp = icmp slt i32 %x, 42
  %addr = select i1 %cmp, i8* blockaddress(@inner3, %one), i8* blockaddress(@inner3, %two)
  %call = call i32 @inner3(i8* %addr)
  ret i32 %call
}
