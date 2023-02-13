#include<iostream>
#include<string>

#include "CNF.hpp"

int main(int argc, char const** argv) {
    std::string const path(argv[1]);
    CNF cnf(path.c_str());

    cnf.simplify();
    cnf.compute_free_vars();

    auto sizes = cnf.get_nb_by_clause_len();
    auto vars = cnf.get_vars_by_clause_len();

    // No BCP:
    // double const intercept = 0.5834995534127562;
    // double const cr = 0.0009258170946816459;
    // std::vector<double> v = {0,
    //     -0.30092175339174243,
    //     -0.08968965737300559,
    //     -0.05299743266805441,
    //     0.0960854116021817,
    //     -0.0006375938024977149};
    
    // with BCP:
     double const intercept = 0.39868256396085533;
     double const cr = -0.001941491867362755;
     std::vector<double> v = {0,
         -0.14354043772150665,
         -0.04408040163500509,
         0.07113005343417171,
         -0.005845120241618398,
         };

    // for(std::size_t i = 0; i < sizes.size(); i++) {
    //     std::cout << i << ": " << sizes[i] << "\n    ";
    //     for(auto const& v : vars[i]) {
    //         std::cout << v << " ";
    //     }
    //     std::cout << "\n";
    // }

    double res = intercept;
    res += cr * cnf.get_nb_clauses() / vars[0].size();

    for(std::size_t i = 1; i < std::min(sizes.size(), v.size()); i++) {
        if(sizes[i] != 0)
            res += v[i] * sizes[i] / vars[i].size();
    }

    std::cout << res << "\n";
    std::cout << res * cnf.get_nb_vars() << "\n";

    return 0;
}
