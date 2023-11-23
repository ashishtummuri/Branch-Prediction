#ifndef SIM_BP_H
#define SIM_BP_H
#include <string>

typedef struct bp_params
{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char *bp_name;
} bp_params;

typedef struct contents
{
    std::string address_in_binary;
    std::string index_in_binary;
    int index_in_decimal;
    int initial_counter;
    int final_counter;
    int number;
    unsigned long int predictions;
    unsigned long int mispredictions;
    double rate;
    int valid;
    std::string prediction;
} contents;



#endif
