#pragma once

#include "config.hpp"
void process_sdp_solution(ProblemConfig &problem_config);

void process_raw_dual(string in_name, string out_name);

vector<double> get_current_extremal_densities(ProblemConfig &problem_config,
                                              string file = "");
