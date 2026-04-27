#ifndef BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonEnergyModifier_H
#define BsToMuMuGammaAnalysis_RadiativeAnalysis_PhotonEnergyModifier_H


namespace PhotonEnergyModifier{

    double getSzymonsModPhotonEnergy (double initE, double eta = 0.0)
    {
        double a0 = 0.599366;  // intercept
        double a1 = 1.02408;  // slope

        double val = initE - a0;
        val /= a1;
        
        return val;
    }

    double getModPhotonEnergy (double initE, double eta)
    {
        double a0 = 0.0;  // intercept
        double a1 = 0.0;  // slope

        // EB
        if (std::fabs(eta) < 1.479)
        {
            if (initE < 20.0){
                a0 = 0.806;
                a1 = 0.980;
            }
            else if (initE < 35.0){
                a0 = -0.670;
                a1 = 1.035;
            }
            else {  // no modification
                a0 = 0.0;
                a1 = 1.0;
            }
        }

        // EES
        else if (std::fabs(eta) > 1.65 && std::fabs(eta) < 2.5)
        {
            if (initE < 50.0){
                a0 = 3.09;
                a1 = 0.952;
            }
            else if (initE < 90.0){
                a0 = 2.7;
                a1 = 0.972;
            }
        }


        if (a1 == 0.0)  return 0.0; // eta or initE out of range

        double val = initE - a0;
        val /= a1;
        
        return val;
    }



}
#endif