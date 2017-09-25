#include "read_fact_files.h"
#include <iostream>

// Call: ./dd decl facts output
// 1: input with all the facts
// 2: output subsets of the facts for the build of the lattice
int main(int argc, const char **argv)
{
    if (argc < 4) {
        std::cerr << "Missing file facts name as input and/or output summary file name." << std::endl;
        return 1;
    }

    // ouputs smt files with subsets of facts from the input file
    read_fact_filest* facts_subsets_writer = new read_fact_filest();
    if (facts_subsets_writer->load_facts(argv[1], argv[2])) {
        std::cerr << "Error reading the input file: " << argv[1] << " and/or " << argv[2] << std::endl;
        return 1;
    }
    facts_subsets_writer->save_facts_smt_queries(argv[3]);
    free(facts_subsets_writer);
    // End of .smt files query creation for co-exist test


    // TODO: add missing extraction of query once needed


    return 0;
}
