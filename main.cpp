// UAIR model checking

#include "main.h"

using namespace std;
// using namespace uair;

uair::Statistics stats;
ofstream *dot_file = nullptr;
uair::Model *model = nullptr;
uair::Checker *ch = nullptr;

void signal_handler(int sig_num) {
    if (ch != nullptr) {
        delete ch;
        ch = nullptr;
    }

    if (model != nullptr) {
        delete model;
        model = nullptr;
    }
    stats.count_total_time_end();
    stats.print();

    // write the dot file tail
    if (dot_file != nullptr) {
        (*dot_file) << "\n}" << endl;
        dot_file->close();
        delete dot_file;
    }
    exit(0);
}

string get_file_name(string &s) {
    size_t start_pos = s.find_last_of("/");
    if (start_pos == string::npos)
        start_pos = 0;
    else
        start_pos += 1;

    string tmp_res = s.substr(start_pos);

    string res = "";
    // remove .aig

    size_t end_pos = tmp_res.find(".aig");
    assert(end_pos != string::npos);

    for (int i = 0; i < end_pos; i++)
        res += tmp_res.at(i);

    return res;
}

void print_usage() {
    printf("Usage: UAIR [-e|-v|-h] <aiger file> <output directory>\n");
    printf("       -e              print witness (Default = off)\n");
    printf(
        "       -v              print verbose information (Default = off)\n");
    printf("       -h              print help information\n");
    exit(0);
}

void check_aiger(int argc, char **argv) {
    bool verbose = false;
    // print trace if unsafe, by default
    bool evidence = true;
    bool minimal_uc = false;
    bool gv = false; // to print dot format for graphviz

    string input;
    string output_dir;
    bool input_set = false;
    bool output_dir_set = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0)
            verbose = true;
        else if (strcmp(argv[i], "-e") == 0)
            evidence = true;
        else if (strcmp(argv[i], "-h") == 0)
            print_usage();
        else if (!input_set) {
            input = string(argv[i]);
            input_set = true;
        } else if (!output_dir_set) {
            output_dir = string(argv[i]);
            output_dir_set = true;
        } else
            print_usage();
    }

    if (!input_set) {
        // for debug easily
        // safe
        input = "../case/6s102.aig";
        // unsafe
        // input = "../unsafeCases/counterp0.aig";
    }
    if (!output_dir_set) {
        output_dir = "./outputFiles";
        // print_usage();
    }

    // std::string output_dir (argv[3]);
    if (output_dir.at(output_dir.size() - 1) != '/')
        output_dir += "/";
    // std::string s (argv[2]);
    std::string filename = get_file_name(input);

    std::string stdout_filename = output_dir + filename + ".log";
    std::string stderr_filename = output_dir + filename + ".err";
    std::string res_file_name = output_dir + filename + ".res";

    std::string dot_file_name = output_dir + filename + ".gv";

    cout << "checking " << input << endl;
    if (!verbose)
        freopen(stdout_filename.c_str(), "w", stdout);
    ofstream res_file;
    res_file.open(res_file_name.c_str());

    if (gv) {
        dot_file = new ofstream();
        dot_file->open(dot_file_name.c_str());
        // prepare the dot header
        (*dot_file)
            << "graph system {\n\t\t\tnode [shape = point];\n\t\t\tedge "
               "[penwidth = 0.1];\n\t\t\tratio = auto;";
    }
    stats.count_total_time_start();
    // get aiger object
    aiger *aig = aiger_init();
    // aiger_open_and_read_from_file(aig, s.c_str());
    aiger_open_and_read_from_file(aig, input.c_str());
    const char *err = aiger_error(aig);
    if (err) {
        printf("read agier file error!\n");
        // throw InputError(err);
        exit(0);
    }
    if (!aiger_is_reencoded(aig))
        aiger_reencode(aig);

    stats.count_model_construct_time_start();
    // initiate model by aiger
    model = new uair::Model(aig);
    stats.count_model_construct_time_end();

    // print model
    if (verbose)
        model->print();

    // set data_structure's num_inputs and num_latches
    uair::State::set_num_inputs_and_latches(model->num_inputs(),
                                            model->num_latches());

    // assume that there is only one output needs to be checked in each aiger
    // model, which is consistent with the HWMCC format
    assert(model->num_outputs() >= 1);

    // initiate Checker by model, stats, ...
    ch = new uair::Checker(model, stats, dot_file, evidence, verbose,
                           minimal_uc);

    aiger_reset(aig);

    // running checking
    ch->check(res_file);

    cout << "checking finished\n" << endl;
    // clean work
    delete model;
    model = nullptr;
    res_file.close();

    // write the dot file tail
    if (dot_file != nullptr) {
        (*dot_file) << "\n}" << endl;
        dot_file->close();
        delete dot_file;
        dot_file = nullptr;
    }
    stats.count_total_time_end();
    stats.print();
    delete ch;
    ch = nullptr;
}

int main(int argc, char **argv) {
    // start checking model
    check_aiger(argc, argv);
    return 0;
}
