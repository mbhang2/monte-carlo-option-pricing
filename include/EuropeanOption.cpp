#include "EuropeanOption.h"
#include <boost/math/distributions/normal.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

using namespace boost;
using namespace std;

EuropeanOption::EuropeanOption(OptionType optionType_, double S0_, double K_, double T_, double r_, double q_, double sigma_): 
        Option(OptionStyle::EUROPEAN, optionType_, S0_, K_, T_, r_, q_, sigma_) {}


EuropeanOption::~EuropeanOption() {}


void EuropeanOption::computePricePath() {
    // Generate random number from a normal distribution
    random_device rd{};
    mt19937 gen{rd()};
    normal_distribution<double> norm(0.0, 1.0);

    // Generate asset price using the equation from pg. 88 from Statistical Inference lecture note.
    S_path = vector<double>(1, S0);
    S_path.push_back(S0 * exp((r - q - 0.5*(sigma * sigma)) * T + sigma * sqrt(T) * norm(gen)));
}


double EuropeanOption::computePrice(Method method) {
    // Standard method.
    if (method == Method::STANDARD) {
        // Compute asset price at time T.
        computePricePath();

        // Return discounted payoff.
        double S_expiry = S_path[S_path.size() - 1];
        return exp(-r * T) * fmax(0.0, optionType == OptionType::CALL ? S_expiry - K : K - S_expiry);
    } 

    // Antithetic method.
    else if (method == Method::ANTITHETIC) {
        pair<double, double> pairs = computeAntitheticS();

        return exp(-r * T) * (fmax(0.0, optionType == OptionType::CALL ? pairs.first - K : K - pairs.first) 
                + fmax(0.0, optionType == OptionType::CALL ? pairs.second - K : K - pairs.second)) / 2;
    }

    // Other methods not implemented for European option.
    else {
        cerr << "ERROR: Unemplemented method for European option";
        return 0.0;
    }
}


double EuropeanOption::BlackScholesPrice() {
    math::normal normal_dist;
    double D1 = (log(S0/K) + (r - q + sigma * sigma / 2) * T) / (sigma * sqrt(T));
    double D2 = D1 - sigma * sqrt(T);

    if (optionType == OptionType::CALL) {
        return exp(-r * T) * (S0 * exp((r - q) * T) * cdf(normal_dist, D1) - K * cdf(normal_dist, D2));
    } else if (optionType == OptionType::PUT) {
        return exp(-r * T) * (-S0 * exp((r - q) * T) * cdf(normal_dist, -D1) + K * cdf(normal_dist, -D2));
    } else {
        cerr << "Invalid option type.";
    }

    return -1;
}


pair<double, double> EuropeanOption::computeAntitheticS() {
    // Generate random number from a normal distribution
    random_device rd{};
    mt19937 gen{rd()};
    normal_distribution<double> norm(0.0, 1.0);
    double randomValue = norm(gen);

    // Generate asset price using the equation from pg. 88 from Statistical Inference lecture note.
    return make_pair(S0 * exp((r - q - 0.5*(sigma * sigma)) * T + sigma * sqrt(T) * randomValue), S0 * exp((r - q - 0.5*(sigma * sigma)) * T - sigma * sqrt(T) * randomValue));
}