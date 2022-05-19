#ifndef CAR_SOLVER_H
#define CAR_SOLVER_H

#ifdef ENABLE_PICOSAT
extern "C" {
#include "picosat/picosat.h"
}
#else
#include "minisat/core/Solver.h"
// #include "glucose/core/Solver.h"
#endif

#include <assert.h>
#include <vector>

namespace uair {
#ifdef ENABLE_PICOSAT
    class DMCSolver
#else
    class DMCSolver : public Minisat::Solver // Glucose:Solver
#endif
    {
      public:
#ifdef ENABLE_PICOSAT
        DMCSolver() { picosat_ = picosat_init(); }
        DMCSolver(bool verbose) : verbose_(verbose) { picosat_reset(picosat_); }
#else
        DMCSolver() {}
        DMCSolver(bool verbose) : verbose_(verbose) {}
#endif

        bool verbose_;

#ifdef ENABLE_PICOSAT
        std::vector<int> assumption_;
#else
        // Assumption for SAT solver
        Minisat::vec<Minisat::Lit> assumption_;
        // Assumption for SAT solver
        // Glucose::vec<Glucose::Lit> assumption_;
#endif

        // functions
        bool solve_assumption();
        std::vector<int> get_model(); // get the model from SAT solver
        std::vector<int> get_uc();    // get UC from SAT solver

        void add_cube(const std::vector<int> &);
        void add_clause_from_cube(const std::vector<int> &);
        void add_clause(int);
        void add_clause(int, int);
        void add_clause(int, int, int);
        void add_clause(int, int, int, int);
        void add_clause(std::vector<int> &);

#ifdef ENABLE_PICOSAT
        int SAT_lit(
            int id); // create the Lit used in PicoSat SAT solver for the id.
        int lit_id(int); // return the id of SAT lit
#else
        // create the Lit used in SAT solver for the id.
        Minisat::Lit SAT_lit(int id);
        // return the id of SAT lit
        int lit_id(Minisat::Lit);

// Glucose::Lit SAT_lit (int id); //create the Lit used in SAT solver for the
// id. int lit_id (Glucose::Lit);  //return the id of SAT lit
#endif

        // inline int size () {return clauses.size ();}
        inline void clear_assumption() { assumption_.clear(); }

        inline void assumption_push(int id) {
#ifdef ENABLE_PICOSAT
            assumption_.push_back(id);
#else
            assumption_.push(SAT_lit(id));
#endif
        }

        inline void assumption_pop() {
#ifdef ENABLE_PICOSAT
            assumption_.pop_back();
#else
            assumption_.pop();
#endif
        }

        // printers
        void print_clauses();
        void print_assumption();

        // l <-> r
        inline void add_equivalence(int l, int r) {
            add_clause(-l, r);
            add_clause(l, -r);
        }

        // l <-> r1 /\ r2
        inline void add_equivalence(int l, int r1, int r2) {
            add_clause(-l, r1);
            add_clause(-l, r2);
            add_clause(l, -r1, -r2);
        }

        // l<-> r1 /\ r2 /\ r3
        inline void add_equivalence(int l, int r1, int r2, int r3) {
            add_clause(-l, r1);
            add_clause(-l, r2);
            add_clause(-l, r3);
            add_clause(l, -r1, -r2, -r3);
        }
#ifdef ENABLE_PICOSAT
      private:
        PicoSAT *picosat_;
#endif
    };
} // namespace uair

#endif
