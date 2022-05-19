#ifndef STATISTICS_H
#define STATISTICS_H

#include <ctime>
#include <iostream>
#include <stdlib.h>

namespace uair {

    class Statistics {
      public:
        Statistics() {
            num_SAT_calls_ = 0;
            num_main_solver_SAT_calls_ = 0;
            num_inv_solver_SAT_calls_ = 0;
            num_start_solver_SAT_calls_ = 0;
            time_SAT_calls_ = 0.0;
            time_total_ = 0.0;
            time_main_solver_SAT_calls_ = 0.0;
            time_inv_solver_SAT_calls_ = 0.0;
            time_start_solver_SAT_calls_ = 0.0;
            time_model_construct_ = 0.0;
            num_reduce_uc_SAT_calls_ = 0;
            time_reduce_uc_SAT_calls_ = 0.0;
            compress_reduce_uc_ratio_ = 0.0;
            orig_uc_size_ = 0;
            reduce_uc_size_ = 0;
            num_clause_contain_ = 0;
            num_state_contain_ = 0;
            num_clause_contain_success_ = 0;
            time_clause_contain_ = 0.0;
            time_state_contain_ = 0.0;
            num_detect_dead_state_SAT_calls_ = 0;
            time_detect_dead_state_SAT_calls_ = 0.0;
            num_detect_dead_state_success_ = 0;
        }
        ~Statistics() {}
        void print() {
            std::cout << "Time of model construct: " << time_model_construct_
                      << std::endl;
            std::cout << "Num of total SAT Calls: " << num_SAT_calls_
                      << std::endl;
            std::cout << "      Num of main solver SAT Calls: "
                      << num_main_solver_SAT_calls_ << std::endl;
            std::cout << "      Num of try solver SAT Calls: "
                      << num_try_solver_SAT_calls_ << std::endl;

            std::cout << "Time of total SAT Calls: " << time_SAT_calls_
                      << std::endl;
            std::cout << "      Time of main solver SAT Calls: "
                      << time_main_solver_SAT_calls_ << std::endl;
            std::cout << "      Time of try solver SAT Calls: "
                      << time_try_solver_SAT_calls_ << std::endl;

            std::cout << "Num of clause contain: " << num_clause_contain_
                      << std::endl;
            std::cout << "Time of clause contain: " << time_clause_contain_
                      << std::endl;
            std::cout << "Num of clause contain success: "
                      << num_clause_contain_success_ << std::endl;
            std::cout << "Clause contain successful rate: "
                      << (double)num_clause_contain_success_ /
                             num_clause_contain_
                      << std::endl;

            std::cout << "Num of state contain: " << num_state_contain_
                      << std::endl;
            std::cout << "Time of state contain: " << time_state_contain_
                      << std::endl;

            std::cout << "Total Time: " << time_total_ << std::endl;
        }
        inline void count_SAT_time_start() { begin_ = clock(); }
        inline void count_SAT_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            num_SAT_calls_ += 1;
            time_SAT_calls_ += duration;
        }
        inline void count_main_solver_SAT_time_start() { begin_ = clock(); }
        inline void count_main_solver_SAT_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_main_solver_SAT_calls_ += duration;
            time_SAT_calls_ += duration;
            num_main_solver_SAT_calls_ += 1;
            num_SAT_calls_ += 1;
        }

        inline void count_try_solver_SAT_time_start() { begin_ = clock(); }
        inline void count_try_solver_SAT_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_try_solver_SAT_calls_ += duration;
            time_SAT_calls_ += duration;
            num_try_solver_SAT_calls_ += 1;
            num_SAT_calls_ += 1;
        }

        inline void count_inv_solver_SAT_time_start() { begin_ = clock(); }
        inline void count_inv_solver_SAT_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_inv_solver_SAT_calls_ += duration;
            time_SAT_calls_ += duration;
            num_inv_solver_SAT_calls_ += 1;
            num_SAT_calls_ += 1;
        }
        inline void count_start_solver_SAT_time_start() { begin_ = clock(); }
        inline void count_start_solver_SAT_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_start_solver_SAT_calls_ += duration;
            time_SAT_calls_ += duration;
            num_start_solver_SAT_calls_ += 1;
            num_SAT_calls_ += 1;
        }
        inline void count_total_time_start() { total_begin_ = clock(); }
        inline void count_total_time_end() {
            total_end_ = clock();
            double duration =
                double(total_end_ - total_begin_) / CLOCKS_PER_SEC;
            time_total_ += duration;
        }
        inline void count_model_construct_time_start() {
            model_begin_ = clock();
        }
        inline void count_model_construct_time_end() {
            model_end_ = clock();
            double duration =
                double(model_end_ - model_begin_) / CLOCKS_PER_SEC;
            time_model_construct_ += duration;
        }
        inline void count_reduce_uc_SAT_time_start() { begin_ = clock(); }
        inline void count_reduce_uc_SAT_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_reduce_uc_SAT_calls_ += duration;
            time_SAT_calls_ += duration;
            num_reduce_uc_SAT_calls_ += 1;
            num_SAT_calls_ += 1;
        }
        inline void count_orig_uc_size(int size) { orig_uc_size_ += size; }
        inline void count_reduce_uc_size(int size) { reduce_uc_size_ += size; }
        inline void count_clause_contain_time_start() { begin_ = clock(); }
        inline void count_clause_contain_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_clause_contain_ += duration;
            num_clause_contain_ += 1;
        }
        inline void count_clause_contain_success() {
            num_clause_contain_success_ += 1;
        }
        inline void count_state_contain_time_start() { begin_ = clock(); }
        inline void count_state_contain_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_state_contain_ += duration;
            num_state_contain_ += 1;
        }

        inline void count_detect_dead_state_time_start() { begin_ = clock(); }
        inline void count_detect_dead_state_time_end() {
            end_ = clock();
            double duration = double(end_ - begin_) / CLOCKS_PER_SEC;
            time_detect_dead_state_SAT_calls_ += duration;
            num_detect_dead_state_SAT_calls_ += 1;
        }
        inline void count_detect_dead_state_success() {
            num_detect_dead_state_success_ += 1;
        }

      private:
        int num_SAT_calls_;
        double time_SAT_calls_;

        int num_main_solver_SAT_calls_;
        double time_main_solver_SAT_calls_;

        int num_try_solver_SAT_calls_;
        double time_try_solver_SAT_calls_;

        int num_inv_solver_SAT_calls_;
        double time_inv_solver_SAT_calls_;

        int num_start_solver_SAT_calls_;
        double time_start_solver_SAT_calls_;

        double time_total_;
        double time_model_construct_;
        int num_reduce_uc_SAT_calls_;
        double time_reduce_uc_SAT_calls_;
        double compress_reduce_uc_ratio_;
        int orig_uc_size_;
        int reduce_uc_size_;

        int num_clause_contain_;
        int num_state_contain_;
        int num_clause_contain_success_;
        double time_clause_contain_;
        double time_state_contain_;
        int num_detect_dead_state_SAT_calls_;
        double time_detect_dead_state_SAT_calls_;

        int num_detect_dead_state_success_;

        clock_t begin_, end_;
        clock_t total_begin_, total_end_;
        clock_t model_begin_, model_end_;
    };

} // namespace uair

#endif
