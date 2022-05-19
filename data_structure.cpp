#include "data_structure.h"
#include "utility.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <set>
#include <vector>

using namespace std;

namespace uair {

    // initiate successor state by s, inputs and latches
    State::State(const State *s, const Assignment &inputs,
                 const Assignment &latches, const bool last) {
        pre_ = const_cast<State *>(s);
        next_ = nullptr;
        inputs_ = inputs;
        _s = latches;
        detect_dead_start_ = 0;
        init_ = false;
        id_ = id_counter_++;
        if (s == nullptr)
            dep_ = 0;
        else
            dep_ = s->dep_ + 1;
        work_count_ = 0;
    }

    bool State::imply(const Cube &cu) const {
        for (int i = 0; i < cu.size(); i++) {
            int index = abs(cu[i]) - num_inputs_ - 1;
            assert(index >= 0);
            if (_s[index] != cu[i])
                return false;
        }
        return true;
    }

    Cube State::intersect(const Cube &cu) {
        Cube res;
        for (int i = 0; i < cu.size(); i++) {
            int index = abs(cu[i]) - num_inputs_ - 1;
            assert(index >= 0);
            if (_s[index] == cu[i])
                res.push_back(cu[i]);
        }
        return res;
    }

    void State::print_evidence(ofstream &out) {
        State *nx = this;
        vector<string> tmp;
        State *start = this;
        // reversve the states order
        tmp.push_back(start->last_inputs());
        while (start->pre() != nullptr) {
            tmp.push_back(start->inputs());
            start = start->pre();
        }
        // start now is the initial state
        for (int i = tmp.size() - 1; i >= 0; i--) {
            if (i == tmp.size() - 1) // init state
                out << start->latches() << endl;
            out << tmp[i] << endl;
        }
    }

    string State::inputs() {
        string res = "";
        for (int i = 0; i < inputs_.size(); i++)
            res += (inputs_[i] > 0) ? "1" : "0";
        return res;
    }

    string State::last_inputs() {
        string res = "";
        for (int i = 0; i < last_inputs_.size(); i++)
            res += (last_inputs_[i] > 0) ? "1" : "0";
        return res;
    }

    string State::latches() {
        string res = "";
        // int input_size = inputs_.size ();
        int j = 0;
        for (int i = 0; i < num_latches_; i++) {
            if (j == _s.size())
                res += "x";
            else if (num_inputs_ + i + 1 < abs(_s[j]))
                res += "x";
            else {
                res += (_s[j] > 0) ? "1" : "0";
                j++;
            }
        }
        return res;
    }

    int State::num_inputs_ = 0;
    int State::num_latches_ = 0;
    int State::id_counter_ = 1;

    void State::set_num_inputs_and_latches(const int n1, const int n2) {
        num_inputs_ = n1;
        num_latches_ = n2;
    }

    void State::dump() {
        for (const auto &elem : _s) {
            std::cout << elem << " ";
        }
        std::cout << endl;
    }
} // namespace uair
