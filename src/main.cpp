#include<iostream>
#include<string>

#include "CNF.hpp"

int main(int argc, char const** argv) {
    if(argc < 2) {
        std::cerr << "too few arguments\nexiting\n";
        exit(1);
    }
    std::string const path(argv[1]);
    CNF cnf(path.c_str());

    cnf.simplify();
    cnf.compute_free_vars();
    //std::cout << cnf;

    //cnf.compute_free_vars();

    auto sizes = cnf.get_nb_by_clause_len();
    auto vars = cnf.get_vars_by_clause_len();

    //// No BCP:
    //// double const intercept = 0.5834995534127562;
    //// double const cr = 0.0009258170946816459;
    //// std::vector<double> v = {0,
    ////     -0.30092175339174243,
    ////     -0.08968965737300559,
    ////     -0.05299743266805441,
    ////     0.0960854116021817,
    ////     -0.0006375938024977149};
    
    // with BCP:
     double const intercept = 0.4784764681683733;
     double const cr = 0.004870940193116897;
     std::vector<double> v = {0,
         0,
         -0.1662521139035124,
         -0.07974636487792655,
         0.028067156465960284,
         0.0017585449812209347,
         };

     //for(std::size_t i = 0; i < sizes.size(); i++) {
         //std::cout << i << ": " << sizes[i] << "\n    ";
         //for(auto const& v : vars[i]) {
             //std::cout << v << " ";
         //}
         //std::cout << "\n";
     //}

    double res = intercept;
    if(vars[0].size() > 0) {
        res += cr * cnf.nb_active_clauses() / vars[0].size();
    }

    for(std::size_t i = 2; i < std::min(sizes.size(), v.size()); i++) {
        //if(vars[i].size() != 0)
        res += v[i] * sizes[i] / (vars[i].size() + 1);
    }

    ////std::cout << res << "\n";
    std::cout << res * vars[0].size() + cnf.nb_free_vars() << "\n";

    return 0;
}
