#pragma once
#include <vector>
#include <map>
#include <iostream>
using namespace std;
using vi = vector< int >;
using vvi = vector< vi >;

/*
  All these functions return the number of new variables introduced by
  the encodings.
*/

namespace encodings {
  int at_most_one_n2(vi& lits, vvi& formula, int num_vars);

  int at_most_one_log(vi& lits, vvi& formula, int num_vars);

  int at_most_one_heule(vi& lits, vvi& formula, int num_vars);

  int at_most_one_ladder(vi& lits, vvi& formula, int num_vars);

  int at_least_one(vi& lits, vvi& formula, int num_vars);
};
