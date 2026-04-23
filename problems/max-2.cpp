#include "balanced-bipartition-utils.hpp"
#include "config.hpp"
#include "flag-calculator.hpp"
#include "flag.hpp"
#include "option-parser.hpp"
#include "problem.hpp"
#include "solver.hpp"
#include "utils.hpp"
#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>

constexpr double BOUND = 0.0673;
constexpr double HIGH_DEGREE_EDGES_CUTOFF = 0.02;

struct VertexNeighborhood {
  vector<flag_coeff> disconnected;
  vector<flag_coeff> connected_low_degree;
  vector<flag_coeff> connected_high_degree;
};

VertexNeighborhood prepare_vertex_neighborhood(int color,
                                               vector<int> colors_low_degree,
                                               vector<int> colors_high_degree) {
  vector<flag_coeff> disconnected;
  vector<flag_coeff> connected_low_degree;
  vector<flag_coeff> connected_high_degree;
  for (int low_degree_color : colors_low_degree) {
    disconnected.push_back(flag_coeff("2 1  " + to_string(color) + " " +
                                      to_string(low_degree_color) + "  1"));

    for (int edge_color : get_allowed_edge_colors()) {
      if (edge_color == 1) {
        continue;
      }
      flag_coeff curr_edge("2 1  " + to_string(color) + " " +
                           to_string(low_degree_color) + "  " +
                           to_string(edge_color));
      if (is_flag_forbidden(curr_edge.g)) {
        continue;
      }
      connected_low_degree.push_back(curr_edge);
    }
  }

  for (int high_degree_color : colors_high_degree) {
    disconnected.push_back(flag_coeff("2 1  " + to_string(color) + " " +
                                      to_string(high_degree_color) + "  1"));

    for (int edge_color : get_allowed_edge_colors()) {
      if (edge_color == 1) {
        continue;
      }
      flag_coeff curr_edge("2 1  " + to_string(color) + " " +
                           to_string(high_degree_color) + "  " +
                           to_string(edge_color));
      if (is_flag_forbidden(curr_edge.g)) {
        continue;
      }
      connected_high_degree.push_back(curr_edge);
    }
  }

  return VertexNeighborhood{disconnected, connected_low_degree,
                            connected_high_degree};
}

CutInfo<1, 5> cut_vertex_high_degree(int color, vector<int> colors_low_degree,
                                     vector<int> colors_high_degree,
                                     bool low_degree_vertices_more_prevalent,
                                     double bound) {

  auto neighborhood =
      prepare_vertex_neighborhood(color, colors_low_degree, colors_high_degree);

  if (low_degree_vertices_more_prevalent) {
    return prepare_cut_halves<1>(
        neighborhood.disconnected, neighborhood.connected_high_degree,
        neighborhood.connected_low_degree, {}, bound, 1.);
  }
  return prepare_cut_halves<1>(
      neighborhood.disconnected, neighborhood.connected_low_degree,
      neighborhood.connected_high_degree, {}, bound, 1.);
}

CutInfo<1, 5> cut_vertex_low_degree(int color, vector<int> colors_low_degree,
                                    vector<int> colors_high_degree,
                                    double bound) {
  auto neighborhood =
      prepare_vertex_neighborhood(color, colors_low_degree, colors_high_degree);

  return prepare_cut_halves<1>(
      join_vectors({neighborhood.connected_low_degree,
                    neighborhood.connected_high_degree}),
      {}, neighborhood.disconnected, {}, bound, 1.);
}

CutInfo<2, 5> cut_on_high_degree_edge_simplified(int edge_color,
                                                 vector<int> colors_low_degree,
                                                 vector<int> colors_high_degree,
                                                 double bound) {
  vector<flag_coeff> left_only;
  vector<flag_coeff> right_only;
  vector<flag_coeff> both;
  vector<flag_coeff> neither;

  for (int low_degree_color : colors_low_degree) {
    left_only.push_back(flag_coeff("3 2  2 2 " + to_string(low_degree_color) +
                                   "  " + to_string(edge_color) + " 2  1"));
    right_only.push_back(flag_coeff("3 2  2 2 " + to_string(low_degree_color) +
                                    "  " + to_string(edge_color) + " 1  2"));
    both.push_back(flag_coeff("3 2  2 2 " + to_string(low_degree_color) + "  " +
                              to_string(edge_color) + " 2  2"));
    neither.push_back(flag_coeff("3 2  2 2 " + to_string(low_degree_color) +
                                     "  " + to_string(edge_color) + " 1  1",
                                 0.5));
  }

  for (int high_degree_color : colors_high_degree) {
    left_only.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                                   "  " + to_string(edge_color) + " 3  1"));
    left_only.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                                   "  " + to_string(edge_color) + " -3  1"));
    right_only.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                                    "  " + to_string(edge_color) + " 1  3"));
    right_only.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                                    "  " + to_string(edge_color) + " 1 -3"));
    both.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                              "  " + to_string(edge_color) + " 3  3"));
    both.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                              "  " + to_string(edge_color) + " 3  -3"));
    both.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                              "  " + to_string(edge_color) + " -3  3"));
    both.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                              "  " + to_string(edge_color) + " -3  -3"));
    neither.push_back(flag_coeff("3 2  2 2 " + to_string(high_degree_color) +
                                     "  " + to_string(edge_color) + " 1  1",
                                 0.5));
  }

  return prepare_cut_halves_simplified<2>(join_vectors({left_only, neither}),
                                          join_vectors({right_only, neither}),
                                          {both}, {}, bound, 1.);
}

CutInfo<2, 6> cut_on_low_degree_edge(int edge_color,
                                     vector<int> colors_low_degree,
                                     vector<int> colors_high_degree,
                                     double bound) {
  vector<flag_coeff> left_only;
  vector<flag_coeff> right_only;
  vector<flag_coeff> both;
  vector<flag_coeff> neither;

  for (int low_degree_color : colors_low_degree) {
    left_only.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) +
                                   "  " + to_string(edge_color) + " 3  1"));
    left_only.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) +
                                   "  " + to_string(edge_color) + " -3  1"));
    right_only.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) +
                                    "  " + to_string(edge_color) + " 1  3"));
    right_only.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) +
                                    "  " + to_string(edge_color) + " 1 -3"));
    both.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) + "  " +
                              to_string(edge_color) + " 3  3"));
    both.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) + "  " +
                              to_string(edge_color) + " 3  -3"));
    both.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) + "  " +
                              to_string(edge_color) + " -3  3"));
    both.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) + "  " +
                              to_string(edge_color) + " -3  -3"));
    neither.push_back(flag_coeff("3 2  1 1 " + to_string(low_degree_color) +
                                 "  " + to_string(edge_color) + " 1  1"));
  }

  for (int high_degree_color : colors_high_degree) {
    left_only.push_back(flag_coeff("3 2  1 1 " + to_string(high_degree_color) +
                                   "  " + to_string(edge_color) + " 2  1"));
    right_only.push_back(flag_coeff("3 2  1 1 " + to_string(high_degree_color) +
                                    "  " + to_string(edge_color) + " 1  2"));
    both.push_back(flag_coeff("3 2  1 1 " + to_string(high_degree_color) +
                              "  " + to_string(edge_color) + " 2  2"));
    neither.push_back(flag_coeff("3 2  1 1 " + to_string(high_degree_color) +
                                 "  " + to_string(edge_color) + " 1  1"));
  }

  return prepare_cut_halves<2>(left_only, right_only,
                               join_vectors({both, neither}), {}, bound, 1.);
}

CutInfo<0, 4> degree_cut(const vector<int> &vertices_less,
                         const vector<int> &vertices_more, double bound) {
  vector<flag_coeff> left;
  vector<flag_coeff> random;
  for (int color : vertices_less) {
    left.push_back(flag_coeff("1 0  " + to_string(color)));
  }
  for (int color : vertices_more) {
    random.push_back(flag_coeff("1 0  " + to_string(color)));
  }
  FlagVector<0, 1> sum;
  FlagVector<0, 1> ones(flag(), 1.);
  for (auto flag : left) {
    assert(flag.g.m_vertices == 1);
    sum += flag;
  }
  for (auto flag : random) {
    assert(flag.g.m_vertices == 1);
    sum += flag;
  }
  assert(sum == ones);
  auto both_left = get_edges_inside<0>(flag(), left);
  auto neither = get_edges_inside<0>(flag(), random);
  auto one_left = get_edges_between<0>(flag(), left, random);
  auto single_left = FlagVector<0, 1>::from_vector(flag(), left);
  auto space_left = 1. / 2 - single_left;
  FlagVector<0, 1> space_right(flag(), 1. / 2);
  auto random_vector = FlagVector<0, 1>::from_vector(flag(), random);
  assert(random_vector == space_left + space_right);

  auto left_side = both_left * random_vector * random_vector;
  left_side += one_left * space_left * random_vector;
  left_side += neither * space_left * space_left;

  auto right_side = neither * space_right * space_right;
  FlagVector<0, 2> lower_bound(flag(), 2 * bound);
  // show_vector(random_vector, "random");
  // show_vector(neither, "neither");
  // show_vector(space_right, "space right");
  // show_vector(lower_bound, "lower bound");
  return CutInfo<0, 4>{
      left_side, right_side, {}, lower_bound * random_vector * random_vector};
}

FlagVector<0, 1> vertex_color_constraint(vector<int> vertices_more) {
  FlagVector<0, 1> result;
  for (int color : vertices_more) {
    result += flag_coeff("1 0  " + to_string(color));
  }
  return result - 1. / 2;
}

FlagVector<0, 2> get_high_degree_edge_assumption(vector<int> colors_high_degree,
                                                 bool greater) {
  vector<flag_coeff> vertices;
  for (int color : colors_high_degree) {
    vertices.push_back(flag_coeff("1 0  " + to_string(color)));
  }
  auto high_degree_edge_vector = get_edges_inside<0>(flag(), vertices);

  if (greater) {
    return high_degree_edge_vector - HIGH_DEGREE_EDGES_CUTOFF;
  }
  return HIGH_DEGREE_EDGES_CUTOFF - high_degree_edge_vector;
}

void add_degree_assumptions(Problem &problem, vector<int> colors_low_degree,
                            vector<int> colors_high_degree) {
  // If we don't constrain the maximum size after multiplication, we will end up
  // with a lot of linear constraints that don't bring much.
  constexpr int max_mult_size = 5;
  for (int low_degree_vertex : colors_low_degree) {
    auto neighborhood = prepare_vertex_neighborhood(
        low_degree_vertex, colors_low_degree, colors_high_degree);
    problem.add_constraint<1, 2, max_mult_size>(
        1. / 2 - FlagVector<1, 2>::from_vector(
                     flag("1 1  " + to_string(low_degree_vertex)),
                     join_vectors({neighborhood.connected_low_degree,
                                   neighborhood.connected_high_degree})));
  }
  for (int high_degree_vertex : colors_high_degree) {
    auto neighborhood = prepare_vertex_neighborhood(
        high_degree_vertex, colors_low_degree, colors_high_degree);
    problem.add_constraint<1, 2, max_mult_size>(
        FlagVector<1, 2>::from_vector(
            flag("1 1  " + to_string(high_degree_vertex)),
            join_vectors({neighborhood.connected_low_degree,
                          neighborhood.connected_high_degree})) -
        1. / 2);
  }
}

int main(int argc, char *argv[]) {
  parse_options(argc, argv, ProblemConfig::instance());
  ProblemConfig::instance().data_directory = "max-2";
  ProblemConfig::instance().csdp_binary = "csdp";
  ProblemConfig::instance().sdpa_binary = "sdpa";
  ProblemConfig::instance().upper_bound = false;
  ProblemConfig::instance().forbidden_subgraphs_directory = "max-2-forbidden";
  ProblemConfig::instance().sdp_directory =
      "max-2-case-" + to_string(ProblemConfig::instance().case_number);
  Problem problem(2 * 2 * 2 * 2 * 2 * 3 * 3 * 5 * 5 * 5 * 5 * 5, true);

  // We use colors on vertices to encode information about them.
  // First, they are used to distinguish vertices based on their degree in
  // comparison to n / 2. Second, they are used to mark whether a cut based on a
  // vertex has more edges on the left or on the right side.

  cerr << "Case number: " << ProblemConfig::instance().case_number << endl;
  cerr << "SDP directory: " << ProblemConfig::instance().sdp_directory << endl;
  cerr << "Bound: " << BOUND << endl;
  bool high_degree_edges_present = true;
  bool low_degree_vertices_more_prevalent = true;
  bool more_edges_half_both_kind = true;
  bool use_cuts_on_low_degree_edge = false;
  bool use_cuts_on_high_degree_edge = false;
  bool use_cuts_on_low_degree_vertices = false;

  if (ProblemConfig::instance().case_number == 1) {
    use_cuts_on_high_degree_edge = true;
    use_cuts_on_low_degree_vertices = true;
  }
  if (ProblemConfig::instance().case_number == 2) {
    use_cuts_on_high_degree_edge = true;
    use_cuts_on_low_degree_vertices = true;
    more_edges_half_both_kind = false;
  }
  if (ProblemConfig::instance().case_number == 3) {
    use_cuts_on_high_degree_edge = true;
    use_cuts_on_low_degree_vertices = true;
    low_degree_vertices_more_prevalent = false;
  }
  if (ProblemConfig::instance().case_number == 4) {
    use_cuts_on_high_degree_edge = true;
    use_cuts_on_low_degree_vertices = true;
    low_degree_vertices_more_prevalent = false;
    more_edges_half_both_kind = false;
  }
  if (ProblemConfig::instance().case_number == 5) {
    high_degree_edges_present = false;
    use_cuts_on_low_degree_edge = true;
  }
  if (ProblemConfig::instance().case_number == 6) {
    high_degree_edges_present = false;
    use_cuts_on_low_degree_edge = true;
    more_edges_half_both_kind = false;
  }
  if (ProblemConfig::instance().case_number == 7) {
    high_degree_edges_present = false;
    use_cuts_on_low_degree_edge = true;
    low_degree_vertices_more_prevalent = false;
  }
  if (ProblemConfig::instance().case_number == 8) {
    high_degree_edges_present = false;
    use_cuts_on_low_degree_edge = true;
    low_degree_vertices_more_prevalent = false;
    more_edges_half_both_kind = false;
  }

  vector<int> low_degree_vertices{1};
  vector<int> high_degree_vertices{2};
  if (use_cuts_on_low_degree_vertices) {
    cerr << "Adding extra color for low degree vertices..." << endl;
    // This color will be used for marking which side of the cut on low degree
    // vertex has more edges in it.
    low_degree_vertices.push_back(3);
  }

  vector<flag_coeff> colors;
  for (int color : join_vectors({low_degree_vertices, high_degree_vertices})) {
    colors.push_back(flag_coeff("1 0  " + to_string(color)));
  }
  auto edge_vector = get_edges_inside<0>(flag(), colors);
  auto objective = 1. - edge_vector;
  problem.add_objective(objective);

  add_degree_assumptions(problem, low_degree_vertices, high_degree_vertices);

  problem.add_constraint(get_high_degree_edge_assumption(
      high_degree_vertices, high_degree_edges_present));

  vector<int> vertices_more, vertices_less;
  if (low_degree_vertices_more_prevalent) {
    cerr << "Assuming there's more vertices of low degree." << endl;
    vertices_more = low_degree_vertices;
    vertices_less = high_degree_vertices;
  } else {
    cerr << "Assuming there's more vertices of high degree." << endl;
    vertices_more = high_degree_vertices;
    vertices_less = low_degree_vertices;
  }
  problem.add_constraint(vertex_color_constraint(vertices_more));

  auto cut_on_degrees = degree_cut(vertices_less, vertices_more, BOUND);
  if (more_edges_half_both_kind) {
    cerr << "Assuming that in the cut degree there is more edges in the half "
            "with vertices of both kind."
         << endl;
    problem.add_constraint(cut_on_degrees.left_side -
                           cut_on_degrees.right_side);
    problem.add_constraint(cut_on_degrees.left_side -
                           cut_on_degrees.lower_bound);
  } else {
    cerr << "Assuming that in the cut degree there is more edges in the half "
            "with vertices of the more prevalent degree."
         << endl;
    problem.add_constraint(cut_on_degrees.right_side -
                           cut_on_degrees.left_side);
    problem.add_constraint(cut_on_degrees.right_side -
                           cut_on_degrees.lower_bound);
  }

  // Random cut.
  problem.add_constraint(edge_vector - 8 * BOUND);

  auto high_degree_vertex_cut =
      cut_vertex_high_degree(2, low_degree_vertices, high_degree_vertices,
                             low_degree_vertices_more_prevalent, BOUND);
  // We don't need to care about the other case, because right side is a
  // subset of a triangle-free graph, which allows us to get a very strong
  // bound even with a simple application of Mantel's theorem.
  problem.add_constraint(high_degree_vertex_cut.left_side -
                         high_degree_vertex_cut.right_side);
  problem.add_constraint(high_degree_vertex_cut.left_side -
                         high_degree_vertex_cut.lower_bound);

  if (use_cuts_on_low_degree_vertices) {
    cerr << "Using cuts on low degree vertices..." << endl;
    auto low_degree_vertex_cut_left = cut_vertex_low_degree(
        1, low_degree_vertices, high_degree_vertices, BOUND);
    problem.add_constraint(low_degree_vertex_cut_left.left_side -
                           low_degree_vertex_cut_left.right_side);
    problem.add_constraint(low_degree_vertex_cut_left.left_side -
                           low_degree_vertex_cut_left.lower_bound);

    auto low_degree_vertex_cut_right = cut_vertex_low_degree(
        3, low_degree_vertices, high_degree_vertices, BOUND);
    problem.add_constraint(low_degree_vertex_cut_right.right_side -
                           low_degree_vertex_cut_right.left_side);
    problem.add_constraint(low_degree_vertex_cut_right.right_side -
                           low_degree_vertex_cut_right.lower_bound);
  }

  if (use_cuts_on_low_degree_edge) {
    cerr << "Using cuts on low degree edge..." << endl;
    auto low_degree_edge_cut_first_orientation = cut_on_low_degree_edge(
        3, low_degree_vertices, high_degree_vertices, BOUND);
    problem.add_constraint(low_degree_edge_cut_first_orientation.left_side -
                           low_degree_edge_cut_first_orientation.right_side);
    problem.add_constraint(low_degree_edge_cut_first_orientation.left_side -
                           low_degree_edge_cut_first_orientation.lower_bound);

    auto low_degree_edge_cut_second_orientation = cut_on_low_degree_edge(
        -3, low_degree_vertices, high_degree_vertices, BOUND);
    problem.add_constraint(low_degree_edge_cut_second_orientation.right_side -
                           low_degree_edge_cut_second_orientation.left_side);
    problem.add_constraint(low_degree_edge_cut_second_orientation.right_side -
                           low_degree_edge_cut_second_orientation.lower_bound);
  }

  if (use_cuts_on_high_degree_edge) {
    cerr << "Using cuts on high degree edge..." << endl;
    auto high_degree_edge_cut_first_orientation =
        cut_on_high_degree_edge_simplified(3, low_degree_vertices,
                                           high_degree_vertices, BOUND);
    problem.add_constraint(high_degree_edge_cut_first_orientation.left_side -
                           high_degree_edge_cut_first_orientation.right_side);
    problem.add_constraint(high_degree_edge_cut_first_orientation.left_side -
                           high_degree_edge_cut_first_orientation.lower_bound);
  }

  solve_sdp_for_problem(problem.get_constraints(), problem.get_objective());
}
