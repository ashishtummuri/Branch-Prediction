#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_bp.h"
#include <inttypes.h>
#include <vector>
#include <bitset>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

FILE *FP;         // File handler
char *trace_file; // Variable that holds the trace file name;
bp_params params; // look at sim_bp.h header file for the definition of struct bp_params
char outcome;           // Variable holds branch outcome
unsigned long int addr; // Variable holds the address read from the input file
contents test, test2, test3;
int size = 0;
char str[2];

int converter(std::string binary) {
    int decimal = 0;
    int ind = 0;
    for (int i = binary.length() - 1; i >= 0; i--) {
        if (binary[i] == '1') {
            decimal += pow(2, ind);
        }
        ind++;
    }
    return decimal;
}

void initializeVector(std::vector<contents>& vect, unsigned long int size, int bit_count) {
    vect.reserve(size); 
    for (int i = 0; i < size; i++) {
        contents temp; 
        temp.final_counter = 2;
        temp.index_in_decimal = i;
        temp.index_in_binary = std::bitset<32>(i).to_string();
        temp.index_in_binary = temp.index_in_binary.substr(32 - bit_count, bit_count);
        vect.push_back(temp); 
    }

}

void processBimodal(contents& test, std::vector<contents>& vect, char outcome) {
    test.predictions++;
    // outcome = str[0];
    test.address_in_binary = std::bitset<32>(addr).to_string();
    test.index_in_binary = test.address_in_binary.substr(32 - (params.M2 + 2), params.M2);

    for (unsigned long int i = 0; i < vect.size(); i++) {
        if (test.index_in_binary == vect[i].index_in_binary) {
            if (outcome == 't') {
                if (vect[i].final_counter < 2) {
                    test.mispredictions++;
                }
                vect[i].final_counter++;
                if (vect[i].final_counter > 3) {
                    vect[i].final_counter = 3;
                }
            } else {
                if (vect[i].final_counter > 1) {
                    test.mispredictions++;
                }
                vect[i].final_counter--;
                if (vect[i].final_counter < 0) {
                    vect[i].final_counter = 0;
                }
            }
            break;
        }
    }
}

void processGshare(contents& test, std::vector<contents>& vect, std::vector<char>& N_bit, char outcome) {
    test.predictions++;
    // outcome = str[0];
    test.address_in_binary = std::bitset<32>(addr).to_string();
    test.index_in_binary = test.address_in_binary.substr(32 - (params.M1 + 2), params.M1);

    for (unsigned long int i = 0; i < params.N; i++) {
        if (test.index_in_binary[i] == N_bit[i]) {
            test.index_in_binary[i] = '0';
        } else {
            test.index_in_binary[i] = '1';
        }
    }

    for (unsigned long int i = 0; i < vect.size(); i++) {
        if (test.index_in_binary == vect[i].index_in_binary) {
            if (outcome == 't') {
                if (vect[i].final_counter < 2) {
                    test.mispredictions++;
                }
                vect[i].final_counter++;
                if (vect[i].final_counter > 3) {
                    vect[i].final_counter = 3;
                }
                for (int i = params.N - 1; i > 0; i--)
                        {
                            N_bit[i] = N_bit[i - 1];
                        }
                        N_bit[0] = '1';
            } 
            else {
                if (vect[i].final_counter > 1) {
                    test.mispredictions++;
                }
                vect[i].final_counter--;
                if (vect[i].final_counter < 0) {
                    vect[i].final_counter = 0;
                }
                for (int i = params.N - 1; i > 0; i--)
                {
                    N_bit[i] = N_bit[i - 1];
                }
                N_bit[0] = '0';
            }
            break;
        }
    }
}

void processHybrid(contents& test, std::vector<contents>& vect, std::vector<contents>& vect2, std::vector<contents>& vect3, std::vector<char>& N_bit, char outcome) {
    test.predictions++;
    // outcome = str[0];
    test.address_in_binary = std::bitset<32>(addr).to_string();
    test.index_in_binary = test.address_in_binary.substr(32 - (params.M2 + 2), params.M2);
    test2.index_in_binary = test.address_in_binary.substr(32 - (params.M1 + 2), params.M1);
    test3.index_in_binary = test.address_in_binary.substr(32 - (params.K + 2), params.K);

    for (unsigned long int i = 0; i < params.N; i++) {
        if (test2.index_in_binary[i] == N_bit[i]) {
            test2.index_in_binary[i] = '0';
        } else {
            test2.index_in_binary[i] = '1';
        }
    }

    unsigned long int decimal = converter(test.index_in_binary);
    unsigned long int decimal2 = converter(test2.index_in_binary);
    unsigned long int i = converter(test3.index_in_binary);

    if (test3.index_in_binary == vect3[i].index_in_binary) {
        if (vect3[i].final_counter >= 2) {
            vect2[decimal2].initial_counter = vect2[decimal2].final_counter;
            vect[decimal].initial_counter = vect[decimal].final_counter;

            if (outcome == 't') {
                if (vect2[decimal2].final_counter < 2) {
                    test.mispredictions++;
                }
                vect2[decimal2].final_counter++;
                if (vect2[decimal2].final_counter > 3) {
                    vect2[decimal2].final_counter = 3;
                }
            } else {
                if (vect2[decimal2].final_counter > 1) {
                    test.mispredictions++;
                }
                vect2[decimal2].final_counter--;
                if (vect2[decimal2].final_counter < 0) {
                    vect2[decimal2].final_counter = 0;
                }
            }
        }
        else
        {
        vect2[decimal2].initial_counter = vect2[decimal2].final_counter;
        vect[decimal].initial_counter = vect[decimal].final_counter;
        if (outcome == 't')
        {
            if (vect[decimal].final_counter < 2)
            {
                test.mispredictions = test.mispredictions + 1;
            }

            vect[decimal].final_counter = vect[decimal].final_counter + 1;
            if (vect[decimal].final_counter > 3)
            {
                vect[decimal].final_counter = 3;
            }
        }
        else
        {
            if (vect[decimal].final_counter > 1)
            {
                test.mispredictions = test.mispredictions + 1;
            }

            vect[decimal].final_counter = (vect[decimal].final_counter) - 1;
            if (vect[decimal].final_counter < 0)
            {
                vect[decimal].final_counter = 0;
            }
        }
    }
            if (outcome == 't') {
                for (int i = params.N - 1; i > 0; i--) {
                    N_bit[i] = N_bit[i - 1];
                }
                N_bit[0] = '1';
                } else {
                for (int i = params.N - 1; i > 0; i--) {
                    N_bit[i] = N_bit[i - 1];
                }
                N_bit[0] = '0';
            }

            if (vect[decimal].initial_counter > 1) {
                vect[decimal].prediction = "taken";
            }
            if (vect[decimal].initial_counter < 2) {
                vect[decimal].prediction = "not_taken";
            }
            if (vect2[decimal2].initial_counter > 1) {
                vect2[decimal2].prediction = "taken";
            }
            if (vect2[decimal2].initial_counter < 2) {
                vect2[decimal2].prediction = "not_taken";
            }

            if (outcome == 't') {
                if (vect[decimal].prediction == "taken" && vect2[decimal2].prediction == "not_taken") {
                    vect3[i].final_counter--;
                    if (vect3[i].final_counter < 0) {
                        vect3[i].final_counter = 0;
                    }
                }
                if (vect[decimal].prediction == "not_taken" && vect2[decimal2].prediction == "taken") {
                    vect3[i].final_counter++;
                    if (vect3[i].final_counter > 3) {
                        vect3[i].final_counter = 3;
                    }
                }
            }
            if (outcome == 'n') {
                if (vect[decimal].prediction == "not_taken" && vect2[decimal2].prediction == "taken") {
                    vect3[i].final_counter--;
                    if (vect3[i].final_counter < 0) {
                        vect3[i].final_counter = 0;
                    }
                }
                if (vect[decimal].prediction == "taken" && vect2[decimal2].prediction == "not_taken") {
                    vect3[i].final_counter++;
                    if (vect3[i].final_counter > 3) {
                        vect3[i].final_counter = 3;
                    }
                }
            }
        }
    }

int main(int argc, char *argv[]) {
    
    std::vector<contents> vect, vect2, vect3;
    std::vector<char> N_bit;

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc - 1);
        exit(EXIT_FAILURE);
    }

    params.bp_name = argv[1];

    if (strcmp(params.bp_name, "bimodal") == 0) {
         if (argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.M2 = std::atoi(argv[2]);
        trace_file = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
        size = pow(2, params.M2);

        initializeVector(vect, size, params.M2);
    } else if (strcmp(params.bp_name, "gshare") == 0) 
    {
        if (argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.M1 = std::atoi(argv[2]);
        params.N = std::atoi(argv[3]);
        trace_file = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);
        size = pow(2, params.M1);

        initializeVector(vect, size, params.M1);

    } else if (strcmp(params.bp_name, "hybrid") == 0) 
    {
        if (argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.K = strtoul(argv[2], NULL, 10);
        params.M1 = strtoul(argv[3], NULL, 10);
        params.N = strtoul(argv[4], NULL, 10);
        params.M2 = strtoul(argv[5], NULL, 10);
        trace_file = argv[6];

        //initializeVector(vect3, pow(2, params.K), 1);
        for (int i = 0; i < pow(2, params.K); i++)
        {
            vect3.push_back(contents());
            vect3[vect3.size() - 1].final_counter = 1;
            vect3[vect3.size() - 1].index_in_decimal = i;
            vect3[vect3.size() - 1].index_in_binary = std::bitset<32>(vect3[vect3.size() - 1].index_in_decimal).to_string();
            vect3[vect3.size() - 1].index_in_binary = vect3[vect3.size() - 1].index_in_binary.substr((32 - params.K), params.K);
        }
        initializeVector(vect, pow(2, params.M2), params.M2);
        initializeVector(vect2, pow(2, params.M1), params.M1);

    } else {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }

    FP = fopen(trace_file, "r");
    if (FP == NULL)
    {
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    for (unsigned long int i = 0; i < params.N; i++)
    {
        N_bit.push_back('0');
    }
    char str[2];
    while (fscanf(FP, "%lx %s", &addr, str) != EOF) {
        outcome = str[0];  // Read the outcome from the input file
        if (strcmp(params.bp_name, "gshare") == 0) {
            processGshare(test, vect, N_bit, outcome);
        } else if (strcmp(params.bp_name, "bimodal") == 0) {
            processBimodal(test, vect, outcome);
        } else {
            processHybrid(test, vect, vect2, vect3, N_bit, outcome);
        }
}
    // Output results
    // ...
    if (strcmp(params.bp_name, "hybrid") == 0)
    {
        test.rate = ((double)test.mispredictions / (double)test.predictions) * 100;

        std::cout << "OUTPUT"
                  << "\n"
                  << "number of predictions: " << std::fixed << test.predictions;
        std::cout << "\n"
                  << "number of mispredictions: " << test.mispredictions << "\n";
        std::cout << "misprediction rate: ";
        std::cout << std::fixed << std::setprecision(2) << test.rate << "%"
                  << "\n";
        std::cout << "FINAL "
                  << "CHOOSER"
                  << " CONTENTS"
                  << "\n";

        for (unsigned long int i = 0; i < vect3.size(); i++)
        {
            std::cout << vect3[i].index_in_decimal << "\t" << vect3[i].final_counter << "\n";
        }
        std::cout << "FINAL "
                  << "GSHARE"
                  << " CONTENTS"
                  << "\n";

        for (unsigned long int i = 0; i < vect2.size(); i++)
        {
            std::cout << vect2[i].index_in_decimal << "\t" << vect2[i].final_counter << "\n";
        }
        std::cout << "FINAL "
                  << "BIMODAL"
                  << " CONTENTS"
                  << "\n";

        for (unsigned long int i = 0; i < vect.size(); i++)
        {
            std::cout << vect[i].index_in_decimal << "\t" << vect[i].final_counter << "\n";
        }
    }
    else
    {
        test.rate = ((double)test.mispredictions / (double)test.predictions) * 100;

        std::cout << "OUTPUT"
                  << "\n"
                  << "number of predictions: " << std::fixed << test.predictions;
        std::cout << "\n"
                  << "number of mispredictions: " << test.mispredictions << "\n";
        std::cout << "misprediction rate: ";
        std::cout << std::fixed << std::setprecision(2) << test.rate << "%"
                  << "\n";
        std::cout << "FINAL " << argv[1] << " CONTENTS"
                  << "\n";

        for (unsigned long int i = 0; i < vect.size(); i++)
        {
            std::cout << vect[i].index_in_decimal << "\t" << vect[i].final_counter << "\n";
        }
    }
    return 0;
}
