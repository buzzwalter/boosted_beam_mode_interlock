// Halide tutorial lesson 1: Getting started with Funcs, Vars, and Exprs
#include "Halide.h"
#include <stdio.h>

int main(int argc, char **argv) {
    Halide::Func gradient;
    Halide::Var x, y;
    Halide::Expr e = x + y;
    gradient(x, y) = e;

    // Create a more basic target without advanced CPU features
    Halide::Target target = Halide::get_host_target();
    // Disable advanced features that might not be supported
    target = target.without_feature(Halide::Target::SVE)
      .without_feature(Halide::Target::SVE2);
    
    // Set a conservative vector width
    target.vector_bits = 128;

    // Realize with the modified target
    Halide::Buffer<int32_t> output = gradient.realize({800, 600}, target);

    // Rest of your verification code remains the same
    for (int j = 0; j < output.height(); j++) {
        for (int i = 0; i < output.width(); i++) {
            if (output(i, j) != i + j) {
                printf("Something went wrong!\n"
                       "Pixel %d, %d was supposed to be %d, but instead it's %d\n",
                       i, j, i + j, output(i, j));
                return -1;
            }
        }
    }

    printf("Success!\n");
    return 0;
}
