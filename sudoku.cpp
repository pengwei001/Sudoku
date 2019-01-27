#include <ios>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <fstream>
#include <assert.h>

using namespace std;
#define RED   "\x1B[31m"
#define RESET "\x1B[0m"

template<typename T>
void print_vec(const vector<T> &a)
{
  for(int i = 0; i < a.size(); ++i)
    std::cout << a[i] << " ";
  std::cout << '\n';
}

struct ConvertSudokuToSat {
  int cell[9][9];
  int result_cell[9][9];
  vector<vector<int> > clauses;

  ConvertSudokuToSat() {  }
  
  template<typename T>
  void print_mat(const T a[][9])
  {
    for(int i = 0; i < 9; ++i){
      for (int j = 0; j < 9; ++j){
	int tmp = a[i][j];
	if (cell[i][j] > 0) 
	  printf(RED "%2d " RESET, tmp);
	else
	  printf("%2d ", tmp);
      }
      std::cout << '\n';
    }
  }

  int pair_to_variable(int row_id, int col_id, int color_id) {
    // all should begin with 1
    assert(row_id>0 && row_id<=9);
    assert(col_id>0 && col_id<=9);
    assert(color_id>0 && color_id<=9);
    return 100 * row_id + 10 * col_id + color_id;
  }

  vector<int> variable_to_pair(int val) {
    // all should begin with 1
    assert (val>0 && val<1000);
    vector<int> tmp(3,0);
    int tmp_val = val;
    for (int i = 0; i<3; ++i) {
      tmp[2-i] = val % 10;
      val = val / 10;
    }
    return tmp;
  }

  void ExactlyOneOf(const vector<int> & v){
    vector<int> tmp = v;
    tmp.push_back(0);
    clauses.push_back(tmp);
    tmp.clear();

    int pick_num = 2;
    vector<bool> bitset(v.size() - pick_num, 0);
    bitset.resize(v.size(),1);
    do {
      for (int i = 0; i < v.size(); ++i)
	if (bitset[i]) tmp.push_back(-v[i]);
      tmp.push_back(0);
      clauses.push_back(tmp);
      tmp.clear();
    } while (next_permutation(bitset.begin(), bitset.end()));
  }

  void generate_CNF() {
    // cell[i][j] contains exactly one digit:
    vector<int> tmp(9,0);
    for (int i = 1; i <= 9; ++i)
      for (int j = 1; j <= 9; ++j)  {
	for (int k = 1; k <= 9; ++k)
	  tmp[k-1] = pair_to_variable(i, j, k);
	ExactlyOneOf(tmp);
      }
    
    // k appears exactly once in row i
    for (int i = 1; i <= 9; ++i)
      for (int k = 1; k <= 9; ++k) {
	for (int j = 1; j <= 9; ++j) 
          tmp[j-1] = pair_to_variable(i, j, k);
        ExactlyOneOf(tmp);
      }
    
    // k appears exactly once in column j
    for (int j = 1; j <= 9; ++j)
      for (int k = 1; k <= 9; ++k) {
	for (int i = 1; i <= 9; ++i)
          tmp[i-1] = pair_to_variable(i, j, k);
        ExactlyOneOf(tmp);
      }

    // k appears exactly once in each 3X3 block
    for (int k = 1; k <= 9; ++k){    
      for (int block_i = 0; block_i < 3; ++block_i)
	for (int block_j = 0; block_j < 3; ++block_j){
	  for (int i = block_i*3+1; i <= block_i*3+3; ++i)
	    for (int j = block_j*3+1; j <= block_j*3+3; ++j)    
	      tmp[(i-block_i*3-1)*3+(j-block_j*3-1)] 
		= pair_to_variable(i, j, k);
	  ExactlyOneOf(tmp);
	}
    }

    // if cell[i][j] already contains k
    tmp.resize(2);
    tmp[1] = 0;
    for (int i = 1; i <= 9; ++i)
      for (int j = 1; j <= 9; ++j) {
	if (cell[i-1][j-1] >= 1) {
	  tmp[0] = pair_to_variable(i, j, cell[i-1][j-1]);
	  clauses.push_back(tmp);
	}
      }
  }
  
  void write_CNF(string filename) {
    ofstream myfile;
    myfile.open(filename);
    myfile << "p cnf " << 999 << " " << clauses.size() << "\n"; 
    for (int i = 0; i < clauses.size(); ++i){ 
      for (int j = 0; j < clauses[i].size(); ++j){
	myfile << clauses[i][j] << " ";
      }
      myfile << "\n";
    }
    myfile.close();
  }

  void read_cells(string filename) {
    ifstream f(filename);
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
	f >> cell[i][j];
      }
    }
    f.close();
  }

  void read_CNF(string filename) {
    ifstream f(filename);
    string firstLine;
    getline(f, firstLine);
    if (firstLine == "UNSAT") return;
    int tmp_val = 1000;
    while (tmp_val != 0) {
      f >> tmp_val;
      if (tmp_val > 0) {
	vector<int> tmp_vec = variable_to_pair(tmp_val);
	result_cell[tmp_vec[0]-1][tmp_vec[1]-1] = tmp_vec[2];
      }
    }
    f.close();
    print_mat<int>(result_cell);
  }
};

int main() {
    ios::sync_with_stdio(false);

    ConvertSudokuToSat converter;
    converter.read_cells("test.txt");
    converter.print_mat<int>(converter.cell);
    converter.generate_CNF();
    converter.write_CNF("sudoku_input.cnf");
    std::string command = "minisat sudoku_input.cnf sudoku_output.cnf";
    system(command.c_str());
    converter.read_CNF("sudoku_output.cnf");
    return 0;
}
