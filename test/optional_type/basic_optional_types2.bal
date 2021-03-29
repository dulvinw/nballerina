// RUN: "%testRunScript" %s %nballerinacc "%java_path" | filecheck %s

public function print_string(string val) = external;

public function main() {
    string? str = "Hello World!";
    print_string("RESULT=");
    print_string(<string>str);
}

// CHECK: RESULT=Hello World!
