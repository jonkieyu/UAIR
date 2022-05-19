#include "mainsolver.h"

using namespace std;

namespace uair {
    int MainSolver::max_flag_ = -1;
    vector<int> MainSolver::frame_flags_;

    MainSolver::MainSolver(Model *m, Statistics *stats, const bool verbose) {
        verbose_ = verbose;
        stats_ = stats;
        model_ = m;
        if (max_flag_ == -1)
            max_flag_ = m->max_id() + 1;
        // constraints
        for (int i = 0; i < m->outputs_start(); i++)
            add_clause(m->element(i));
        // outputs
        for (int i = m->outputs_start(); i < m->latches_start(); i++)
            add_clause(m->element(i));
        // latches
        for (int i = m->latches_start(); i < m->size(); i++)
            add_clause(m->element(i));
    }

    void MainSolver::set_assumption(const Assignment &st, const int id) {
        assumption_.clear();
        assumption_push(id);
        for (const auto elem : st) {
            assumption_push(elem);
        }
    }

    void MainSolver::set_assumption(const Assignment &st,
                                    const Assignment &id) {
        assumption_.clear();
        for (const auto &elem : st) {
            assumption_push(elem);
        }
        for (const auto &elem : id) {
            assumption_push(elem);
        }
    }

    void MainSolver::set_assumption(const Assignment &a) {
        assumption_.clear();
        // if (frame_level > -1)
        // 	assumption_push(flag_of(frame_level));
        for (const auto id : a) {
            assumption_push(id);
        }
    }

    Assignment MainSolver::get_state(const bool forward) {
        // model from SAT solver
        Assignment model = get_model();
        // isolate newState from model
        shrink_model(model, forward);
        // assertion, should be no 0!
        for (const auto &elem : model) {
            // lit that value 0 will affect addClauseFromCube() and setAssumption()
            assert(elem != 0);
        }
        assert(model.size() > 0);
        // end of assertion
        return model;
    }

    // this version is used for bad check only
    Cube MainSolver::get_conflict(const int bad) {
        Cube conflict = get_uc();
        Cube res;
        for (int i = 0; i < conflict.size(); i++) {
            if (conflict[i] != bad)
                res.push_back(conflict[i]);
        }

        std::sort(res.begin(), res.end(), uair::comp);
        return res;
    }

    Cube MainSolver::get_conflict(const bool minimal, bool &constraint) {
        Cube conflict = get_uc();

        if (minimal) {
            stats_->count_orig_uc_size(int(conflict.size()));
            stats_->count_reduce_uc_size(int(conflict.size()));
        }

        model_->shrink_to_latch_vars(conflict, constraint);

        std::sort(conflict.begin(), conflict.end(), uair::comp);

        return conflict;
    }

    // add clause to block cu
    void MainSolver::add_clause_from_cube(const Cube &cu) {
        // int flag = flag_of(frame_level);
        vector<int> clause;
        for (const auto &elem : cu) {
            if (abs(elem) > model_->max_id() / 2) {
                // for flag and primeOfBad
                clause.push_back(-elem);
            } else {
                clause.push_back(-model_->prime(elem));
            }
            // else
            // if choose this, no successor get
            // clause.push_back(-elem);
        }
        if (DEBUG) {
            // std::cout << "add clause: ";
            // print(clause);
        }
        add_clause(clause);
    }

    // model: assignment from sat solver;
    // model_ : static model from aiger, including transition relation
    // get value of inputs and latches from model
    void MainSolver::shrink_model(Assignment &model, const bool forward) {
        Assignment res;
        if (forward) {
            // input
            for (int i = 0; i < model_->num_inputs(); ++i) {
                if (i >= model.size()) {
                    // the value is DON'T CARE, so we just set to 0
                    res.push_back(0);
                } else
                    res.push_back(model[i]);
            }

            // latch
            for (int i = model_->num_inputs();
                 i < model_->num_inputs() + model_->num_latches();
                 ++i) { // the value is DON'T CARE
                if (i >= model.size())
                    break;
                res.push_back(model[i]);
            }
        } else {
            // input
            // for (int i = 0; i < model_->num_inputs(); ++i)
            // {
            // 	if (i >= model.size())
            // 	{
            // 		// the value is DON'T CARE, so we just set to 0
            // 		res.push_back(0);
            // 	}
            // 	else
            // 		res.push_back(model[i]);
            // }
            Assignment input;
            for (int i = 0; i < model_->num_inputs(); ++i) {
                if (i >= model.size()) {
                    continue;
                } else {
                    // TODO, refactor to use input_prime_ rather next_map_
                    int p = model_->prime(i + 1);
                    if (p >= model.size() || p == 0) {
                        input.push_back(-i - 1);
                        continue;
                    }
                    assert(model.size() > abs(p));
                    int val = model[abs(p) - 1];
                    if (p == val)
                        input.push_back(i + 1);
                    else
                        input.push_back(-i - 1);
                }
            }
            for (const auto &elem : input)
                res.push_back(elem);

            // latch
            Assignment tmp(model_->num_latches(), 0);
            for (int i = model_->num_inputs() + 1;
                 i <= model_->num_inputs() + model_->num_latches(); ++i) {
                int p = model_->prime(i);
                assert(p != 0);
                assert(model.size() > abs(p));

                int val = model[abs(p) - 1];
                if (p == val) {
                    // positive value
                    tmp[i - model_->num_inputs() - 1] = i;
                } else {
                    // negative value
                    tmp[i - model_->num_inputs() - 1] = -i;
                }
            }
            for (const auto &elem : tmp) {
                res.push_back(elem);
            }
        }
        model = res;
    }
} // namespace uair