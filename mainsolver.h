#ifndef MAIN_SOLVER_H
#define MAIN_SOLVER_H

#include "carsolver.h"
#include "data_structure.h"
#include "model.h"
#include "statistics.h"
#include <cassert>
#include <iostream>
#include <vector>

#include "utility.h"
#include <algorithm>

#define DEBUG false

namespace uair {

    /**
     * used for computing state and reachability
     * */
    class MainSolver : public DMCSolver {
      public:
        MainSolver(Model *, Statistics *stats, const bool verbose = false);
        ~MainSolver() {}

        // public funcitons
        void set_assumption(const Assignment &);
        void set_assumption(const Assignment &, const int);
        void set_assumption(const Assignment &, const Assignment &);

        inline bool solve_with_assumption(const Assignment &st, const int p) {
            set_assumption(st, p);
            if (verbose_)
                std::cout << "MainSolver::";
            return solve_assumption();
        }

        inline bool solve_with_assumption(const Assignment &st,
                                          const Assignment &t) {
            set_assumption(st, t);
            if (verbose_)
                std::cout << "MainSolver::";
            return solve_assumption();
        }

        inline bool solve_with_assumption() {
            if (verbose_)
                std::cout << "MainSolver::";
            return solve_assumption();
        }

        Assignment get_state(const bool forward = true);

        // this version is used for bad check only
        Cube get_conflict(const int bad);
        Cube get_conflict(const bool minimal, bool &constraint);

        // overload
        void add_clause_from_cube(const Cube &cu);

        inline void update_constraint(Cube &cu) {
            DMCSolver::add_clause_from_cube(cu);
        }

        inline static void clear_frame_flags() { frame_flags_.clear(); }

      private:
        // members
        static int max_flag_;
        static std::vector<int> frame_flags_;

        Model *model_;

        Statistics *stats_;

        // bool verbose_;

        // functions

        void shrink_model(Assignment &model, const bool forward);
    };

} // namespace uair

#endif
