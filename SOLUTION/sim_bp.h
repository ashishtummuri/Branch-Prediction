#ifndef SIM_BP_H
#define SIM_BP_H
#include <string>


typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;

// Put additional data structures here as per your requirement


typedef struct parameters
{
    std::string binary_address;
    std::string binary_index;
    int index_in_decimal;
    int initial_counter;
    int final_counter;
    int number;
    unsigned long int predictions;
    unsigned long int mispredictions;
    double rate;
    int valid;
    std::string prediction;
} parameters;

#endif
