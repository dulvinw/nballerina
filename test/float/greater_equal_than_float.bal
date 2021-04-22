// RUN: "%testRunScript" %s %nballerinacc "%java_path" "%target_variant" "%skip_bir_gen" | filecheck %s

public function print_string(string val) = external;

public function printf64(float val) = external;

public function main() {
    float a = 10.5;
    float b = 5.5;
    print_string("RESULT=");
    if (a >= b) {
        printf64(a);
    }
    else {
        printf64(b);
    }
}
// CHECK: RESULT=10.5

