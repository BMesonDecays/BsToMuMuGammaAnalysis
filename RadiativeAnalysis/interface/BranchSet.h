#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_BranchSet_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_BranchSet_h

// Storage and branches of a TTree, declared once per variable:
//   BranchSet b(tree);
//   b.bookScalar<int>("evt_run", -1);   b.bookVector<float>("mu_pt");
//   b.reset();  b.scalar<int>("evt_run") = 1;  b.vec<float>("mu_pt").push_back(5.);  tree->Fill();
// Accessing a variable that was not booked throws, so a typo cannot create a silent extra variable.
// Supported types: int, unsigned int, unsigned long long, float, double (scalars and std::vector).

#include "TTree.h"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace bmmg {

  class BranchSet {
  public:
    explicit BranchSet(TTree* tree) : tree_(tree) {}

    template <typename T>
    void bookScalar(const std::string& name, T defaultValue) {
      auto& s = store<T>();
      if (s.scalars.count(name) || s.vectors.count(name)) throw std::logic_error("BranchSet: booked twice: " + name);
      auto value = std::make_unique<T>(defaultValue);
      tree_->Branch(name.c_str(), value.get());
      s.defaults[name] = defaultValue;
      s.scalars[name] = std::move(value);
    }

    template <typename T>
    void bookVector(const std::string& name) {
      auto& s = store<T>();
      if (s.scalars.count(name) || s.vectors.count(name)) throw std::logic_error("BranchSet: booked twice: " + name);
      auto value = std::make_unique<std::vector<T>>();
      tree_->Branch(name.c_str(), value.get());
      s.vectors[name] = std::move(value);
    }

    template <typename T>
    T& scalar(const std::string& name) {
      auto& s = store<T>();
      auto it = s.scalars.find(name);
      if (it == s.scalars.end()) throw std::logic_error("BranchSet: scalar not booked: " + name);
      return *it->second;
    }

    template <typename T>
    std::vector<T>& vec(const std::string& name) {
      auto& s = store<T>();
      auto it = s.vectors.find(name);
      if (it == s.vectors.end()) throw std::logic_error("BranchSet: vector not booked: " + name);
      return *it->second;
    }

    // scalars to their defaults, vectors cleared
    void reset() {
      std::apply([](auto&... s) { (s.reset(), ...); }, stores_);
    }

  private:
    template <typename T>
    struct Store {
      std::map<std::string, std::unique_ptr<T>> scalars;
      std::map<std::string, T> defaults;
      std::map<std::string, std::unique_ptr<std::vector<T>>> vectors;
      void reset() {
        for (auto& [name, value] : scalars) *value = defaults[name];
        for (auto& [name, value] : vectors) value->clear();
      }
    };

    template <typename T>
    Store<T>& store() {
      return std::get<Store<T>>(stores_);
    }

    TTree* tree_;
    std::tuple<Store<int>, Store<unsigned int>, Store<unsigned long long>, Store<float>, Store<double>> stores_;
  };

}  // namespace bmmg

#endif
