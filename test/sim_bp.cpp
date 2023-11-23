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
parameters module, module2, module3;
int size = 0;
char str[2];

int binary_to_decimal(const std::string& binary) {
    std::bitset<32> bits(binary);
    return static_cast<int>(bits.to_ulong());
}

void initializeVector(std::vector<parameters>& vect, unsigned long int size,unsigned long int final,unsigned long int bit_count) {
    vect.reserve(size); 
    for (int i = 0; i < size; i++) {
        parameters temp; 
        temp.final_counter = final;
        temp.index_in_decimal = final;
        temp.binary_index = std::bitset<32>(i).to_string();
        temp.binary_index = temp.binary_index.substr(32 - bit_count, bit_count);
        vect.push_back(temp); 
    }
}

void processBimodal(parameters& module, std::vector<parameters>& vect, char outcome) {
    module.predictions++;
    module.binary_address = std::bitset<32>(addr).to_string();
    module.binary_index = module.binary_address.substr(32 - (params.M2 + 2), params.M2);

    for (unsigned int i = 0; i < vect.size(); ++i) {
        if (module.binary_index == vect[i].binary_index) {
            bool wasTaken = vect[i].final_counter >= 2;
            bool outcomeTaken = outcome == 't';

            if (wasTaken != outcomeTaken) {
                module.mispredictions++;
            }
            if (outcomeTaken) {
                vect[i].final_counter = std::min(3, vect[i].final_counter + 1);
            } else {
                vect[i].final_counter = std::max(0, vect[i].final_counter - 1);
            }

            break; 
        }
    }
}

void processGshare(parameters& module, std::vector<parameters>& vect, std::vector<char>& N_bit, char outcome) {
    module.predictions++;
    module.binary_address = std::bitset<32>(addr).to_string();
    module.binary_index = module.binary_address.substr(32 - (params.M1 + 2), params.M1);

    for (unsigned long int i = 0; i < params.N; ++i) {
        module.binary_index[i] = (module.binary_index[i] == N_bit[i]) ? '0' : '1';
    }

    for (size_t i = 0; i < vect.size(); ++i) {
        if (module.binary_index == vect[i].binary_index) {
            bool wasTaken = vect[i].final_counter >= 2;
            bool outcomeTaken = outcome == 't';

            if (wasTaken != outcomeTaken) {
                module.mispredictions++;
            }

            vect[i].final_counter = outcomeTaken ? std::min(3, vect[i].final_counter + 1)
                                                : std::max(0, vect[i].final_counter - 1);

            std::rotate(N_bit.rbegin(), N_bit.rbegin() + 1, N_bit.rend());
            N_bit[0] = outcomeTaken ? '1' : '0';

            break;
        }
    }
}

void processHybrid(parameters& module, std::vector<parameters>& vect, std::vector<parameters>& vect2, std::vector<parameters>& vect3, std::vector<char>& N_bit, char outcome) {
    module.predictions++;

    module.binary_address = std::bitset<32>(addr).to_string();
    module.binary_index = module.binary_address.substr(32 - (params.M2 + 2), params.M2);
    module2.binary_index = module.binary_address.substr(32 - (params.M1 + 2), params.M1);
    module3.binary_index = module.binary_address.substr(32 - (params.K + 2), params.K);

    for (unsigned long int i = 0; i < params.N; i++) {
        if (module2.binary_index[i] == N_bit[i]) {
            module2.binary_index[i] = '0';
        } else {
            module2.binary_index[i] = '1';
        }
    }

    int decimal = binary_to_decimal(module.binary_index);
    int decimal2 = binary_to_decimal(module2.binary_index);
    unsigned long int i = binary_to_decimal(module3.binary_index);

    if (module3.binary_index == vect3[i].binary_index) {
        vect2[decimal2].initial_counter = vect2[decimal2].final_counter;
        vect[decimal].initial_counter = vect[decimal].final_counter;

        int *counter = (vect3[i].final_counter >= 2) ? &vect2[decimal2].final_counter : &vect[decimal].final_counter;

        if (outcome == 't') {
            if (*counter < 2) {
                module.mispredictions++;
            }
            *counter = (*counter < 3) ? (*counter + 1) : 3;
        } else {
            if (*counter > 1) {
                module.mispredictions++;
            }
            *counter = (*counter > 0) ? (*counter - 1) : 0;
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

            bool opposite_predictions = (vect[decimal].prediction == "taken" && vect2[decimal2].prediction == "not_taken") ||
                                            (vect[decimal].prediction == "not_taken" && vect2[decimal2].prediction == "taken");

            if (outcome == 't' && opposite_predictions) {
                vect3[i].final_counter += (vect[decimal].prediction == "not_taken" ? 1 : -1);
            }
            else if (outcome == 'n' && opposite_predictions) {
                vect3[i].final_counter += (vect[decimal].prediction == "taken" ? 1 : -1);
            }
            if (vect3[i].final_counter < 0) {
                vect3[i].final_counter = 0;
            } else if (vect3[i].final_counter > 3) {
                vect3[i].final_counter = 3;
            }
        }
    }

int main(int argc, char *argv[]) {
    
    std::vector<parameters> vect, vect2, vect3;
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

        initializeVector(vect, size, 2, params.M2);
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

        initializeVector(vect, size,2, params.M1);

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
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name,params.K, params.M1, params.N, params.M2, trace_file);


        initializeVector(vect3, pow(2, params.K),1, params.K);
        initializeVector(vect, pow(2, params.M2), 2, params.M2);
        initializeVector(vect2, pow(2, params.M1),2, params.M1);

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
        outcome = str[0]; 
        if (strcmp(params.bp_name, "gshare") == 0) {
            processGshare(module, vect, N_bit, outcome);
        } else if (strcmp(params.bp_name, "bimodal") == 0) {
            processBimodal(module, vect, outcome);
        } else {
            processHybrid(module, vect, vect2, vect3, N_bit, outcome);
        }
}
    // Output results
    // ...
    if (strcmp(params.bp_name, "hybrid") == 0)
    {
        module.rate = ((double)module.mispredictions / (double)module.predictions) * 100;

        std::cout << "OUTPUT"
                  << "\n"
                  << "number of predictions: " << std::fixed << module.predictions;
        std::cout << "\n"
                  << "number of mispredictions: " << module.mispredictions << "\n";
        std::cout << "misprediction rate: ";
        std::cout << std::fixed << std::setprecision(2) << module.rate << "%"
                  << "\n";
        std::cout << "FINAL " << "CHOOSER " << "CONTENTS" << "\n";

        for (unsigned long int i = 0; i < vect3.size(); i++)
        {
            std::cout << i << "\t" << vect3[i].final_counter << "\n";
        }
        std::cout << "FINAL " << "GSHARE " << "CONTENTS" << "\n";

        for (unsigned long int i = 0; i < vect2.size(); i++)
        {
            std::cout << i << "\t" << vect2[i].final_counter << "\n";
        }
        std::cout << "FINAL " << "BIMODAL " << "CONTENTS" << "\n";

        for (unsigned long int i = 0; i < vect.size(); i++)
        {
            std::cout << i << "\t" << vect[i].final_counter << "\n";
        }
    }
    else
    {
        module.rate = ((double)module.mispredictions / (double)module.predictions) * 100;

        std::cout << "OUTPUT"
                  << "\n"
                  << "number of predictions: " << std::fixed << module.predictions;
        std::cout << "\n"
                  << "number of mispredictions: " << module.mispredictions << "\n";
        std::cout << "misprediction rate: ";
        std::cout << std::fixed << std::setprecision(2) << module.rate << "%"
                  << "\n";
        std::cout << "FINAL " << argv[1] << " contents"
                  << "\n";

        for (unsigned long int i = 0; i < vect.size(); i++) {
        std::cout << i << "\t" << vect[i].final_counter << "\n";
        }
    }
    return 0;
}
