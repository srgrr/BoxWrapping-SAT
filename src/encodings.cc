#include "encodings.h"

namespace encodings {
  int at_most_one_n2(vi& lits, vvi& formula, int num_vars) {
    int n = int(lits.size());
    for(int i = 0; i < n; ++i) {
      for(int j = i + 1; j < n; ++j) {
        formula.push_back({-lits[i], -lits[j]});
      }
    }
    return 0;
  }

  int at_most_one_log(vi& lits, vvi& formula, int num_vars) {
    int n = int(lits.size());
    //TODO: this __builtin stuff is gcc dependent i think
    int num_bits = 8 * sizeof(int) - __builtin_clz(n);
    map< int, int > index;
    int cur = 0;
    for(int lit : lits) {
      if(!index.count(lit)) {
        index[lit] = cur++;
      }
    }
    for(int i = 0; i < n; ++i) {
      int ix = index[lits[i]];
      for(int j = 0; j < num_bits; ++j) {
        int sgn = (ix & (1<<j)) ? 1 : - 1;
        vi to_add = {
            -lits[i], sgn * (j + num_vars + 1)
        };
        formula.push_back(to_add);
      }
    }
    return num_bits;
  }

  int _at_most_one_heule(vi& lits, vvi& formula, int num_vars) {
    int n = int(lits.size());
    if(n <= 3) {
      return at_most_one_n2(lits, formula, num_vars);
    }
    vi lp = {lits[n - 1], lits[n - 2], num_vars + 1};
    int lhs = at_most_one_n2(lp, formula, num_vars + 1);
    lits.pop_back();
    lits.pop_back();
    lits.push_back(-(num_vars + 1));
    return 1 + lhs + _at_most_one_heule(lits, formula, num_vars + 1);
  }

  int at_most_one_heule(vi& lits, vvi& formula, int num_vars) {
    vi l = lits;
    return _at_most_one_heule(l, formula, num_vars);
  }

  int at_most_one_ladder(vi& lits, vvi& formula, int num_vars) {
    int n = int(lits.size());
    // xi => yi
    for(int i = 0; i < n; ++i) {
      formula.push_back({-lits[i], num_vars + i + 1});
    }
    // xi => !yi + 1
    for(int i = 0; i < n - 1; ++i) {
      formula.push_back({-lits[i], -(num_vars + i + 2)});
    }
    // yi => yi - 1
    for(int i = 1; i < n; ++i) {
      formula.push_back({-(num_vars + i + 1), num_vars + i});
    }
    return n;
  }

  // was this really necessary? xD
  int at_least_one(vi& lits, vvi& formula, int num_vars) {
    formula.push_back(lits);
    return 0;
  }

};
