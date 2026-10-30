#include "AsianOption.h"
#include <boost/math/distributions/normal.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/operations.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <random>
#include <iostream>

using namespace boost;
using namespace std;
namespace ublas = boost::numeric::ublas;

AsianOption::AsianOption(OptionType optionType_, double S0_, double K_, double T_, double r_, double q_, double sigma_, int m_): 
        Option(OptionStyle::ASIAN, optionType_, S0_, K_, T_, r_, q_, sigma_), m(m_) {}


AsianOption::~AsianOption() {}


void AsianOption::computePricePath() {
    // Generate random number from a normal distribution
    random_device rd{};
    mt19937 gen{rd()};
    normal_distribution<double> norm(0.0, 1.0);

    // Compute the average price S_bar_T
    S_path = vector<double>(1, S0);
    double average = 0.0;
    for (int i = 0; i < m; i++) {
        double S = S_path[S_path.size() - 1] * exp((r - q - 0.5*(sigma * sigma)) * T/m + sigma * sqrt(T/m) * norm(gen));
        S_path.push_back(S);
        average += S;
    }

    S_avg = average / m;
}


double AsianOption::computePrice(Method method) {
    // Standard method.
    if (method == Method::STANDARD) {
        // Compute asset price at time T.
        computePricePath();

        // Return discounted payoff.
        return exp(-r * T) * fmax(0.0, optionType == OptionType::CALL ? S_avg - K : K - S_avg);
    } 
    
    // Control variate method.
    else if (method == Method::CONTROLVARIATE) {
        // Compute Black-Scholes formula for the geometric Asian option
        math::normal normal_dist;

        double sigma_hat_T = ((sigma * sigma * T * (m + 1) * (2 * m + 1)) / (6 * m * m));
        double mu_T = (0.5 * sigma_hat_T + (r - 0.5 * sigma * sigma) * T * (m + 1) / (2 * m));
        double D1 = ((log(S0 / K) + (mu_T + 0.5 * sigma_hat_T)) / sqrt(sigma_hat_T));
        double D2 = D1 - sqrt(sigma_hat_T);

        double G_bs = optionType == OptionType::CALL ? exp(-r * T) * (S0 * exp(mu_T) * cdf(normal_dist, D1) - K * cdf(normal_dist, D2)) 
                : exp(-r * T) * (K * cdf(normal_dist, -D2) - S0 * exp(mu_T) * cdf(normal_dist, -D1));

        // Compute geometric Asian option payoff
        double payoff_geometric = computeGeometricPayoff();

        // Compute arithmetic Asian option payoff
        computePricePath();
        double payoff_arithmetic = exp(-r * T) * fmax(0.0, optionType == OptionType::CALL ? S_avg - K : K - S_avg);

        // Return the price.
        return payoff_arithmetic - (payoff_geometric - G_bs);
    }

    else if (method == Method::MOMENTMATCHING) {
        // Refer to p.g. 44 of https://dspacemainprd01.lib.uwaterloo.ca/server/api/core/bitstreams/1f7397de-e49f-4d16-ad8a-89d080dc352c/content
        computePricePath();
        auto [mu, sigma_sq] = momentMatchLogNormal();
        math::normal normal_dist;

        if (optionType == OptionType::CALL) {
            return exp(mu - r*T + 0.5 * sigma_sq) * cdf(normal_dist, (mu - log(K) + sigma_sq)/sqrt(sigma_sq)) - K * exp(-r*T) * cdf(normal_dist, (mu - log(K))/sqrt(sigma_sq));
        } else {
            return -exp(mu - r*T + 0.5 * sigma_sq) * cdf(normal_dist, (-mu + log(K) - sigma_sq)/sqrt(sigma_sq)) + K * exp(-r*T) * cdf(normal_dist, (-mu + log(K))/sqrt(sigma_sq));
        }
    }

    // Other methods not implemented for Asian option.
    else {
        cerr << "ERROR: Unemplemented method for Asian option";
        return 0.0;
    }
}


double AsianOption::computeGeometricPayoff() {
    // Generate random number from a normal distribution
    random_device rd{};
    mt19937 gen{rd()};
    normal_distribution<double> norm(0.0, 1.0);

    // Compute the average price S_bar_T
    double log_sum = 0.0;
    double geometric_S = S0;
    for (int i = 0; i < m; i++) {
        geometric_S = geometric_S * exp((r - q - 0.5*(sigma * sigma)) * T/m + sigma * sqrt(T/m) * norm(gen));
        log_sum += log(geometric_S);
    }

    geometric_S = exp(log_sum / m);

    // return the payoff
    return exp(-r * T) * fmax(0.0, optionType == OptionType::CALL ? geometric_S - K : K - geometric_S);
}


pair<double, double> AsianOption::momentMatchLogNormal() {
    // Compute mean and variance of the underlying asset price path
    double mean = vectorMean(S_path);
    double variance = vectorVariance(S_path, mean);

    // Then match µ and σ^2
    double sigma_sq = log((variance + mean * mean)/ (mean * mean));
    double mu = log(mean * mean / sqrt(variance + mean * mean));

    return {mu, sigma_sq};
}


double AsianOption::vectorMean(vector<double> v) {
    double sum = 0.0;
    for (double it : v) {sum += it;}

    return sum / v.size();
}


double AsianOption::vectorVariance(vector<double> v, double mean) {
    double variance = 0.0;
    for (double it : v) {variance += (it - mean) * (it - mean);}

    return variance / (v.size() - 1);
}
