#pragma once

#include "Option.h"
#include <utility>

using namespace std;

/**
 * @brief Class for European Options.
 */
class EuropeanOption : public Option {
    public:
        /**
         * Constructor for the European Option class.
         * 
         * @param optionType Type of the option (Call/Put).
         * @param S0_ Initial underlying asset price.
         * @param K_ Strike price.
         * @param T_ Maturity.
         * @param r_ Continuously compounded risk free interest rate.
         * @param q_ Continous yield of the asset.
         * @param sigma_ Volatility per year for the underlying asset.
         */
        EuropeanOption(OptionType optionType_, double S0_, double K_, double T_, double r_, double q_, double sigma_);

        /**
         * Destructor for the European Option class. 
         */
        ~EuropeanOption();

        /**
         * Compute the price of the option.
         * 
         * @param method The variance reduction method.
         */
        double computePrice(Method method) override;

        /**
         * Compute the option price using the Black Scholes formula.
         * 
         * @return Computed option price.
         */
        double BlackScholesPrice();

    private:
        /**
         * Compute the underlying asset price path.
         */
        void computePricePath() override;

        /**
         * Compute the underlying asset price antithetically.
         * 
         * @return Pair of replicated underlying asset price.
         */
        pair<double, double> computeAntitheticS();
};