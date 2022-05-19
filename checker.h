#ifndef CHECKER_H
#define CHECKER_H

#include "data_structure.h"
#include "mainsolver.h"
#include "model.h"
#include "statistics.h"
#include "utility.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <list>
#include <set>
#include <stack>

#define MAX_SOLVER_CALL 500
#define MAX_TRY 4
// set to false when run on all cases
#define DEBUG true

namespace uair {
    class Comparator {
      public:
        // Comparator (std::vector<int>& counter): counter_ (counter) {}
        Comparator(Model *model) : model_(model) {}

        bool operator()(int i, int j) {
            // int id1 = i > 0 ? 2*i : 2*(-i)+1, id2 = j > 0 ? 2*j : 2*(-j)+1;
            // return counter_[id1] < counter_[id2];
            int id1 = model_->prime(i), id2 = model_->prime(j);
            return abs(id1) < abs(id2);
        }

      private:
        // std::vector<int> counter_;
        Model *model_;
    };

    class Checker {
      public:
        /*
        including model, some settings, some pointer of output files, etc.
        */
        Checker(Model *model, Statistics &stats, std::ofstream *dot,
                bool evidence = false, bool verbose = false,
                bool minimal_uc = false);
        ~Checker();

        // entry of checker
        bool check(std::ofstream &);
        void printEvidence(std::ofstream &);
        inline void print_frames_sizes() {}

      protected:
        // flags
        bool _partial_state;
        bool _minimal_uc;
        bool _evidence;
        bool _verbose;

        // members
        Statistics *_stats;

        // times of iteration
        long _maxIteration;
        const bool UNSAFE = true;
        const bool SAFE = false;
        int _flag;

        std::ofstream *_dot;         // for dot file
        int _solver_call_counter;    // counter for solver_ calls
        int _trySolver_call_counter; // counter for solver_ calls

        int _minimal_update_level;
        // the start state
        State *_init;
        // the start state for backward CAR
        State *_last;
        int _bad;

        // store ctx
        std::stack<std::string> _traceStack;
        // store verified invariant, store real invariant C0,...,Cn
        std::vector<std::vector<Cube>> _bigCList;
        // store state n
        std::vector<Assignment> _blockStateNList;

        Model *_model;
        MainSolver *_solver;
        MainSolver *_trySolver;

        // functions
        bool immediate_satisfiable();
        bool immediate_satisfiable(const State *);
        bool immediate_satisfiable(const Cube &);
        State *get_new_state(const State *s, const bool forward,
                             MainSolver *currentMainSolver);
        State *get_new_state2(const State *s, const bool forward);
        bool solve_with(const Cube &s, MainSolver *currentMainSolver);
        std::pair<Assignment, Assignment> state_pair(const Assignment &st);

        void uair_initialization();
        void uair_finalization();
        void destroy_states();
        /**
         * @brief main part of UAIR model checking
         *
         * @return true unsafe
         * @return false otherwise
         */
        bool uair_check();
        bool check(const int &bad, std::vector<Cube> &inv, const int &depth);
        bool check(const Assignment &t, std::vector<Cube> &inv,
                   const int &depth);
        bool trySolve(const State *s, const int &primeOfBad,
                      std::vector<Cube> &bigC, const int flag,
                      MainSolver *currentMainSolver);
        bool trySolve(const State *s, Assignment &primeOfT,
                      std::vector<Cube> &bigC, const int flag,
                      MainSolver *currentMainSolver);
        void tseitinTransform(const std::vector<Cube> &disjunctionOfCubes,
                              std::vector<Clause> &conjunctionOfClause);
        Assignment getPartial(const State *s, Assignment t);
        Assignment getPartial4(const State *s, const Assignment &input,
                               const Assignment &input2, Assignment &t);
        Assignment getPartial2(const State *s, std::vector<Cube> bigC);
        Assignment getPartial3(const State *s, std::vector<Cube> bigC);
        Assignment getPartial5(const State *s, const State *badState,
                               const Assignment &inputs, const Assignment &t,
                               MainSolver *currentSolver);

        Assignment getPrimeOfCube(const Cube &latchList);
        int getPrimeOfBad(int bad);
        Cube getElementOfBigC(std::vector<Cube> &bigC);
        bool isInitContainedInS(Cube pm, Cube init);
        void blockInvForever(const std::vector<Cube> &bigC, MainSolver *solver);
        void blockInvForever(const std::vector<Cube> &bigC, MainSolver *solver,
                             std::vector<std::vector<Cube>> &blockInvList);
        void addBigCToSolver(const std::vector<Cube> &bigC, MainSolver *solver);
        void addBigCToSolver(const std::vector<Cube> &bigC, MainSolver *solver,
                             std::vector<std::vector<Cube>> &bigCList);

        // inline functions

        // inline bool reconstruct_start_solver_required()
        // {
        // 	start_solver_call_counter_++;
        // 	if (start_solver_call_counter_ == MAX_SOLVER_CALL)
        // 	{
        // 		start_solver_call_counter_ = 0;
        // 		return true;
        // 	}
        // 	return false;
        // }

        // inline void reconstruct_start_solver()
        // {
        // 	delete start_solver_;
        // 	start_solver_ = new StartSolver(model_, bad_, verbose_);
        // }

        inline bool reconstruct_solver_required() {
            _solver_call_counter++;
            if (_solver_call_counter == MAX_SOLVER_CALL) {
                _solver_call_counter = 0;
                return true;
            }
            return false;
        }

        inline void reconstruct_solver() {
            delete _solver;
            MainSolver::clear_frame_flags();
            _solver = new MainSolver(_model, _stats, _verbose);
        }

        // st is value of state, p can be bad
        inline bool solver_solve_with_assumption(const Assignment &st,
                                                 const int p) {
            // if (reconstruct_solver_required())
            // reconstruct_solver();
            _stats->count_main_solver_SAT_time_start();
            // main solver solve
            bool res = _solver->solve_with_assumption(st, p);
            _stats->count_main_solver_SAT_time_end();
            return res;
        }

        /**
         * @brief used for trySolve
         *
         * @param st assumption
         * @param p assumption
         * @return true SAT
         * @return false UNSAT
         */
        inline bool trySolver_solve_with_assumption(const Assignment &st,
                                                    const int p) {
            // if (reconstruct_solver_required())
            // reconstruct_solver();
            _stats->count_try_solver_SAT_time_start();
            // main solver solve
            bool res = _trySolver->solve_with_assumption(st, p);
            _stats->count_try_solver_SAT_time_end();
            return res;
        }

        inline bool
        solver_solve_with_assumption(const Assignment &st, const Assignment &t,
                                     MainSolver *currentMainSolver) {
            // if (reconstruct_solver_required())
            // reconstruct_solver();
            _stats->count_main_solver_SAT_time_start();
            // main solver solve
            bool res = currentMainSolver->solve_with_assumption(st, t);
            _stats->count_main_solver_SAT_time_end();
            return res;
        }

        inline bool trySolver_solve_with_assumption(const Assignment &st,
                                                    const Assignment &t) {
            // if (reconstruct_solver_required())
            // reconstruct_solver();
            _stats->count_try_solver_SAT_time_start();
            // main solver solve
            bool res = _trySolver->solve_with_assumption(st, t);
            _stats->count_try_solver_SAT_time_end();
            return res;
        }

        inline bool
        solver_solve_with_assumption(const Assignment &st,
                                     MainSolver *currentMainSolver) {
            Assignment st2 = st;
            currentMainSolver->set_assumption(st2);
            _stats->count_main_solver_SAT_time_start();
            bool res = currentMainSolver->solve_with_assumption();
            _stats->count_main_solver_SAT_time_end();
            return res;
        }

        inline bool trySolver_solve_with_assumption(const Assignment &st) {
            Assignment st2 = st;
            _trySolver->set_assumption(st2);
            _stats->count_try_solver_SAT_time_start();
            bool res = _trySolver->solve_with_assumption();
            _stats->count_try_solver_SAT_time_end();
            return res;
        }
    };
} // namespace uair
#endif