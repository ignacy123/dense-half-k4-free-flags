#include "solution-processor.hpp"
#include "config.hpp"
#include "flag-calculator.hpp"
#include "flag-generator.hpp"
#include "flag.hpp"
#include "option-parser.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

void save_processed_densities(FlagVector<0, V> raw_densities, ostream &ost,
                              int largest_flag_to_show, bool skip_if_empty) {
  for (int j = largest_flag_to_show; j > 0; j--) {
    ost << "Vector of extremal densities on " << j << " vertices:" << endl;
    ost << (int)get_unlabeled_flags(j).size() << endl;
    for (int i = 0; i < (int)get_unlabeled_flags(j).size(); i++) {
      flag_coeff fc;
      fc.g = get_unlabeled_flags(j)[i];
      fc.coefficient = raw_densities.density(j, i);
      if (fc.print(ost, skip_if_empty, false, to_string(i))) {
        ost << endl;
      }
    }
    ost << endl;
  }
}

void process_raw_dual(string in_name, string out_name) {
  ifstream infile;
  infile.open(in_name, ifstream::in);
  // TODO: Make sure this gets called automatically when we try to access the
  // unlabeled flags!
  generate_unlabeled_flags_of_size(V);
  FlagVector<0, V> densities;
  for (int i = 0; i < (int)get_unlabeled_flags(V).size(); i++) {
    double density;
    infile >> density;
    flag current;
    current.load_from_stream(infile, -1, -1);
    infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    flag_coeff fc;
    fc.g = current;
    fc.coefficient = density;
    densities += fc;
  }
  ofstream outfile;
  outfile.open(out_name, ofstream::out);

  save_processed_densities(densities, outfile, V, false);
}

void process_csdp_solution(istream &ist, ostream &ost, int largest_flag_to_show,
                           bool skip_if_empty) {

  double sum = 0;
  double sum_print = 0;
  double count_printed = 0;

  // TODO: Make sure this gets called automatically when we try to access the
  // unlabeled flags!
  generate_unlabeled_flags_of_size(V);
  FlagVector<0, V> densities;
  for (int i = 0; i < (int)get_unlabeled_flags(V).size(); i++) {
    double density;
    ist >> density;

    sum += density;

    count_printed++;
    sum_print += density;
    densities.set_coefficient(i, density);
  }

  save_processed_densities(densities, ost, largest_flag_to_show, skip_if_empty);

  ost << "Printed: " << count_printed << "/" << get_unlabeled_flags(V).size()
      << "  Sum of coefficients: " << sum_print << "/" << sum << endl;
}

void process_sdp_solution(ProblemConfig &problem_config) {
  ifstream results;
  results.open(problem_config.get_sdp_solution_name().c_str(), ifstream::in);
  if (!results.good()) {
    cerr << "Failed opening file with CSDP result "
         << problem_config.get_sdp_solution_name() << endl;
    exit(1);
  }
  cerr << "Processing CSDP solution file: "
       << problem_config.get_sdp_solution_name() << endl;
  cout << endl;
  process_csdp_solution(results, cout, problem_config.largest_flag_to_show,
                        problem_config.skip_empty_flags_when_processing);
  results.close();
  results.open(problem_config.get_sdp_solution_name().c_str(), ifstream::in);
  if (!results.good()) {
    cerr << "Failed opening file with CSDP result "
         << problem_config.get_sdp_solution_name() << endl;
    exit(1);
  }
  ofstream outfile;
  outfile.open(problem_config.get_sdp_extremal_construction_name().c_str(),
               ofstream::out);
  if (!outfile.good()) {
    cerr << "Failed opening file "
         << problem_config.get_sdp_extremal_construction_name() << endl;
    exit(1);
  }
  cerr << "Saving full densities of extremal construction to file: "
       << problem_config.get_sdp_extremal_construction_name() << endl;
  process_csdp_solution(results, outfile, V, false);
  outfile.close();
}

vector<double> get_current_extremal_densities(ProblemConfig &problem_config,
                                              string file) {
  if (file == "") {
    file = problem_config.get_sdp_extremal_construction_name().c_str();
  }
  ifstream infile;
  infile.open(file, ifstream::in);
  string line;
  getline(infile, line);
  int flag_count;
  infile >> flag_count;
  infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  vector<double> densities(flag_count);
  int flag_id;
  for (int i = 0; i < flag_count; i++) {
    getline(infile, line);
    istringstream iss(line);
    iss >> flag_id;
    iss >> densities[i];
  }
  return densities;
}
