#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

namespace uair {
    typedef std::vector<int> Assignment;
    typedef std::vector<int> Cube;
    typedef std::vector<int> Clause;

    // state
    class State {
      public:
        State() {}
        State(const Assignment &latches)
            : _s(latches), pre_(nullptr), next_(nullptr) {}

        State(const State *s, const Assignment &inputs,
              const Assignment &latches, const bool last = false);

        State(State *s)
            : pre_(s->pre_), next_(s->next_), _s(s->_s), inputs_(s->inputs_),
              last_inputs_(s->last_inputs_), init_(s->init_), id_(s->id_),
              dep_(s->dep_) {}

        ~State() {}

        bool imply(const Cube &cu) const;

        Cube intersect(const Cube &cu);

        inline void set_detect_dead_start(int pos) { detect_dead_start_ = pos; }

        inline int detect_dead_start() { return detect_dead_start_; }

        // set latch
        inline void set_latches(const Assignment &st) { _s = st; }

        inline void set_inputs(const Assignment &st) { inputs_ = st; }

        inline void set_last_inputs(const Assignment &st) { last_inputs_ = st; }

        inline void set_initial(bool val) { init_ = val; }

        inline void set_final(bool val) { final_ = val; }

        inline void set_depth(int pos) { dep_ = pos; }

        inline int id() { return id_; }

        inline void print() { std::cout << latches() << std::endl; }

        void dump();

        void print_evidence(std::ofstream &);

        inline int depth() { return dep_; }

        inline Assignment &s() { return _s; }

        const inline Assignment &getS() const { return _s; }

        inline State *next() { return next_; }

        inline State *pre() { return pre_; }

        inline Assignment &inputs_vec() { return inputs_; }

        const inline Assignment &getInputVec() const { return inputs_; }

        std::string inputs();

        std::string last_inputs();

        std::string latches();

        inline int size() { return _s.size(); }

        inline int element(int i) { return _s[i]; }

        const inline int getElement(int i) const { return _s[i]; }

        inline void set_next(State *nx) { next_ = nx; }

        static void set_num_inputs_and_latches(const int n1, const int n2);

        inline void set_nexts(std::vector<int> &nexts) {
            nexts_ = nexts;
            computed_next_ = true;
        }

        inline std::vector<int> &nexts() { return nexts_; }

        inline bool computed_next() const { return computed_next_; }

        inline int work_level() const { return work_level_; }

        inline void set_work_level(int id) { work_level_ = id; }

        inline void work_count_inc() { work_count_++; }

        inline int work_count() { return work_count_; }

        inline int work_count_reset() { work_count_ = 0; }

      private:
        // _s contains all latches, but if the value of latch l is not cared,
        // assign it to -1.
        Assignment _s;
        State *next_;
        State *pre_;
        std::vector<int> inputs_;
        std::vector<int> last_inputs_; // for backward CAR only!

        bool init_;  // whether it is an initial state
        bool final_; // whether it is an final state
        int id_;     // the state id
        int dep_;    // the length from the starting state

        bool computed_next_; // flag to label whether the next part of the state
                             // has been computed
        std::vector<int> nexts_; // the next part which can be decided by the
                                 // state without input

        int work_level_;
        int work_count_;

        int detect_dead_start_; // to store the start position to check whether
                                // it is a dead state

        static int num_inputs_;
        static int num_latches_;
        static int id_counter_;
    };
} // namespace uair
#endif
