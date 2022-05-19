#include "checker.h"
#include "statistics.h"
#include "utility.h"
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

/**
 * @brief UAIR algorithm
 *
 */
namespace uair {
    ////////////////////main functions////////////////////
    bool Checker::check(std::ofstream &out) {
        // each output is bad, bad is a int value
        for (int i = 0; i < _model->num_outputs(); i++) {
            _bad = _model->output(i);
            // for the particular case when bad_ is true or false
            if (_bad == _model->true_id()) {
                // bad is true
                out << "1" << std::endl;
                out << "b" << i << std::endl;
                if (_evidence) {
                    // print init state
                    out << _init->latches() << std::endl;
                    // print an arbitary input vector
                    for (int j = 0; j < _model->num_inputs(); j++)
                        out << "0";
                    out << std::endl;
                }
                out << "." << std::endl;
                if (_verbose) {
                    std::cout << "return SAT since the output is true"
                              << std::endl;
                }
                return UNSAFE;
            } else if (_bad == _model->false_id()) {
                // bad is always false
                out << "0" << std::endl;
                out << "b" << std::endl;
                out << "." << std::endl;
                if (_verbose) {
                    std::cout << "return UNSAT since the output is false"
                              << std::endl;
                }
                return false;
            }
            uair_initialization();
            // checking
            bool res = uair_check();
            if (res) {
                // unsafe
                out << "1" << std::endl;
            } else {
                out << "0" << std::endl;
            }
            out << "b" << i << std::endl;
            // unsafe
            if (_evidence && res)
                printEvidence(out);
            out << "." << std::endl;
            uair_finalization();
            return res;
        }
    }

    /**
     * @brief checking model
     *
     * @return true: model unsafe
     * @return false: safe
     */
    bool Checker::uair_check() {
        if (DEBUG)
            std::cout << "-----start check-----" << std::endl;
        // check _init
        if (immediate_satisfiable()) {
            // unsafe
            if (_verbose)
                std::cout << "return SAT for immediate_satisfiable"
                          << std::endl;
            if (_evidence) {
                _traceStack.push(_init->inputs());
                _traceStack.push(_init->latches());
            }
            return UNSAFE;
        }
        _init->set_depth(0);
        _init->set_initial(true);
        std::vector<Cube> inv;
        const int INIT_DEPTH = 0;
        if (check(_bad, inv, INIT_DEPTH)) {
            // input of _ini is empty
            if (_evidence) {
                _traceStack.push(_init->latches());
            }
            return UNSAFE;
        } else {
            return SAFE;
        }
    }

    /*
    check with bad
    */
    bool Checker::check(const int &bad, std::vector<Cube> &inv,
                        const int &depth) {
        // set of uc(the reason that a state cannot reach bad)
        std::vector<Cube> bigC;
        int primeOfBad = getPrimeOfBad(bad);
        // control ~C'
        int flagNegCPrime = _flag++;
        if (DEBUG) {
            std::cout << "\n-----start trySolve_with_bad-----" << std::endl;
            // all s is initState
            std::cout << "t': " << primeOfBad << std::endl;
        }
        if (trySolve(_init, primeOfBad, bigC, flagNegCPrime, _solver)) {
            return UNSAFE;
        } else {
            if (DEBUG && !bigC.empty()) {
                std::cout << "C:" << std::endl;
                for (const auto &elem : bigC)
                    print(elem);
            }
            // potential invariant C got
            if (DEBUG) {
                std::cout << "\n-----finish trySolve_with_bad, go back to "
                             "check_with_bad, depth: "
                          << depth << "-----" << std::endl;
                std::cout
                    << "\nstart SAT(C/\\T/\\~C') from check_with_bad, depth: "
                    << depth << std::endl;
            }
            long bigCAddedIndex = 0;
            long stateNBlockedIndex = 0;
            // SAT(C/\T/\~C')
            for (int i = 0; i < bigC.size(); ++i) {
                bool enterWhileLoop = false;
                Cube assumption = bigC[i];
                // ~C' must controlled by flag，otherwise affect SAT(n/\T/\bad')
                assumption.push_back(flagNegCPrime);
                while (solver_solve_with_assumption(assumption, _solver)) {
                    if (!enterWhileLoop) {
                        enterWhileLoop = true;
                    }
                    if (DEBUG) {
                        std::cout
                            << "\nin while(C/\\T/\\~C') loop of check_with_bad"
                            << std::endl;
                    }
                    // n in ~C
                    State *n = get_new_state(nullptr, false, _solver);
                    // start SAT(n/\T/\bad')
                    if (solver_solve_with_assumption(n->s(), primeOfBad)) {
                        if (DEBUG) {
                            std::cout << "n: ";
                            print(n->s());
                        }
                        // Assignment partialN = getPartial3(n, bigC);
                        Assignment badCube;
                        badCube.push_back(primeOfBad);
                        Assignment inputs =
                            get_new_state(nullptr, true, _solver)
                                ->getInputVec();
                        std::string inputsStr =
                            get_new_state(nullptr, true, _solver)->inputs();
                        // prime of input
                        std::string inputsStr2 =
                            get_new_state(nullptr, false, _solver)->inputs();
                        State *badState =
                            get_new_state(nullptr, false, _solver);
                        Assignment partialN =
                            getPartial5(n, badState, inputs, badCube, _solver);
                        delete badState;
                        // enter check_with_state
                        // block n
                        _solver->add_clause_from_cube(partialN);
                        _blockStateNList.push_back(partialN);
                        ++stateNBlockedIndex;
                        std::vector<Cube> inv2;
                        const int DEPTH_ONE = 1;
                        if (_evidence) {
                            _traceStack.push(inputsStr2);
                            _traceStack.push(inputsStr);
                        }
                        if (check(partialN, inv2, DEPTH_ONE)) {
                            if (DEBUG) {
                                std::cout << "size of C: " << inv2.size()
                                          << ", in check_with_bad, depth: "
                                          << depth + 1 << std::endl;
                            }
                            delete n;
                            return UNSAFE;
                        } else {
                            // None of the states in inv2 can be transferred to
                            // states outside of inv2, nor can they be
                            // transferred to partialN
                            if (_evidence) {
                                _traceStack.pop();
                                _traceStack.pop();
                            }
                            if (DEBUG) {
                                std::cout << "size of C1: " << inv2.size()
                                          << ", in check_with_bad, depth: 1"
                                          << std::endl;
                                std::cout << "finish check_with_state, back "
                                             "to check_with_bad"
                                          << std::endl;
                            }
                            // add bigC
                            int bigCListSize = _bigCList.size();
                            while (bigCAddedIndex < bigCListSize) {
                                if (DEBUG) {
                                    std::cout << "add bigC to solver, in "
                                                 "check_with_bad"
                                              << std::endl;
                                }
                                addBigCToSolver(_bigCList[bigCAddedIndex],
                                                _solver);
                                ++bigCAddedIndex;
                            }
                            // block stateN
                            int blockNListSize = _blockStateNList.size();
                            while (stateNBlockedIndex < blockNListSize) {
                                _solver->add_clause_from_cube(
                                    _blockStateNList[stateNBlockedIndex]);
                                ++stateNBlockedIndex;
                            }
                        }
                    } else {
                        // UNSAT(n/\T/\bad')
                        if (DEBUG) {
                            std::cout << "UNSAT(n, t') from check_with_bad"
                                      << std::endl;
                        }
                        // add uc to C
                        Cube uc = _solver->get_uc();
                        // filter uc
                        for (int i = 0; i < uc.size(); ++i) {
                            if (abs(uc[i]) == abs(primeOfBad) ||
                                ((abs(uc[i]) < _model->num_inputs() + 1) ||
                                 (abs(uc[i]) > _model->num_latches() +
                                                   _model->num_inputs()))) {
                                uc.erase(uc.begin() + i);
                                --i;
                            }
                        }
                        if (!uc.empty()) {
                            bigC.push_back(uc);
                            uc.push_back(flagNegCPrime);
                            _solver->add_clause_from_cube(uc);
                        } else {
                            // avoid dead loop
                            if (DEBUG) {
                                std::cout << "get empty uc from check_with_bad"
                                          << std::endl;
                            }
                            delete n;
                            inv = bigC;
                            return SAFE;
                            // break;
                        }
                    }
                    delete n;
                }
                if (!enterWhileLoop) {
                    if (DEBUG) {
                        std::cout << "while(SAT(c/\\T/\\~C')) = false, in "
                                     "check_with_bad"
                                  << std::endl;
                    }
                } else {
                    if (DEBUG) {
                        std::cout << "while(SAT(c/\\T/\\~C')) finished, in "
                                     "check_with_bad"
                                  << std::endl;
                    }
                }
            }
            if (DEBUG) {
                std::cout << "finish SAT(C/\\T/\\~C')" << std::endl;
            }
            inv = bigC;
            return false;
        }
    }

    /*
    recursive function, check with state t
    t can reach bad in >= 1 steps
    if return true, all states in inv cannot reach bad
    */
    bool Checker::check(const Assignment &t, std::vector<Cube> &inv,
                        const int &depth) {
        if (DEBUG) {
            std::cout << "\n-----start check_with_state(depth: " << depth
                      << ")-----" << std::endl;
        }
        // set of uc
        std::vector<Cube> bigC;
        // t'
        Assignment primeOfT = getPrimeOfCube(t);
        if (DEBUG) {
            std::cout << "t: ";
            print(t);
        }
        int flagNegCPrime = _flag++;
        MainSolver *currentMainSolver =
            new MainSolver(_model, _stats, _verbose);
        long bigCAddedIndex = 0;
        long stateNBlockedIndex = 0;
        // block n
        for (const auto &stateN : _blockStateNList) {
            if (stateN != t)
                currentMainSolver->add_clause_from_cube(stateN);
            ++stateNBlockedIndex;
        }
        for (const auto &tempBigC : _bigCList) {
            addBigCToSolver(tempBigC, currentMainSolver);
            ++bigCAddedIndex;
        }
        if (DEBUG) {
            std::cout << "\n-----start trySolve_with_state-----" << std::endl;
            // s is always initState
            std::cout << "t': ";
            print(primeOfT);
        }
        if (trySolve(_init, primeOfT, bigC, flagNegCPrime, currentMainSolver)) {
            delete currentMainSolver;
            return UNSAFE;
        } else {
            // print info of bigC
            if (DEBUG && !bigC.empty()) {
                std::cout << "C:" << std::endl;
                for (const auto &elem : bigC)
                    print(elem);
            }
            if (DEBUG) {
                std::cout << "-----finish trySolve_with_state-----"
                          << std::endl;
                std::cout
                    << "\nstart SAT(C/\\T/\\~C') from check_with_state, depth: "
                    << depth << std::endl;
            }
            // SAT(Cdepth/\T/\~Cdepth')
            for (int i = 0; i < bigC.size(); ++i) {
                bool enterWhileLoop = false;
                Cube assumption = bigC[i];
                assumption.push_back(flagNegCPrime);
                while (solver_solve_with_assumption(assumption,
                                                    currentMainSolver)) {
                    if (!enterWhileLoop) {
                        enterWhileLoop = true;
                    }
                    // n in ~C
                    State *n = get_new_state(nullptr, false, currentMainSolver);
                    if (DEBUG) {
                        std::cout << "\nin while(C/\\T/\\~C') loop of "
                                     "check_with_state, "
                                     "depth: "
                                  << depth << std::endl;
                        std::cout << "n: ";
                        print(n->s());
                    }
                    // SAT(n, t')
                    if (solver_solve_with_assumption(n->s(), primeOfT,
                                                     currentMainSolver)) {
                        if (DEBUG) {
                            std::cout
                                << "SAT(n/\\t') = true, in check_with_state"
                                << std::endl;
                        }
                        // SAT(n, t') = true
                        Assignment inputs =
                            get_new_state(nullptr, true, currentMainSolver)
                                ->getInputVec();
                        std::string inputsStr =
                            get_new_state(nullptr, true, currentMainSolver)
                                ->inputs();
                        // Assignment inputs2;
                        // for (const int &elem : inputs) {
                        //     if (_model->prime(elem) != 0) {
                        //         inputs2.push_back(
                        //             currentMainSolver->get_model()
                        //                 [abs(_model->prime(elem)) - 1]);
                        //     }
                        // }
                        // Assignment partialN = getPartial4(n, inputs,
                        // inputs2, primeOfT);
                        State *badState =
                            get_new_state(nullptr, false, currentMainSolver);
                        Assignment partialN = getPartial5(
                            n, badState, inputs, primeOfT, currentMainSolver);
                        delete badState;
                        if (_evidence) {
                            _traceStack.push(inputsStr);
                        }
                        currentMainSolver->add_clause_from_cube(partialN);
                        _blockStateNList.push_back(partialN);
                        ++stateNBlockedIndex;
                        // store Cdepth+1
                        std::vector<Cube> inv2;
                        if (check(partialN, inv2, depth + 1)) {
                            if (DEBUG) {
                                std::cout << "size of C: " << inv2.size()
                                          << ", in check_with_state, depth: "
                                          << depth + 1 << std::endl;
                            }
                            delete n;
                            delete currentMainSolver;
                            return UNSAFE;
                        } else {
                            // None of the states in inv2 can be transferred to
                            // states outside of inv2, nor can they be
                            // transferred to partialN
                            if (DEBUG) {
                                std::cout << "size of C: " << inv2.size()
                                          << ", in check_with_state, depth: "
                                          << depth + 1 << std::endl;
                            }
                            if (_evidence) {
                                _traceStack.pop();
                            }
                            // add bigC
                            int bigCListSize = _bigCList.size();
                            while (bigCAddedIndex < bigCListSize) {
                                if (DEBUG) {
                                    std::cout << "add bigC to solver, in "
                                                 "check_with_bad"
                                              << std::endl;
                                }
                                addBigCToSolver(_bigCList[bigCAddedIndex],
                                                currentMainSolver);
                                ++bigCAddedIndex;
                            }
                            // block stateN
                            int blockNListSize = _blockStateNList.size();
                            while (stateNBlockedIndex < blockNListSize) {
                                Assignment stateN =
                                    _blockStateNList[stateNBlockedIndex];
                                if (stateN != t) {
                                    currentMainSolver->add_clause_from_cube(
                                        stateN);
                                }
                                ++stateNBlockedIndex;
                            }
                        }
                    } else {
                        // SAT(n, t') = false
                        if (DEBUG) {
                            std::cout
                                << "SAT(n, t') is false, from check_with_state"
                                << std::endl;
                        }
                        // add uc to C
                        auto uc = currentMainSolver->get_uc();
                        // filter uc
                        for (int i = 0; i < uc.size(); ++i) {
                            if ((abs(uc[i]) < _model->num_inputs() + 1) ||
                                (abs(uc[i]) > _model->num_latches() +
                                                  _model->num_inputs())) {
                                uc.erase(uc.begin() + i);
                                --i;
                            }
                        }
                        if (!uc.empty()) {
                            // assert(!uc.empty());
                            bigC.push_back(uc);
                            uc.push_back(flagNegCPrime);
                            currentMainSolver->add_clause_from_cube(uc);
                        } else {
                            if (DEBUG) {
                                std::cout << "return false, from check with "
                                             "state, break branch"
                                          << std::endl;
                            }
                            delete n;
                            inv = bigC;
                            delete currentMainSolver;
                            _bigCList.push_back(bigC);
                            return false;
                            // break;
                        }
                    }
                    delete n;
                }
            }
            inv = bigC;
            if (DEBUG) {
                std::cout << "return false, from check_with_state" << std::endl;
            }
            delete currentMainSolver;
            // add Cdepth to bigCList
            _bigCList.push_back(bigC);
            return false;
        }
    }

    /*
    try with bad
    */
    bool Checker::trySolve(const State *s, const int &primeOfBad,
                           std::vector<Cube> &bigC, const int flag,
                           MainSolver *currentMainSolver) {
        Assignment stateS = s->getS();
        if (trySolver_solve_with_assumption(stateS, primeOfBad)) {
            // SAT(s/\T/\bad') is true
            // unsafe
            if (_evidence) {
                std::string input = get_new_state2(s, true)->inputs();
                std::string input2 = get_new_state2(s, false)->inputs();
                _traceStack.push(input2);
                _traceStack.push(input);
            }
            return UNSAFE;
        } else {
            // SAT(s/\T/\bad') is false
            Cube uc = _trySolver->get_uc();
            // filter, remove primeOfBad from uc
            for (int i = 0; i < uc.size(); ++i) {
                if (abs(uc[i]) == abs(primeOfBad)) {
                    uc.erase(uc.begin() + i);
                    break;
                }
            }
            // C = C + c
            bigC.push_back(uc);
            // (~C)'
            uc.push_back(flag);
            // used for ~C' in check_with_bad
            currentMainSolver->add_clause_from_cube(uc);
            _trySolver->add_clause_from_cube(uc);
            // SAT(s/\T/\~C')
            while (trySolver_solve_with_assumption(stateS, flag)) {
                // get m from ~C
                State *m = get_new_state2(s, false);
                std::string input = get_new_state2(s, true)->inputs();
                // newAdd，invalid
                // Assignment partialM = getPartial3(m, bigC);
                // m->set_latches(partialM);
                // end of newAdd
                if (trySolve(m, primeOfBad, bigC, flag, currentMainSolver)) {
                    if (_evidence) {
                        // if (s->inputs().length() > 0)
                        // 	_traceStack.push(s->inputs());
                        _traceStack.push(input);
                    }
                    delete m;
                    return UNSAFE;
                }
                delete m;
            }
            return false;
        }
    }

    /*
    Recursive process, return after the end
    try with state
    */
    bool Checker::trySolve(const State *s, Assignment &primeOfT,
                           std::vector<Cube> &bigC, const int flag,
                           MainSolver *currentMainSolver) {
        Assignment stateS = s->getS();
        if (trySolver_solve_with_assumption(stateS, primeOfT)) {
            // unsafe
            if (_evidence) {
                std::string input = get_new_state2(s, true)->inputs();
                _traceStack.push(input);
            }
            return UNSAFE;
        } else {
            // SAT(s/\T/\t') is false
            Cube uc = _trySolver->get_uc();
            // remove those not in latch from uc
            // filter element of primeOfT those not in latch list
            for (int i = 0; i < uc.size(); ++i) {
                if ((abs(uc[i]) < _model->num_inputs() + 1) ||
                    (abs(uc[i]) >
                     _model->num_latches() + _model->num_inputs())) {
                    uc.erase(uc.begin() + i);
                    --i;
                }
            }
            assert(!uc.empty());
            bigC.push_back(uc);
            uc.push_back(flag);
            // used for ~C' in check()
            currentMainSolver->add_clause_from_cube(uc);
            _trySolver->add_clause_from_cube(uc);
            // SAT(s/\T/\~C')
            while (trySolver_solve_with_assumption(stateS, flag)) {
                // true
                // get from ~C
                State *m = get_new_state2(s, false);
                std::string input = get_new_state2(s, true)->inputs();
                // newAdd, try to get partialM, invalid
                // Assignment partialM = getPartial3(m, bigC);
                // m->set_latches(partialM);
                // end of newAdd
                if (trySolve(m, primeOfT, bigC, flag, currentMainSolver)) {
                    if (_evidence) {
                        _traceStack.push(input);
                    }
                    delete m;
                    return UNSAFE;
                }
                delete m;
            }
            return false;
        }
    }

    //////////////helper functions/////////////////////////////////////////////

    Checker::Checker(Model *model, Statistics &stats, std::ofstream *dot,
                     bool evidence, bool verbose, bool minimal_uc) {

        _model = model;
        _stats = &stats;
        _dot = dot;
        _solver = nullptr;
        _trySolver = nullptr;
        _init = new State(_model->init());
        _last = nullptr;
        _minimal_uc = minimal_uc;
        _evidence = evidence;
        _verbose = verbose;
        _solver_call_counter = 0;
        _trySolver_call_counter = 0;
        _flag = _model->max_id() + 10;
    }

    Checker::~Checker() {
        if (_init != nullptr) {
            delete _init;
            _init = nullptr;
        }
        if (_last != nullptr) {
            delete _last;
            _last = nullptr;
        }
        uair_finalization();
    }

    void Checker::uair_initialization() {
        _solver = new MainSolver(_model, _stats, _verbose);
        _trySolver = new MainSolver(_model, _stats, _verbose);
    }

    void Checker::uair_finalization() {
        destroy_states();
        if (_solver != nullptr) {
            delete _solver;
            _solver = nullptr;
        }
        if (_trySolver != nullptr) {
            delete _trySolver;
            _trySolver = nullptr;
        }
    }

    void Checker::destroy_states() {}

    /*
    get partialS by t
    */
    Assignment Checker::getPartial(const State *s, Assignment t) {
        return s->getS();
        Assignment assumption = s->getS();
        Assignment input = s->getInputVec();
        assumption.insert(assumption.end(), input.begin(), input.end());
        int flag = _flag++;
        t.push_back(flag);
        _solver->add_clause_from_cube(t);
        bool res = solver_solve_with_assumption(assumption, flag);
        assert(!res);
        Cube uc = _solver->get_uc();
        // filter uc
        for (int i = 0; i < uc.size(); ++i) {
            if ((abs(uc[i]) < _model->num_inputs() + 1) ||
                (abs(uc[i]) > _model->num_latches() + _model->num_inputs())) {
                uc.erase(uc.begin() + i);
                --i;
            }
        }
        assert(!uc.empty());
        sort(uc.begin(), uc.end(),
             [](int x, int y) { return abs(x) < abs(y); });
        return uc;
    }

    /*
    partial with C, probably better than getPartial()
    use C rather than flag(stand for ~C')
    wrong partial may result in fake unsafe
    */
    Assignment Checker::getPartial2(const State *s, std::vector<Cube> bigC) {
        // return s->getS();
        Assignment assumption = s->getS();
        Assignment input = s->getInputVec();
        assumption.insert(assumption.end(), input.begin(), input.end());
        int flag = _flag++;
        std::vector<Clause> newBigC;
        // get disjunction form of bigC
        tseitinTransform(bigC, newBigC);
        for (auto &c : newBigC) {
            c.push_back(flag);
            _solver->update_constraint(c);
        }
        // assumption = input + latch + flag
        bool res = solver_solve_with_assumption(assumption, flag);
        // !res may be true
        assert(!res);
        Cube uc = _solver->get_uc();
        // filter uc
        for (int i = 0; i < uc.size(); ++i) {
            if ((abs(uc[i]) < _model->num_inputs() + 1) ||
                (abs(uc[i]) > _model->num_latches() + _model->num_inputs())) {
                uc.erase(uc.begin() + i);
                --i;
            }
        }
        assert(!uc.empty());
        sort(uc.begin(), uc.end(),
             [](int x, int y) { return abs(x) < abs(y); });
        return uc;
    }

    /*
    partial with C, probably better than getPartial()
    n, C -> partialN
    m, C -> partialM
    */
    Assignment Checker::getPartial3(const State *s, std::vector<Cube> bigC) {
        Assignment assumption = s->getS();
        Assignment input = s->getInputVec();
        // Sinput /\ Slatch
        assumption.insert(assumption.end(), input.begin(), input.end());
        int flag = _flag++;
        std::vector<Clause> newBigC;
        // get disjunction form of bigC
        tseitinTransform(bigC, newBigC);
        for (auto &c : newBigC) {
            c.push_back(-flag);
            _solver->add_clause(c);
        }
        // assumption = input + latch + flag
        // !res may be true
        bool res = solver_solve_with_assumption(assumption, flag);
        assert(!res);
        Cube uc = _solver->get_uc();
        // filter uc
        for (int i = 0; i < uc.size(); ++i) {
            if ((abs(uc[i]) < _model->num_inputs() + 1) ||
                (abs(uc[i]) > _model->num_latches() + _model->num_inputs())) {
                uc.erase(uc.begin() + i);
                --i;
            }
        }
        assert(!uc.empty());
        sort(uc.begin(), uc.end(),
             [](int x, int y) { return abs(x) < abs(y); });
        return uc;
    }

    /*
    partial with t
    */
    Assignment Checker::getPartial4(const State *s, const Assignment &inputs,
                                    const Assignment &inputs2, Assignment &t) {
        Assignment assumption = s->getS();
        // Assignment input = s->getInputVec();
        assumption.insert(assumption.end(), inputs.begin(), inputs.end());
        assumption.insert(assumption.end(), inputs2.begin(), inputs2.end());
        // bool res11 = solver_solve_with_assumption(assumption, t);
        // if (DEBUG)
        // {
        // 	std::cout << "prime of bad:" << std::endl;
        // 	print(t);
        // 	// _solver->print_clauses();
        // 	std::cout << "model 1:" << std::endl;
        // 	print(_solver->get_model());
        // 	// std::cout << "inputs: " << get_new_state(nullptr,
        // false)->inputs() << std::endl;
        // 	// print(get_new_state(nullptr, false)->getS());
        // }
        // assert(res11);
        // bool res22 = solver_solve_with_assumption(assumption, -t[0]);
        // assert(!res22);
        if (DEBUG) {
            // assert(res22);
            // std::cout << "model 2:" << std::endl;
            // print(_solver->get_model());
            // std::cout << "inputs: " << get_new_state(nullptr,
            // false)->inputs() << std::endl; print(get_new_state(nullptr,
            // false)->getS());
        }
        // for (auto &elem : t)
        // 	elem = -elem;
        int flag = _flag++;
        t.push_back(flag);
        _solver->update_constraint(t);
        bool res = solver_solve_with_assumption(assumption, flag);
        if (DEBUG) {
            std::cout << "assumption(latch):" << std::endl;
            print(s->getS());
            std::cout << "assumption(input+latch):" << std::endl;
            print(assumption);
        }
        assert(!res);
        Cube uc = _solver->get_uc();
        // filter uc
        for (int i = 0; i < uc.size(); ++i) {
            if ((abs(uc[i]) < _model->num_inputs() + 1) ||
                (abs(uc[i]) > _model->num_latches() + _model->num_inputs())) {
                uc.erase(uc.begin() + i);
                --i;
            }
        }
        assert(!uc.empty());
        sort(uc.begin(), uc.end(),
             [](int x, int y) { return abs(x) < abs(y); });
        return uc;
    }

    /**
     * @brief Checker::getPartial
     * get ucBad|input, s/\T/\~ucBad|input UNSAT,
     * @param s
     * @param badState a full-state of bad
     * @param inputs inputs/\s/\T/\badState' SAT
     * @param t
     * @return Assignment
     */
    Assignment Checker::getPartial5(const State *s, const State *badState,
                                    const Assignment &inputs,
                                    const Assignment &t,
                                    MainSolver *currentSolver) {
        // return s->getS();

        // s/\T/\bad'|input SAT, s/\T/\~bad'|input UNSAT
        Assignment badS = badState->getS();
        int flag = _flag++;
        badS.push_back(flag);
        currentSolver->add_clause_from_cube(badS);
        Assignment assumption = s->getS();
        assumption.insert(assumption.end(), inputs.begin(), inputs.end());
        bool res = currentSolver->solve_with_assumption(assumption, flag);
        assert(!res);
        Cube uc = currentSolver->get_uc();
        // filter uc
        for (int i = 0; i < uc.size(); ++i) {
            if ((abs(uc[i]) < _model->num_inputs() + 1) ||
                (abs(uc[i]) > _model->num_latches() + _model->num_inputs())) {
                uc.erase(uc.begin() + i);
                --i;
            }
        }
        assert(!uc.empty());
        return uc;

        // get partial of s by ucBad|input
        // Assignment assumption = badState->getS();
        // Assignment badSInput = badState->getInputVec();
        // assumption.insert(assumption.end(), badSInput.begin(),
        // badSInput.end()); Assignment negBad; for (const auto &elem : t)
        // {
        // 	negBad.push_back(-elem);
        // }
        // // badInput/\badLatch/\~t
        // res = currentSolver->solve_with_assumption(assumption, negBad);
        // assert(!res);
        // Cube ucBad = currentSolver->get_uc();
        // // filter input
        // for (int i = 0; i < ucBad.size(); ++i)
        // {
        // 	if ((abs(ucBad[i]) < _model->num_inputs() + 1) || (abs(ucBad[i])
        // > _model->num_latches() + _model->num_inputs()))
        // 	{
        // 		ucBad.erase(ucBad.begin() + i);
        // 		--i;
        // 	}
        // }
        // Assignment assumption = s->getS();
        // assumption.insert(assumption.end(), inputs.begin(), inputs.end());
        // res = currentSolver->solve_with_assumption(s->getS(), ucBad);
        // assert(!res);
        // Cube uc = currentSolver->get_uc();
        // // filter input
        // for (int i = 0; i < uc.size(); ++i)
        // {
        // 	if ((abs(uc[i]) < _model->num_inputs() + 1) || (abs(uc[i]) >
        // _model->num_latches() + _model->num_inputs()))
        // 	{
        // 		uc.erase(uc.begin() + i);
        // 		--i;
        // 	}
        // }
        // return uc;
    }

    // SAT(I, ~P)
    bool Checker::immediate_satisfiable() {
        bool res = solver_solve_with_assumption(_init->s(), _bad);
        if (res) {
            Assignment st = _solver->get_model();
            // if (verbose_)
            std::cout << "immediate_satisfiable" << std::endl;
            std::pair<Assignment, Assignment> pairA = state_pair(st);
            _init->set_inputs(pairA.first);
            return UNSAFE;
        }

        return false;
    }

    // get successor, st = model_: inputs + latches
    std::pair<Assignment, Assignment>
    Checker::state_pair(const Assignment &st) {
        Assignment inputs, latches;
        for (int i = 0; i < _model->num_inputs(); i++)
            // inputs of next state
            inputs.push_back(st[i]);

        // get latch
        for (int i = _model->num_inputs(); i < st.size(); i++) {
            if (abs(st[i]) > _model->num_inputs() + _model->num_latches())
                break;
            latches.push_back(st[i]);
        }
        return std::pair<Assignment, Assignment>(inputs, latches);
    }

    // compute bad state reachable
    bool Checker::immediate_satisfiable(const State *s) {
        bool res =
            solver_solve_with_assumption(const_cast<State *>(s)->s(), _bad);
        if (res) {
            // s is actually the last_ state
            Assignment st = _solver->get_model();
            if (_verbose)
                std::cout << "	bool Checker::immediate_satisfiable(const "
                             "State *s)"
                          << std::endl;
            std::pair<Assignment, Assignment> pa = state_pair(st);
            const_cast<State *>(s)->set_last_inputs(pa.first);
            _last = new State(const_cast<State *>(s));
            _last->set_final(true);
            //////generate dot data
            if (_dot != nullptr)
                (*_dot) << "\n\t\t\t" << _last->id()
                        << " [shape = circle, color = red, label = \"final\", "
                           "size = "
                           "0.01];";
            //////generate dot data
            return UNSAFE;
        }
    }

    // a copy for cube
    bool Checker::immediate_satisfiable(const Cube &cu) {
        bool res = solver_solve_with_assumption(cu, _bad);
        return res;
    }

    bool Checker::solve_with(const Cube &s, MainSolver *currentMainSolver) {
        bool res = solver_solve_with_assumption(s, currentMainSolver);
        return res;
    }

    // get new state from solver
    State *Checker::get_new_state(const State *s, const bool forward,
                                  MainSolver *currentMainSolver) {
        // get state
        Assignment st = currentMainSolver->get_state(forward);
        // isolate
        std::pair<Assignment, Assignment> pa = state_pair(st);
        // combine input and latch as new state
        State *res = new State(s, pa.first, pa.second);
        return res;
    }

    // get new state from trySolver
    State *Checker::get_new_state2(const State *s, const bool forward) {
        // get state
        Assignment st = _trySolver->get_state(forward);
        // isolate
        std::pair<Assignment, Assignment> pa = state_pair(st);
        State *res = new State(s, pa.first, pa.second);
        return res;
    }

    void Checker::printEvidence(std::ofstream &out) {
        while (!_traceStack.empty()) {
            auto topElement = _traceStack.top();
            _traceStack.pop();
            out << topElement << std::endl;
        }
    }

    bool cmp(const int a, const int b) { return abs(a) < abs(b); }

    Assignment Checker::getPrimeOfCube(const Cube &latchList) {
        Assignment res;
        // var
        for (const auto &latch : latchList) {
            int p = _model->prime(latch);
            res.push_back(p);
        }
        return res;
    }

    int Checker::getPrimeOfBad(int bad) {
        // bad
        int res = _model->output_prime(bad);
        // assume must exist
        assert(res != 0);
        return res;
    }

    /*
    getElementOfBigC, skip primeOfBad
    */
    Cube Checker::getElementOfBigC(std::vector<Cube> &bigC) {
        Cube res;
        int primeOfBad = getPrimeOfBad(_bad);
        for (const auto &elem : bigC)
            for (const auto &elem2 : elem) {
                if (elem2 == primeOfBad) {
                    // skip
                    continue;
                }
                res.push_back(elem2);
            }
        return res;
    }

    /**
     * @brief do tseitin transformation for a cube
     * transform disjunction of many cubes to conjunction of many *
     * clauses. Only and, or, not operator exist
     * @return flag that make clause work
     */
    void Checker::tseitinTransform(const std::vector<Cube> &disjunctionOfCubes,
                                   std::vector<Clause> &conjunctionOfClause) {
        // preparation
        for (auto &elem : conjunctionOfClause)
            elem.clear();
        conjunctionOfClause.clear();
        // main
        std::vector<int> flagVec;
        for (const auto &cube : disjunctionOfCubes) {
            int flag = _flag++;
            flagVec.push_back(flag);
            // add clause
            Clause clause1, clause2;
            // part1, ((a/\b/\c/\d)->flag) <-> (~flag\/(a/\b/\c/\d))
            // part2, (flag->(a/\b/\c/\d)) <-> (~(a/\b/\c/\d)\/flag)
            for (const auto &literal : cube) {
                clause1.push_back(-literal);
                clause2.push_back(literal);
                clause2.push_back(-flag);
                conjunctionOfClause.push_back(clause2);
                clause2.clear();
            }
            clause1.push_back(flag);
            conjunctionOfClause.push_back(clause1);
            clause1.clear();
        }
        // add clause contains all flags, flag1\/flag2\/flag3...
        Clause flagClauses;
        for (const auto &flag : flagVec) {
            flagClauses.push_back(flag);
        }
        conjunctionOfClause.push_back(flagClauses);
    }

    /**
     * @brief judge if init state contained in current state
     *
     * @param s
     * @param init
     * @return true, init contained in s
     * @return false, init not contained in s
     */
    bool Checker::isInitContainedInS(Cube s, Cube init) {
        std::set<int> initSet(init.begin(), init.end());
        for (const auto &elem : s) {
            if (initSet.find(elem) == initSet.end())
                return false;
        }
        return true;
    }

    /**
     * @brief used for block inv forever in SAT solver
     *
     * @param bigC inv to block
     * @param solver
     */
    void Checker::blockInvForever(const std::vector<Cube> &bigC,
                                  MainSolver *currentSolver) {
        if (!bigC.empty()) {
            if (bigC.size() == 1) {
                Cube uc = bigC[0];
                for (const int &literal : uc) {
                    currentSolver->add_clause(-literal);
                }
            } else {
                for (auto elem : bigC)
                    currentSolver->update_constraint(elem);
            }
        }
    }

    /**
     * @brief used for block inv forever in SAT solver
     *
     * @param bigC inv to block
     * @param solver
     * @param blockInvList list of inv to block
     */
    void
    Checker::blockInvForever(const std::vector<Cube> &bigC,
                             MainSolver *currentSolver,
                             std::vector<std::vector<Cube>> &blockInvList) {
        if (!bigC.empty()) {
            if (DEBUG) {
                std::cout << "block inv forever." << std::endl;
            }
            if (bigC.size() == 1) {
                blockInvList.push_back(bigC);
                Cube uc = bigC[0];
                for (const int &literal : uc) {
                    currentSolver->add_clause(-literal);
                }
            } else {
                std::vector<Clause> blockInv;
                tseitinTransform(bigC, blockInv);
                blockInvList.push_back(blockInv);
                for (auto elem : blockInv)
                    currentSolver->update_constraint(elem);
            }
        }
    }

    /**
     * @brief add bigC to solver, used for check with bad
     *
     * @param bigC
     * @param solver
     */
    void Checker::addBigCToSolver(const std::vector<Cube> &bigC,
                                  MainSolver *currentSolver) {
        if (bigC.empty())
            return;
        // add C by tseitinTransform
        if (bigC.size() == 1) {
            Cube c = bigC[0];
            for (const int &literal : c) {
                currentSolver->add_clause(literal);
            }
        } else {
            std::vector<Cube> conjunctiveBigC;
            tseitinTransform(bigC, conjunctiveBigC);
            for (Clause clause : conjunctiveBigC) {
                currentSolver->add_clause(clause);
            }
        }
    }

    /**
     * @brief used for add bigC to solver, used for check with state
     *
     * @param bigC
     * @param solver
     * @param bigCList
     */
    void Checker::addBigCToSolver(const std::vector<Cube> &bigC,
                                  MainSolver *solver,
                                  std::vector<std::vector<Cube>> &bigCList) {
        if (bigC.empty())
            return;
        // add C by tseitinTransform
        if (bigC.size() == 1) {
            auto c = bigC[0];
            for (int literal : c) {
                solver->add_clause(literal);
            }
            bigCList.push_back(bigC);
        } else {
            std::vector<Cube> conjunctiveBigC;
            tseitinTransform(bigC, conjunctiveBigC);
            bigCList.push_back(conjunctiveBigC);
            for (auto element : conjunctiveBigC) {
                solver->add_clause(element);
            }
        }
    }
} // namespace uair