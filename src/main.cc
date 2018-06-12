#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <unordered_set>
#include "encodings.h"
using namespace std;

struct instance {

  struct box {
    int a, b;
  };

  int n, H, W, num_vars;
  vector< box > boxes;
  vector< vector< int > > formula;
  map< vector< int >, vector< int > > equivalent_boxes;

  /*
    Non extra var indices.
  */

  //            H      W      n      2
  int cijkr(int i, int j, int k, int r) {
    return 1 + i * W * n * 2 + j * n * 2 + k * 2 + r;
  }

  //           H      W      n
  int xijk(int i, int j, int k) {
    return cijkr(H - 1, W - 1, n - 1, 1) + 1 + i * W * n + j * n + k;
  }

  //      H + 1     n (row ik in report.pdf)
  int rik(int i, int k) {
    return xijk(H - 1, W - 1, n - 1) + 1 + i * n + k;
  }

  //      W + 1      n (col ik in report.pdf)
  int cik(int i, int k) {
    return rik(H, n - 1) + 1 + i * n + k;
  }

  int get_last_var() {
    return cik(W, n - 1);
  }

  void add_constraints() {
    // "ladder" stuff
    // Equation 1
    for(int k = 0; k < n; ++k) {
      for(int i = 1; i < H; ++i) {
        formula.push_back({-rik(i, k), rik(i - 1, k)});
      }
    }
    // Equation 2
    for(int k = 0; k < n; ++k) {
      for(int i = 1; i < W; ++i) {
        formula.push_back({-cik(i, k), cik(i - 1, k)});
      }
    }
    // rik or cik must be true if cijkr is true
    // Equations 3 and 4
    for(int i = 0; i < H; ++i) {
      for(int j = 0; j < W; ++j) {
        for(int k = 0; k < n; ++k) {
          for(int r = 0; r < 2; ++r) {
            formula.push_back({-cijkr(i, j, k, r), rik(i, k)});
            formula.push_back({-cijkr(i, j, k, r), -rik(i + 1, k)});
            formula.push_back({-cijkr(i, j, k, r), cik(j, k)});
            formula.push_back({-cijkr(i, j, k, r), -cik(j + 1, k)});
          }
        }
      }
    }
    // Equations 5 and 6
    for(auto& kv : equivalent_boxes) {
      vector< int >& b = kv.second;
      for(int i = 0; i < int(b.size()) - 1; ++i) {
        for(int r = 0; r < H; ++r) {
          formula.push_back({-rik(r, b[i + 1]), rik(r + 1, b[i + 1]), -rik(r + 1, b[i])});
          for(int c = 0; c < W; ++c) {
            // oh god
            formula.push_back({-rik(r, b[i + 1]), rik(r + 1, b[i + 1]), -rik(r, b[i]), rik(r + 1, b[i]), -cik(c, b[i + 1]), cik(c + 1, b[i + 1]), -cik(c + 1, b[i])});
          }
        }
      }
    }

    // Each box has exactly one upper left corner (rotated or not)
    // Equation 7
    for(int k = 0; k < n; ++k) {
      vi lits;
      for(int i = 0; i < H; ++i) {
        for(int j = 0; j < W; ++j) {
          lits.push_back(cijkr(i, j, k, 0));
          lits.push_back(cijkr(i, j, k, 1));
        }
      }
      num_vars += encodings::at_least_one(lits, formula, num_vars);
      num_vars += encodings::at_most_one_ladder(lits, formula, num_vars);
    }

    // Each position has at most one box
    // Equation 8
    for(int i = 0; i < H; ++i) {
      for(int j = 0; j < W; ++j) {
        vi lits;
        for(int k = 0; k < n; ++k) {
          lits.push_back(xijk(i, j, k));
        }
        num_vars += encodings::at_most_one_ladder(lits, formula, num_vars);
      }
    }

    // Anti-overlap constraints
    // Equations 9 and 10
    for(int k = 0; k < n; ++k) {
      int a = boxes[k].a, b = boxes[k].b;
      for(int r = 0; r < 2; ++r) {
        for(int i = 0; i < H; ++i) {
          for(int j = 0; j < W; ++j) {
            if(i + a > H || j + b > W) {
              formula.push_back({-cijkr(i, j, k, r)});
            }
            else {
              for(int ii = i; ii < i + a; ++ii) {
                for(int jj = j; jj < j + b; ++jj) {
                  formula.push_back({-cijkr(i, j, k, r), xijk(ii, jj, k)});
                }
              }
            }
          }
        }
        if(a == b) break;
        swap(a, b);
      }
    }
    // Extra constraints
    // First box goes to first quadrant
    for(int i = 0; i < H; ++i) {
      for(int j = 0; j < W; ++j) {
        if(2 * i > H || 2 * j > W) {
          formula.push_back({-cijkr(i, j, 0, 0)});
          formula.push_back({-cijkr(i, j, 0, 1)});
        }
      }
    }
    // At least (exactly) one box goes to (0, 0)
    {
      vi lits;
      for(int k = 0; k < n; ++k) {
        lits.push_back(cijkr(0, 0, k, 0));
        lits.push_back(cijkr(0, 0, k, 1));
      }
      num_vars += encodings::at_least_one(lits, formula, num_vars);
    }
    // If a box is square assume it is not rotated
    for(int k = 0; k < n; ++k) {
      if(boxes[k].a != boxes[k].b) continue;
      for(int i = 0; i < H; ++i) {
        for(int j = 0; j < W; ++j) {
          formula.push_back({-cijkr(i, j, k, 1)});
        }
      }
    }
  }

  void restrict_rows(int k) {
    for(int i = 0; i < H; ++i) {
      for(int j = 0; j < W; ++j) {
        for(int b = 0; b < n; ++b) {
          if(i >= k) {
            formula.push_back({-xijk(i, j, b)});
          }
          if(i + boxes[b].a > k) {
            formula.push_back({-cijkr(i, j, b, 0)});
          }
          if(i + boxes[b].b > k) {
            formula.push_back({-cijkr(i, j, b, 1)});
          }
        }
      }
    }
  }

  int get_min_height() {
    int area_sum = 0;
    for(int i = 0; i < n; ++i) {
      area_sum += boxes[i].a * boxes[i].b;
    }
    return (area_sum + W - 1) / W;
  }

  int get_used_space(unordered_set< int >& poslits) {
    int max_row = 0;
    for(int i = 0; i < H; ++i) {
      for(int j = 0; j < W; ++j) {
        for(int k = 0; k < n; ++k) {
          if(poslits.count(cijkr(i, j, k, 0))) {
            max_row = max(max_row, i + boxes[k].a);
          }
          else if(poslits.count(cijkr(i, j, k, 1))) {
            max_row = max(max_row, i + boxes[k].b);
          }
        }
      }
    }
    return max_row;
  }

  instance(string filein) {
    ifstream ifs(filein.c_str());
    ifs >> W;
    H = 0;
    int k, a, b;
    n = 0;
    while(ifs >> k >> a >> b) {
      n += k;
      while(k--) {
        if(a > b) {
          swap(a, b);
        }
        equivalent_boxes[{a, b}].push_back(boxes.size());
        boxes.push_back({a, b});
        H += b;
      }
    }
    num_vars = get_last_var();
    add_constraints();
  }

  void print_formula(string fileout) {
    ofstream oss(fileout.c_str());
    oss << "p cnf " << num_vars << " " << formula.size() << "\n";
    for(vi& clause : formula) {
      for(int lit : clause) {
        oss << lit << " ";
      }
      oss << 0 << "\n";
    }
    oss.close();
  }

  void print_result(unordered_set< int >& poslits, string fileout) {
    ofstream out(fileout.c_str());
    out << get_used_space(poslits) << "\n";
    for(int i = 0; i < H; ++i) {
      for(int j = 0; j < W; ++j) {
        for(int k = 0; k < n; ++k) {
          bool norm = poslits.count(cijkr(i, j, k, 0));
          bool rot  = poslits.count(cijkr(i, j, k, 1));
          if(norm || rot) {
            out << j << " " << i << " ";
            if(norm) {
              out << j + boxes[k].b - 1 << " " << i + boxes[k].a - 1 << "\n";
            }
            else {
              out << j + boxes[k].a - 1 << " " << i + boxes[k].b - 1 << "\n";
            }
          }
        }
      }
    }
    out.close();
    cerr << "------ DEBUG ------" << "\n";
    for(auto& kv : equivalent_boxes) {
      cerr << "Group {" << kv.first[0] << ", " << kv.first[1] << "}" << "\n";
      vector< int >& b = kv.second;
      for(int x : b) {
        cerr << x << " -> ";
        for(int i = 0; i < H; ++i) {
          for(int j = 0; j < W; ++j) {
            if(poslits.count(cijkr(i, j, x, 0)) || poslits.count(cijkr(i, j, x, 1))) {
              cerr << "(" << i << ", " << j << ")";
            }
          }
        }
        cerr << " | ";
      }
      cerr << "\n";
    }
  }

};

int main(int argc, char **argv) {
  assert(argc == 3);
  string filein(argv[1]),
         fileout(argv[2]);
  instance ins(filein);
  int l = ins.get_min_height(), r = ins.H;
  cout << "Search interval is [" << l << ", " << r << "]" << "\n";
  pair< int, unordered_set < int > > bst(ins.H + 1, unordered_set < int >());
  while(l <= r) {
    int h = (l + 2 * r) / 3;
    cout << "Current target height is " << h << "\n";
    instance cur = ins;
    cur.restrict_rows(h);
    cur.print_formula("tmp.cnf");
    system("lingeling < tmp.cnf > out.dimacs --thanks=fornothing");
    ifstream ifs("out.dimacs");
    unordered_set< int > poslits;
    string line;
    while(getline(ifs, line)) {
      istringstream iss(line);
      string t;
      iss >> t;
      if(t == "v") {
        int x;
        while(iss >> x) {
          if(x > 0) {
            poslits.insert(x);
          }
        }
      }
    }
    if(poslits.empty()) {
      cout << "h = " << h << " is unsat :((" << "\n";
      l = h + 1;
    }
    else {
      r = cur.get_used_space(poslits) - 1;
      if(bst.first > r + 1) {
        cout << "Found sol with cost " << r + 1 << "\n";
        bst = {r + 1, poslits};
      }
    }
  }
  if(!bst.second.empty()) {
    ins.print_result(bst.second, fileout);
  }
  else {
    cout << "no solution" << "\n";
  }
}
