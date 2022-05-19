#ifndef MODEL_H
#define MODEL_H

extern "C" {
#include "aiger.h"
}
#include "data_structure.h"
#include "hash_map.h"
#include "hash_set.h"
#include <cassert>

#include "utility.h"
#include <assert.h>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <vector>
namespace uair {
    /**
     * @brief model and bad states. 从aiger中读取的模型
     *
     */
    class Model {
      public:
        Model(aiger *, const bool verbose = false);
        ~Model() {}

        int prime(const int);
        std::vector<int> previous(const int);

        bool state_var(const int id) {
            return (id >= 1) && (id <= num_inputs_ + num_latches_);
        }
        bool latch_var(const int id) {
            return (id >= num_inputs_ + 1) &&
                   (id <= num_inputs_ + num_latches_);
        }

        inline int num_inputs() { return num_inputs_; }
        inline int num_latches() { return num_latches_; }
        inline int num_ands() { return num_ands_; }
        inline int num_constraints() { return num_constraints_; }
        inline int num_outputs() { return num_outputs_; }
        inline int max_id() { return max_id_; }
        inline int outputs_start() { return outputs_start_; }
        inline int latches_start() { return latches_start_; }
        inline int size() { return cls_.size(); }
        // correspond clause
        inline std::vector<int> &element(const int id) { return cls_[id]; }
        inline int output(const int id) { return outputs_[id]; }
        /**
         * @brief get prime of output
         *
         * @param id
         * @return int
         */
        inline int output_prime(const int id) {
            return outputs_prime_.find(id) == outputs_prime_.end()
                       ? 0
                       : outputs_prime_.find(id)->second;
        }
        /**
         * @brief get prime of inputs
         *
         * @param id current input id
         * @return int, prime of id
         */
        inline int input_prime(const int id) {
            return inputs_prime_.find(id) == inputs_prime_.end()
                       ? 0
                       : inputs_prime_.find(id)->second;
        }

        inline Cube &init() { return init_; }

        void shrink_to_previous_vars(Cube &cu, bool &constraint);
        void shrink_to_latch_vars(Cube &cu, bool &constraint);

        inline int true_id() { return true_; }
        inline int false_id() { return false_; }

        // print information of model
        void print();

      private:
        // members
        bool verbose_;

        int num_inputs_;
        int num_latches_;
        int num_ands_;
        int num_constraints_;
        int num_outputs_;

        // maximum used id in the model
        int max_id_;

        int true_;  // id for true
        int false_; // id for false

        typedef std::vector<int> vect;
        // each element of Clauses is a clause(vector<int>)
        typedef std::vector<vect> Clauses;

        // initial state
        vect init_;
        // output ids
        vect outputs_;
        // prime of inputs
        std::map<int, int> inputs_prime_;
        // prime of outputs
        std::map<int, int> outputs_prime_;
        vect constraints_; // constraint ids
        // sets storing \/s
        Clauses
            cls_; // set of clauses, it contains three parts:
                  //(1) clauses for constraints, i.e. those before position
                  // outputs_start_; (2) clauses for outputs, i.e. those before
                  // position latches_start_; (3) clauses for latches, i.e. all

        // the index of cls_ to point the start position of outputs
        int outputs_start_;
        // the index of cls_ to point the start position of latches
        int latches_start_;

        // map int to int
        typedef hash_map<int, int> nextMap;
        // map from latches to their next values
        nextMap next_map_;
        // map from the next values of latches to latches
        typedef hash_map<int, std::vector<int>> reverseNextMap;
        reverseNextMap reverse_next_map_;
        // BE careful the situation when next (a) = c and next (b) = c!!

        // vars evaluated to be true, and their negation is false
        hash_set<unsigned> trues_;

        // functions
        inline bool is_true(const unsigned id) {
            return (id == 1) || (trues_.find(id) != trues_.end());
        }

        inline bool is_false(const unsigned id) {
            return (id == 0) ||
                   (trues_.find((id % 2 == 0) ? (id + 1) : (id - 1)) !=
                    trues_.end());
        }

        inline int car_var(const unsigned id) {
            assert(id != 0 && id != 0);
            return ((id % 2 == 0) ? (id / 2) : -(id / 2));
        }

        inline vect clause(int id) {
            vect res;
            res.push_back(id);
            return res;
        }

        inline vect clause(int id1, int id2) {
            vect res;
            res.push_back(id1);
            res.push_back(id2);
            return res;
        }

        inline vect clause(int id1, int id2, int id3) {
            vect res;
            res.push_back(id1);
            res.push_back(id2);
            res.push_back(id3);
            return res;
        }

        inline void set_outputs_start() { outputs_start_ = cls_.size(); }

        inline void set_latches_start() { latches_start_ = cls_.size(); }

        void collect_trues(const aiger *aig);
        void create_next_map(const aiger *aig);
        void create_clauses(const aiger *aig);
        void add_output_prime();
        void collect_necessary_gates(const aiger *aig, const aiger_symbol *as,
                                     const int as_size,
                                     hash_set<unsigned> &exist_gates,
                                     std::vector<unsigned> &gates,
                                     bool next = false);
        aiger_and *necessary_gate(const unsigned id, const aiger *aig);
        void recursively_add(const aiger_and *aa, const aiger *aig,
                             hash_set<unsigned> &exist_gates,
                             std::vector<unsigned> &gates);
        void add_clauses_from_gate(const aiger_and *aa);
        void set_init(const aiger *aig);
        void set_constraints(const aiger *aig);
        void set_outputs(const aiger *aig);
        void insert_to_reverse_next_map(const int index, const int val);

      public:
    };

} // namespace uair

#endif
