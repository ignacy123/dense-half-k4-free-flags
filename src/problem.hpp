#pragma once

#include "flag-calculator.hpp"
#include <filesystem>
#include <ostream>
#include <string>

class Problem {
public:
  Problem(int scaling_factor = 1, bool ensure_integers = false)
      : scaling_factor(scaling_factor), ensure_integers(ensure_integers) {
    filesystem::remove(ProblemConfig::instance().get_cuts_name());
  };
  template <int theta, int vertices, int multiplied_size = V>
  void add_constraint(FlagVector<theta, vertices> constraint) {
    if (multiplied_size > V) {
      cerr << "Unable to multiply" << endl;
      exit(1);
    }
    if (vertices == multiplied_size) {
      add_constraint_raw(constraint);
      return;
    }
    FlagVector<theta, theta + multiplied_size - vertices> filling_flags(
        constraint.get_type());
    for (flag filling_flag : filling_flags.get_flags()) {
      FlagVector<theta, theta + multiplied_size - vertices> filling_vector(
          filling_flag);
      FlagVector<theta, multiplied_size> multiplied =
          constraint * filling_vector;
      add_constraint_raw(multiplied);
    }
  }

  template <int theta, int vertices>
  void add_objective(FlagVector<theta, vertices> raw_objective) {
    FlagVector<0, vertices> unlabeled =
        raw_objective.template project<0, vertices>();
    FlagVector<0, V> projected_up = unlabeled.template project<0, V>();
    objective = projected_up;
  }

  const vector<FlagVector<0, V>> &get_constraints() {
    cerr << "There are " << constraints.size() << " constraints." << endl;
    return constraints;
  }
  const FlagVector<0, V> &get_objective() { return objective; }

private:
  template <int theta, int vertices>
  void add_constraint_raw(FlagVector<theta, vertices> constraint) {
    auto unlabeled = constraint.template project<0, vertices>();
    auto projected = unlabeled.template project<0, V>();
    projected *= scaling_factor;
    if (ensure_integers) {
      projected.ensure_coefficients_are_integers();
    }
    constraints.push_back(projected);
  }

  vector<FlagVector<0, V>> constraints;
  FlagVector<0, V> objective;
  int scaling_factor = 1;
  bool ensure_integers = false;
};

template <int theta, int vertices>
void show_vector(FlagVector<theta, vertices> flag_vector, string name,
                 ostream &ostr = cout) {
  auto unlabeled = flag_vector.template project<0, vertices>();
  auto projected = unlabeled.template project<0, V>();
  ostr << "# " << name << endl;
  projected.debug_print(ostr);
}

template <int theta, int vertices>
void print_vector_to_file(FlagVector<theta, vertices> flag_vector,
                          string filename, string name) {
  ofstream out(filename, ios::app);
  show_vector(flag_vector, name, out);
}
