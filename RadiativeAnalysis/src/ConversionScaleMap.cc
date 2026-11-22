#include "BsToMuMuGammaAnalysis/RadiativeAnalysis/interface/ConversionScaleMap.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "nlohmann/json.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>

namespace bmmg {

  namespace {
    // bin of x, clamped to [0, n_bins - 1]
    int clampedBin(const std::vector<double>& edges, double x) {
      const int n = static_cast<int>(edges.size()) - 1;
      const int i = static_cast<int>(std::upper_bound(edges.begin(), edges.end(), x) - edges.begin()) - 1;
      return std::clamp(i, 0, n - 1);
    }

    std::vector<double> edgesOf(const nlohmann::json& var, const std::string& path) {
      std::vector<double> e = var.at("edges").get<std::vector<double>>();
      if (e.size() < 2 || !std::is_sorted(e.begin(), e.end()))
        throw cms::Exception("ConversionScaleMap") << path << ": bad edges of " << var.at("name");
      return e;
    }
  }  // namespace

  ConversionScaleMap::ConversionScaleMap(const std::string& path) : path_(path) {
    std::ifstream in(path);
    if (!in) throw cms::Exception("ConversionScaleMap") << "cannot open " << path;
    nlohmann::json j;
    try {
      in >> j;
      const auto& vars = j.at("variables");
      if (vars.size() != 2)
        throw cms::Exception("ConversionScaleMap") << path << ": expected 2 variables, found " << vars.size();
      // the variables of conv_scale.py: pt (expr pt_est) and abseta (expr abs(conv_eta)), in either order
      int iPt = -1, iEta = -1;
      for (int k = 0; k < 2; ++k) {
        const std::string expr = vars[k].at("expr").get<std::string>();
        if (expr == "pt_est")
          iPt = k;
        else if (expr == "abs(conv_eta)")
          iEta = k;
      }
      if (iPt < 0 || iEta < 0)
        throw cms::Exception("ConversionScaleMap")
            << path << ": the map variables must be pt_est and abs(conv_eta), found " << vars[0].at("expr") << ", "
            << vars[1].at("expr");
      ptEdges_ = edgesOf(vars[iPt], path);
      etaEdges_ = edgesOf(vars[iEta], path);
      const size_t nPt = ptEdges_.size() - 1, nEta = etaEdges_.size() - 1;
      const auto& s = j.at("scale");
      scale_.assign(nPt * nEta, 1.);
      for (size_t ip = 0; ip < nPt; ++ip)
        for (size_t ie = 0; ie < nEta; ++ie) {
          const double v = (iPt == 0 ? s.at(ip).at(ie) : s.at(ie).at(ip)).get<double>();
          if (!(v > 0.5 && v < 1.5))
            throw cms::Exception("ConversionScaleMap") << path << ": implausible scale " << v << " in bin (" << ip
                                                       << ", " << ie << ")";
          scale_[ip * nEta + ie] = v;
        }
    } catch (const nlohmann::json::exception& e) {
      throw cms::Exception("ConversionScaleMap") << path << ": " << e.what();
    }
  }

  double ConversionScaleMap::scaleAt(double ptTrue, double absEta) const {
    const size_t nEta = etaEdges_.size() - 1;
    return scale_[clampedBin(ptEdges_, ptTrue) * nEta + clampedBin(etaEdges_, std::abs(absEta))];
  }

  double ConversionScaleMap::scale(double ptReco, double eta) const {
    const double s0 = scaleAt(ptReco, eta);
    return scaleAt(ptReco / s0, eta);
  }

}  // namespace bmmg
