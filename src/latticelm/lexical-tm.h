#pragma once

#include <latticelm/ll-stats.h>
#include <latticelm/sentence.h>
#include <latticelm/data-lattice.h>
#include <fst/vector-fst.h>
#include <fst/float-weight.h>
#include <cmath>
#include <iostream>

namespace latticelm {

class LexicalTM {

public:

  LexicalTM(SymbolSet<std::string> f_vocab, SymbolSet<std::string> e_vocab, float alpha) {
    f_vocab_size_ = f_vocab.size();
    e_vocab_size_ = e_vocab.size();
    f_vocab_ = f_vocab;
    e_vocab_ = e_vocab;
    log_alpha_ = LogWeight(-log(alpha));

    // Zero the count vectors. Assign uniform log probabilities to the CPD

    // Doesn't matter if we're including or excluding foreign epsilons as we're conditioning on the foreign side.
    for(int i=0; i < e_vocab_size_; i++) {
      vector<fst::LogWeight> cpd_row;
      vector<fst::LogWeight> cpd_accumulator_row;
      vector<fst::LogWeight> base_dist_row;
      vector<int> counts_row;
      for(int j=0; j < f_vocab_size_; j++) {
        cpd_row.push_back(fst::LogWeight(-log(1.0/f_vocab_size_)));
        cpd_accumulator_row.push_back(fst::LogWeight::Zero());
        base_dist_row.push_back(fst::LogWeight(-log(1.0/f_vocab_size_)));
        counts_row.push_back(0);
      }
      cpd_.push_back(cpd_row);
      cpd_accumulator_.push_back(cpd_accumulator_row);
      base_dist_.push_back(base_dist_row);
      counts_.push_back(counts_row);
    }

  }

  void RemoveSample(const Alignment & align);
  void AddSample(const Alignment & align);
  Alignment CreateSample(const DataLattice & lat, LLStats & stats);
  void ResampleParameters();
  fst::VectorFst<fst::LogArc> CreateReducedTM(const DataLattice & lattice);
  fst::VectorFst<fst::LogArc> CreateReducedTM(const DataLattice & lattice, const vector<vector<fst::LogWeight>> & cpd);
  void FindBestPaths(const vector<DataLatticePtr> & lattices);
  void Normalize(int epochs);
  void Dijkstra(const fst::Fst<fst::LogArc> & lattice, fst::MutableFst<fst::LogArc> * shortest_path);
  void StringFromBacktrace(const vector<int> & prev_state, const vector<pair<int,int>> & prev_align);

  // Test methods to be moved elsewhere later
  void TestLogWeightSampling();

  // Helpful methods
  void PrintParams();
  void PrintCounts();

protected:

  // Assuming our vocab fits in an int.
  int f_vocab_size_;
  int e_vocab_size_;
  SymbolSet<std::string> f_vocab_;
  SymbolSet<std::string> e_vocab_;
  LogWeight log_alpha_; //Concentration parameter for the Dirichlet process.

  // A conditional probability disribution that will give the probability of
  // seeing a Foreign WordId given an English WordId.
  vector<vector<fst::LogWeight>> cpd_;
  // A grid that stores the sampling of the CPD at each iteration and gets
  // normalized after all the sampling is complete.
  vector<vector<fst::LogWeight>> cpd_accumulator_;
  // A uniform base distribution that the Dirichlet process will use.
  vector<vector<fst::LogWeight>> base_dist_;
  // The number of times we've seen a Foreign WordId align to an English WordId.
  vector<vector<int>> counts_;

};

}
