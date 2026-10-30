#include "AmericanOption.h"
#include <boost/math/distributions/normal.hpp>
#include <cmath>
#include <Eigen/Dense>
#include <iostream>
#include <random>

using namespace boost;
using namespace Eigen;
using namespace std;

AmericanOption::AmericanOption(OptionType optionType_, double S0_, double K_, double T_, double r_, double q_, double sigma_, int m_, int N_, int k_): 
        Option(OptionStyle::AMERICAN, optionType_, S0_, K_, T_, r_, q_, sigma_), m(m_), N(N_), k(k_) {}


AmericanOption::~AmericanOption() {}


double AmericanOption::computePrice(Method method) {
    // Standard method.
    if (method == Method::STANDARD) {
        // Compute asset prices at time T. 
        vector<vector<double>> S_path_matrix(N);
        vector<vector<double>> cashflow(N, vector<double>(m + 1, 0.0));
        vector<vector<double>> payoff(N);

        for (int i = 0; i < N; i++) {
            computePricePath();
            
            for (double j : S_path) {
                S_path_matrix[i].push_back(j);
                payoff[i].push_back(optionType == OptionType::CALL ? fmax(j - K, 0.0) : fmax(K - j, 0.0));
            }
        }

        // Compute cashflow at expiry
        for (int i = 0; i < N; i++) {
            cashflow[i][m] = payoff[i][m];
        }

        // Backward induction
        for (int step = m - 1; step >= 1; step--) {
            vector<double> x; // In-the-money stock prices
            vector<double> y; // Corresponding discounted cashflows from the next step.

            for (int i = 0; i < N; i++) {
                if (payoff[i][step] > 0) {
                    x.push_back(S_path_matrix[i][step]);
                    y.push_back(payoff[i][step + 1] * exp(-r * T / m));
                }
            }

            // Build the regression matrix X and vector Y for least squares
            MatrixXd X(x.size(), k + 1);
            VectorXd Y(y.size());

            for (size_t i = 0; i < x.size(); i++) {
                for (int j = 0; j <= k; j++) {
                    X(i, j) = laguerrePolynomial(j, x[i]);
                }
                Y(i) = y[i];
            }

            // Perform SVD, which is more stable, and solve least squares (https://eigen.tuxfamily.org/dox/group__LeastSquares.html)
            VectorXd theta = X.bdcSvd(ComputeThinU | ComputeThinV).solve(Y);

            // Update cashflows
            for (int i = 0; i < N; i++) {
                if (payoff[i][step] > 0) {
                    double continuation_value = 0.0;
                    for (int d = 0; d <= k; d++) {
                        continuation_value += theta(d) * pow(S_path_matrix[i][step], d);
                    }

                    // Compare continuation value with immediate exercise payoff
                    if (payoff[i][step] > continuation_value) {
                        cashflow[i][step] = payoff[i][step];    // Exercise the option

                        // Invalidate future cashflow
                        for (int j = step + 1; j <= m; j++) {
                            cashflow[i][j] = 0;
                        }
                    }
                }
            }
        }

        // Discount cashflows to present value
        double avg_PV = 0.0;
        vector<double> PV_vector;

        for (int i = 0; i < N; i++) {
            for (int step = 0; step <= m; step++) {
                if (cashflow[i][step] > 0) {
                    double PV = cashflow[i][step] * exp(-r * step * T / m);

                    avg_PV += PV;
                    PV_vector.push_back(PV);
                    break;
                }
            }
        }

        avg_PV /= N;

        // Compute variance of PVs
        double variance = 0.0;
        for (int i = 0; i < N; i++) {variance += (PV_vector[i] - avg_PV) * (PV_vector[i] - avg_PV);}
        variance /= N;

        // Standard error
        SE = sqrt(variance / N);

        return avg_PV;
    } 

    // Binomial Black-Scholes with Richardson Extrapolation (BBSR)
    else if (method == Method::BBSR) {
        // Please refer to https://business.columbia.edu/sites/default/files-efs/pubfiles/1847/rfs_new_bounds.pdf.
        double C1 = binomialBlackScholes(static_cast<int>(m/2));
        double C2 = binomialBlackScholes(m);

        return 2 * C2 - C1;
    }

    // Other methods not implemented for American option.
    else {
        cerr << "ERROR: Unemplemented method for American option";
        return 0.0;
    }
}


double AmericanOption::getSE() {return SE;}


void AmericanOption::computePricePath() {
    // Generate random number from a normal distribution
    random_device rd{};
    mt19937 gen{rd()};
    normal_distribution<double> norm(0.0, 1.0);

    // Compute the underlying asset price path
    S_path = vector<double>(1, S0);
    for (int i = 0; i < m; i++) {
        double S = S_path[i] * exp((r - q - 0.5*(sigma * sigma)) * T/m + sigma * sqrt(T/m) * norm(gen));
        S_path.push_back(S);
    }
}


double AmericanOption::laguerrePolynomial(int degree, double x) {
    switch (degree) {
        case 0: return 1.0;
        case 1: return 1 - x;
        case 2: return 0.5 * (2 - 4 * x + x * x);
        case 3: return (6 - 18 * x + 9 * x * x - x * x * x)/6;
        default: throw invalid_argument("Laguerre polynomial beyond degree 3 not implemented...");
    }
}


double AmericanOption::BlackScholesPrice(double S, double ttm) {
    math::normal normal_dist(0.0, 1.0);
    double D1 = (log(S / K) + (r - q + 0.5 * sigma * sigma) * ttm) / (sigma * sqrt(ttm));
    double D2 = D1 - sigma * sqrt(ttm);

    if (optionType == OptionType::CALL) {
        return S * exp(-q * ttm) * cdf(normal_dist, D1) - K * exp(-r * ttm) * cdf(normal_dist, D2);
    } else {
        return K * exp(-r * ttm) * cdf(normal_dist, -D2) - S * exp(-q * ttm) * cdf(normal_dist, -D2);
    }
}


double AmericanOption::binomialBlackScholes(int n) {
    // Allocate space
    vector<double> s(2 * n + 1, 0.0);
    vector<double> v(2 * n + 1, 0.0);

    // Initialize parameters
    double dT = T/n;
    double r_inv = exp(-r * dT);
    double a = exp((r - q) * dT);
    double b_sq = a * a * (exp(sigma * sigma * dT) - 1);
    double tmp = a * a + b_sq+ 1;
    double u = (tmp + sqrt(tmp * tmp - 4 * a * a))/(2*a);
    double d = 1/u;
    double p = (a - d) / (u - d);
    double p_inv = r_inv * p;
    double q_inv = r_inv * (1 - p);

    // Compute stock price
    s[n] = S0;
    for (int j = 1; j <= n; j++) {
        s[n + j] = s[n + j - 1] * u;
        s[n - j] = s[n - j + 1] * d;
    }

    // Store option values at time index i = n
    for (int j = -n; j <= n; j += 2) {
        v[n + j] = fmax(optionType == OptionType::CALL ? fmax(s[n + j] - K, 0.0) : fmax(K - s[n + j], 0.0), BlackScholesPrice(s[n + j], dT));
    }

    // Backward induction
    for (int step = n - 1; step >= 0; step--) {
        for (int j = -step; j <= step; j += 2) {
            v[n + j] = fmax(p_inv * v[n + j + 1] + q_inv * v[n + j - 1], optionType == OptionType::CALL ? fmax(s[n + j] - K, 0.0) : fmax(K - s[n + j], 0.0));
        }
    }

    return v[n];
}