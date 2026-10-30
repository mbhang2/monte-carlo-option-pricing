#include "../include/AmericanOption.h"
#include "../include/AsianOption.h"
#include "../include/EuropeanOption.h"
#include "../include/MonteCarlo.h"
#include <iostream>

using namespace std;

int main() {
    // Q1
    cout << "Q1" << endl;
    MonteCarlo<EuropeanOption, OptionType, double, double, double, double, double, double> europeanMonteCarlo(256000, OptionType::PUT, 100, 100, 0.5, 0.04, 0.02, 0.2);
    
    europeanMonteCarlo.runSimulation(0.05, Method::STANDARD);
    cout << endl;

    // Q1.1
    cout << "Q1.1" << endl;
    europeanMonteCarlo.runSimulation(0.05, Method::ANTITHETIC);
    cout << endl;

    // Q2
    cout << "Q2" << endl;
    MonteCarlo<AsianOption, OptionType, double, double, double, double, double, double, int> asianMonteCarlo(1000000, OptionType::CALL, 100, 100, 1, 0.10, 0.0, 0.2, 50);

    asianMonteCarlo.runSimulation(0.05, Method::STANDARD);
    cout << endl;

    // Q2.1
    cout << "Q2.1" << endl;
    asianMonteCarlo.runSimulation(0.05, Method::CONTROLVARIATE);
    cout << endl;

    // Q2.2
    cout << "Q2.2" << endl;
    asianMonteCarlo.runSimulation(0.05, Method::MOMENTMATCHING);
    cout << endl;

    // Q3
    cout << "Q3" << endl;
    MonteCarlo<AmericanOption, OptionType, double, double, double, double, double, double, int, int, int> americanMonteCarlo(1, OptionType::PUT, 100, 100, 1.0/12.0, 0.04, 0.02, 0.2, 50, 1024000, 2);

    americanMonteCarlo.runSimulation(0.05, Method::STANDARD);
    cout << endl;

    // Q3.1
    cout << "Q3.1" << endl;
    americanMonteCarlo.runSimulation(0.05, Method::BBSR);
    cout<<endl;

    return 0;
}