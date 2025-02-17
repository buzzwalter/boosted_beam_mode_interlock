#include "Halide.h"
#include <iostream>

int main() {
    std::cout << "Halide Target: " << Halide::get_host_target().to_string() << std::endl;
    return 0;
}
