#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonEnergyModifier_H
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonEnergyModifier_H


namespace PhotonEnergyModifier{


    double getModPhotonEnergy (double initE, double eta)
    {
        double a0 = 0.0;   // intercept
        double a1 = 0.0;  // slope

        // EB
        if (std::fabs(eta) < 1.479)
        {
            if (initE < 40.0){
                a0 = 0.90;
                a1 = 0.974;
            }
            else if (initE < 120.0){
                a0 = 1.0;
                a1 = 1.007;
            }
        }
        // EES
        else if (std::fabs(eta) > 1.65 && std::fabs(eta) < 2.5)
        {
            if (initE < 54.0){
                a0 = 4.0;
                a1 = 0.928;
            }
            else if (initE < 100.0){
                a0 = 2.3;
                a1 = 0.978;
            }
        }

        if (a1 == 0.0)  return 0.0;

        double val = initE - a0;
        val /= a1;
        
        return val;
    }



}
#endif