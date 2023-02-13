#include<iostream>
#include<string>

#include "CNF.hpp"

int main(int argc, char const** argv) {
    std::string const path(argv[1]);
    CNF cnf(path.c_str());
    cnf.simplify();
    cnf.compute_free_vars();
    return 0;
}
