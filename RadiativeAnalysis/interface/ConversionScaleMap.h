#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_ConversionScaleMap_h
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_ConversionScaleMap_h

// Energy scale of the converted photons, E_reco = s * E_true at the peak, measured with
// chi_c -> J/psi gamma(conv) by analysis/scripts/conv_scale.py (JSON ScaleMap, report.md 2026-11-22).
// The map is binned in the TRUE photon pt (pt_est of the calibration) and |eta|; the correction of a photon
// is p -> p / s. Bins without a measurement are already filled in the JSON (from the last measured pt bin).
// The lookup uses the bin values, clamped to the outer bins (as conv_scale.py with interpolate: false).

#include <string>
#include <vector>

namespace bmmg {

  class ConversionScaleMap {
  public:
    // reads the JSON file; throws cms::Exception if it is not a 2D map in pt_est and abs(conv_eta)
    explicit ConversionScaleMap(const std::string& path);

    // s in the bin of the true photon pt and |eta|
    double scaleAt(double ptTrue, double absEta) const;
    // s of a reconstructed photon: the map at pt_reco / s (one iteration: the map is binned in the true pt)
    double scale(double ptReco, double eta) const;

    const std::vector<double>& ptEdges() const { return ptEdges_; }
    const std::vector<double>& absEtaEdges() const { return etaEdges_; }
    const std::string& path() const { return path_; }

  private:
    std::string path_;
    std::vector<double> ptEdges_, etaEdges_;
    std::vector<double> scale_;   // [ipt * n_eta + ieta]
  };

}  // namespace bmmg

#endif
