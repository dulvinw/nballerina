// RUN: "%testRunScript" %s %nballerinacc "%java_path" "%target_variant" "%skip_bir_gen" | filecheck %s

public function print_string(string val) = external;

public function print_integer(int val) = external;

public function main() {
    boolean[5] arr4 = [];
    arr4[0] = true;
    arr4[1] = false;
    boolean b = arr4[0];
    boolean c = arr4[1];
    if (b) {
        print_string("RESULT=");
    }
    if (!c) {
        print_integer(1);
    }
}
// CHECK: RESULT=1
