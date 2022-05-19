#include "carsolver.h"
#include "utility.h"
#include <iostream>
#include <vector>
using namespace std;

#ifndef ENABLE_PICOSAT
using namespace Minisat;
// using namespace Glucose;
#endif

namespace uair {
#ifdef ENABLE_PICOSAT
#else

    Lit DMCSolver::SAT_lit(int id) {
        assert(id != 0);
        int var = abs(id) - 1;
        while (var >= nVars())
            newVar();
        return ((id > 0) ? mkLit(var) : ~mkLit(var));
    }

    int DMCSolver::lit_id(Lit l) {
        if (sign(l))
            return -(var(l) + 1);
        else
            return var(l) + 1;
    }

    /*
    submit to SAT solver(MINISAT) to solve
    */
    bool DMCSolver::solve_assumption() {
        lbool ret = solveLimited(assumption_);
        // if (verbose_)
        // {
        // 	cout << "DMCSolver::solve_assumption: assumption_ is" << endl;
        // 	for (int i = 0; i < assumption_.size(); i++)
        // 		cout << lit_id(assumption_[i]) << ", ";
        // 	cout << endl;
        // }
        if (ret == l_True) {
            // reachable?
            return true;
        } else if (ret == l_Undef)
            exit(0);
        return false;
    }

    // return the model from SAT solver when it provides SAT
    std::vector<int> DMCSolver::get_model() {
        // adjust res'size to nVars(), value 0
        std::vector<int> res(nVars(), 0);
        // res.resize(nVars(), 0);
        for (int i = 0; i < nVars(); ++i) {
            if (model[i] == l_True)
                res[i] = i + 1;
            else if (model[i] == l_False)
                res[i] = -(i + 1);
        }
        return res;
    }

    // return the UC from SAT solver when it provides UNSAT
    // last solve is unsat, formula can't be sat, state can't reach bad or other
    // state
    std::vector<int> DMCSolver::get_uc() {
        std::vector<int> reason;
        if (verbose_)
            cout << "get uc: \n";
        for (int k = 0; k < conflict.size(); ++k) {
            Lit l = conflict[k];
            reason.push_back(-lit_id(l));
            if (verbose_)
                cout << -lit_id(l) << ", ";
        }
        if (verbose_)
            cout << endl;
        return reason;
    }

    void DMCSolver::add_clause(std::vector<int> &v) {
        vec<Lit> lits;
        for (const int &elem : v) {
            lits.push(SAT_lit(elem));
        }

        if (verbose_) {
            cout << "Adding clause " << endl << "(";
            for (int i = 0; i < lits.size(); ++i)
                cout << lit_id(lits[i]) << ", ";
            cout << ")" << endl;
            cout << "Before adding, size of clauses is " << clauses.size()
                 << endl;
        }
        bool res = addClause(lits);
        // if (!res && verbose_)
        if (!res) {
            // failed to add clause
            // print(v);
            cout << "Warning: Adding clause does not success\n";
        }
    }

#endif

    void DMCSolver::add_clause(int id) {
        std::vector<int> v;
        v.push_back(id);
        add_clause(v);
    }

    void DMCSolver::add_clause(int id1, int id2) {
        std::vector<int> v;
        v.push_back(id1);
        v.push_back(id2);
        add_clause(v);
    }

    void DMCSolver::add_clause(int id1, int id2, int id3) {
        std::vector<int> v;
        v.push_back(id1);
        v.push_back(id2);
        v.push_back(id3);
        add_clause(v);
    }

    void DMCSolver::add_clause(int id1, int id2, int id3, int id4) {
        std::vector<int> v;
        v.push_back(id1);
        v.push_back(id2);
        v.push_back(id3);
        v.push_back(id4);
        add_clause(v);
    }

    void DMCSolver::add_cube(const std::vector<int> &cu) {
        for (int i = 0; i < cu.size(); ++i)
            add_clause(cu[i]);
    }

    // block a answer
    void DMCSolver::add_clause_from_cube(const std::vector<int> &cu) {
        vector<int> v;
        for (int i = 0; i < cu.size(); ++i)
            v.push_back(-cu[i]);
        add_clause(v);
    }

    void DMCSolver::print_clauses() {
#ifndef ENABLE_PICOSAT
        cout << "clauses in SAT solver: \n";
        for (int i = 0; i < clauses.size(); ++i) {
            Clause &c = ca[clauses[i]];
            for (int j = 0; j < c.size(); ++j)
                cout << lit_id(c[j]) << " ";
            cout << "0 " << endl;
        }
#endif
    }

    void DMCSolver::print_assumption() {
        cout << "assumptions in SAT solver: \n";
        for (int i = 0; i < assumption_.size(); ++i)
            cout << lit_id(assumption_[i]) << " ";
        cout << endl;
    }

} // namespace uair
