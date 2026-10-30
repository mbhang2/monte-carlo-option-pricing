#pragma once

#include "Option.h"
#include <vector>

using namespace std;

/**
 * @brief Class for American Options.
 */
class AmericanOption : public Option {
    public:
        /**
         * Constructor for the American Option class.
         * 
         * @param optionType Type of the option (Call/Put).
         * @param S0 Initial underlying asset price.
         * @param K_ Strike price.
         * @param T_ Maturity.
         * @param r_ Continuously compounded risk free interest rate.
         * @param q_ Continous yield of the asset.
         * @param sigma_ Volatility per year for the underlying asset.
         * @param m_ The number of steps.
         * @param N_ The number of asset price paths generated.
         * @param k_ The number of regressors.
         */
        AmericanOption(OptionType optionType_, double S0, double K_, double T_, double r_, double q_, double sigma_, int m_, int N_, int k_);

        /**
         * Destructor for the Asian Option class. 
         */
        ~AmericanOption();

        /**
         * Compute the price of the option.
         * 
         * @param method Method of computing the price.
         */
        double computePrice(Method method) override;


        /**
         * Getter for the standard error.
         * 
         * @return Standard error.
         */
        double getSE();


    private:
        int m;      /*> The number of steps. */
        int N;      /*> The number of asset price paths generated. */
        int k;      /*> The number of regressors. */
        double SE;  /*> Standard error. */

        /**
         * Compute the underlying asset price.
         */
        void computePricePath() override;

        /**
         * Private helper function to compute Laguerre polynomials.
         * 
         * @param degree The degree of Laguerre polynomial to compute.
         * @param x Input.
         * @return Computed Laguerre polynomial.
        */
        double laguerrePolynomial(int degree, double x);

        /**
         * Private helper function to compute European option price using Black Scholes formula. 
         * 
         * @param S The underlying asset price.
         * @param ttm Time to maturity
         * @return Option price.
         */
        double BlackScholesPrice(double S, double ttm);

        /**
         * Private helper function to compute American option price using Binomial Black Scholes model.
         * 
         * @param n Number of steps.
         */
        double binomialBlackScholes(int n);
};
