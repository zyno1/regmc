//
// Created by oz on 1/12/22.
//

#include "CNF.hpp"

#include<fstream>
#include<iostream>
#include<sstream>
#include<algorithm>

Literal::Literal (Variable v) : l((v.get() << 1)) {
}

Literal::Literal (Variable v, int sign) : l((v.get() << 1) | (sign < 0 ? -1 : 1)) {
}

Literal::Literal (int i) : l(((std::abs(i) - 1) << 1) ^ (i < 0 ? 1 : 0)) {
}

bool Literal::operator == (Literal p) const {
    return l == p.get();
}

bool Literal::operator != (Literal p) const {
    return l != p.get();
}

// '<' makes p, ~p adjacent in the ordering.
bool Literal::operator <  (Literal p) const {
    return l < p.get();
}  

Variable::Variable(int i) : v(std::abs(i) - 1) {
}

Variable::Variable(Literal l) : v(l.get() >> 1) {
}

bool Variable::operator == (Variable p) const {
    return v == p.v;
}

bool Variable::operator != (Variable p) const {
    return v != p.v;
}

bool Variable::operator <  (Variable p) const {
    return v < p.v;
}

Clause::Clause() {
}

void Clause::push(Literal const& l) {
    c.push_back(l);
}

void Clause::remove(Literal const& l) {
    auto it = std::remove(c.begin(), c.end(), l);
    c.erase(it, c.end());
}

void Clause::remove(Variable const& v) {
    auto it = std::remove_if(c.begin(), c.end()
            , [v] (Literal const& li) {
                return Variable(li) == v;
            });
    c.erase(it, c.end());
}

//bool is_unit(Clause const& c) {
    //return c.size() == 1;
//}

//bool is_empty(Clause const& c) {
    //return c.empty();
//}

CNF::CNF(char const* path) {
    std::ifstream f(path);

    if(!f) {
        std::cerr << "error opening file " << path << "\n";
        return;
    }

    int nb_vars;
    std::string line;
    while(getline(f, line)) {
        if(line.rfind("p cnf ", 0) == 0) {
            std::stringstream iss(line);
            std::string tmp;
            iss >> tmp >> tmp;
            iss >> nb_vars;

            idx.reserve(2 * nb_vars);

            for(int i = 1; i <= nb_vars; i++) {
                auto tmp = Variable(i);
                vars.insert(tmp);

                idx.push_back({});
                idx.push_back({});
            }
        }
        else if(line.rfind("c ind ", 0) == 0) {
            std::stringstream iss(line);
            std::string tmp;

            iss >> tmp >> tmp;
            while(iss) {
                int v;
                iss >> v;
                if(v != 0) {
                    ind.insert(Variable(v));
                }
            }
        }
        else if(line[0] != 'c' && line[0] != 'p') {
            Clause clause;
            std::stringstream iss(line);

            while(iss) {
                int v;
                iss >> v;
                if(v != 0) {
                    auto tmp = Literal(v);
                    clause.push(tmp);

                    idx[tmp.get()].insert(clauses.size());
                }
            }

            clauses.push_back(clause);
            active.push_back(true);
        }
    }

    nb_active = active.size();
    //simplify();
    //compute_free_vars();
}

std::ostream & operator<<(std::ostream & out, CNF const& cnf) {
    out << "p cnf " << cnf.vars.size() << " " << (cnf.nb_active + cnf.units.size());
    for(std::size_t i = 0; i < cnf.clauses.size(); i++) {
        if(cnf.active[i]) {
            out << "\n" << cnf.clauses[i];
        }
    }

    for(Literal const& l : cnf.units) {
        out << "\n" << l << " 0";
    }

    for(Variable const& v : cnf.free) {
        out << "\nc " << v;
    }

    return out;
}

void CNF::simplify() {
    bool change = true;
    while(change) {
        change = false;
        std::set<Literal> u;

        for(std::size_t i = 0; i < clauses.size(); i++) {
            if(active[i] && clauses[i].size() == 1) {
                u.insert(clauses[i][0]);
                units.insert(clauses[i][0]);
                active[i] = false;
                nb_active -= 1;
            }
        }

        change = u.size() != 0;

        for(Literal const& l : u) {
            Literal nl = ~l;

            if(l.get() >= idx.size()) {
                std::cout << "error\n";
                exit(-1);
            }
            if(idx[l.get()].empty()) {
                std::cout << "empty\n";
                exit(-1);
            }
            for(std::size_t i : idx[l.get()]) {
                if(active[i]) {
                    nb_active -= 1;
                }
                active[i] = false;

                for(Literal const& li : clauses[i]) {
                    if(li != l) {
                        idx[li.get()].erase(i);
                    }
                }
            }
            idx[l.get()].clear();

            for(std::size_t id : idx[(nl).get()]) {
                clauses[id].remove(nl);
            }
            idx[(nl).get()].clear();
        }
    }
}

void CNF::compute_free_vars() {
    free = vars;

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            for(Literal const& l : clauses[i]) {
                free.erase(Variable(l));
            }
        }
    }

    for(Literal const& l : units) {
        free.erase(Variable(l));
    }
}

std::vector<std::size_t> CNF::get_nb_by_clause_len() const {
    std::vector<std::size_t> res;
    res.push_back(0);
    res.push_back(units.size());

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            Clause const& c = clauses[i];

            while(c.size() >= res.size()) {
                res.push_back(0);
            }
            res[c.size()]++;
        }
    }

    return res;
}

std::vector<std::set<Variable> > CNF::get_vars_by_clause_len() const {
    std::vector<std::set<Variable> > res;
    res.reserve(vars.size());
    res.push_back({});
    res.push_back({});

    for(Literal const& l : units) {
        res[1].insert(Variable(l));
    }

    for(std::size_t i = 0; i < clauses.size(); i++) {
        if(active[i]) {
            Clause const& c = clauses[i];

            while(c.size() >= res.size()) {
                res.push_back({});
            }
            for(auto const& l : c) {
                res[c.size()].insert(Variable(l));
                res[0].insert(Variable(l));
            }
        }
    }

    return res;
}

//void CNF::compute_free_vars() {
//}

//std::size_t CNF::get_nb_vars() const {
    //return vars.size();
//}

//std::size_t CNF::get_nb_clauses() const {
    //return cnf.size();
//}

//std::size_t CNF::get_nb_units() const {
    //return units.size();
//}

//bool CNF::unsat() const {
    //return std::any_of(cnf.begin(), cnf.end(), is_empty);
//}

//void CNF::compute_free_vars() {
    //free.clear();
    //free = vars;

    //for(Clause const& c : cnf) {
        //for(Literal l : c) {
            //free.erase(std::abs(l));
        //}
    //}

    //for(Literal l : units) {
        //free.erase(std::abs(l));
    //}
//}

//bool CNF::simplify(Literal backbone_lit) {
    //bool res = false;
    //auto it = std::remove_if(cnf.begin(), cnf.end(), [&](Clause & c) {
        //if(c.find(backbone_lit) != c.end()) {
            //res = true;
            //return true;
        //}
        //else {
            //bool tmp = c.erase(-backbone_lit) >= 1;
            //res = res || tmp;
            //return false;
        //}
    //});

    ////vars.erase(std::abs(backbone_lit));

    //cnf.erase(it, cnf.end());
    //units.insert(backbone_lit);

    //return res;
//}

//bool CNF::simplify() {
    //bool res = false;
    //std::vector<Literal> tmp;

    //auto it = std::remove_if(cnf.begin(), cnf.end(), [&](Clause const& c) {
        //if(is_unit(c)) {
            //tmp.push_back(*(c.begin()));
            //res = true;
            //return true;
        //}
        //return false;
    //});

    //while (!tmp.empty()) {
        //cnf.erase(it, cnf.end());
        //for(Literal l : tmp) {
            //units.insert(l);
            //bool tt = simplify(l);
            //res = res || tt;
        //}
        //tmp.clear();
        //it = std::remove_if(cnf.begin(), cnf.end(), [&](Clause const& c) {
            //if(is_unit(c)) {
                //tmp.push_back(*(c.begin()));
                //res = true;
                //return true;
            //}
            //return false;
        //});
    //}

    //return res;
//}

//bool intersects(std::set<int> const& a, std::set<int> const& b) {
    //return std::any_of(a.begin(), a.end(), [&](int i) {
        //return b.find(i) != b.end();
    //});
//}

//void merge(std::set<int> & a, std::set<int> const& b) {
    //for(int i : b) {
        //a.insert(i);
    //}
//}

//void CNF::add_clause(Clause const& c) {
    //cnf.push_back(c);
//}

//std::set<Literal> get_conflicting_variables(Clause const& c1, Clause const& c2) {
    //std::set<Literal> res;

    //for(Literal l1 : c1) {
        //for(Literal l2 : c2) {
            //if(l1 == -l2) {
                //res.insert(l1);
            //}
        //}
    //}

    //return res;
//}

//bool is_trivial(Clause const& c) {
    //for(Literal l1 : c) {
        //for(Literal l2 : c) {
            //if(l1 == -l2) {
                //return true;
            //}
        //}
    //}
    //return false;
//}

//bool is_strict_subset(std::set<int> const& a, std::set<int> const& b) {
    //return a.size() < b.size() && std::all_of(a.begin(), a.end(), [&](int i) {
        //return b.find(i) != b.end();
    //});
//}

//void print_cl_sub(Clause const& lh, Clause const& rh) {
    //for(Literal l : lh) {
        //std::cout << l << " ";
    //}
    //std::cout << "0 -> ";
    //for(Literal l : rh) {
        //std::cout << l << " ";
    //}
    //std::cout << "0\n";
//}

////void CNF::propagate(std::vector<int> const& backbone) {
////#pragma omp parallel for
////    for(std::size_t i = 0; i < cnf.size(); i++) {
////        for(std::size_t j = 0; j < backbone.size(); j++) {
////            cnf[i].erase(-backbone[j]);
////        }
////    }
////
////    auto it = std::remove_if(cnf.begin(), cnf.end(), [&](auto const& c) {
////            for(int l : c) {
////                int i = std::abs(l) - 1;
////                if(l == backbone[i]) {
////                    return true;
////                }
////            }
////            return false;
////    });
////
////    cnf.erase(it, cnf.end());
////}
