#pragma once

#include "Option.h"
#include <tuple>

/**
 * @brief Class for Asian Options.
 */
class AsianOption : public Option {
    public:
        /**
         * Constructor for the Asian Option class.
         * 
         * @param optionType Type of the option (Call/Put).
         * @param S0_ Initial underlying asset price.
         * @param K_ Strike price.
         * @param T_ Maturity.
         * @param r_ Continuously compounded risk free interest rate.
         * @param q_ Continous yield of the asset.
         * @param sigma_ Volatility per year for the underlying asset.
         * @param m_ The number of steps.
         */
        AsianOption(OptionType optionType_, double S0_, double K_, double T_, double r_, double q_, double sigma_, int m_);

        /**
         * Destructor for the Asian Option class. 
         */
        ~AsianOption();

        /**
         * Compute the price of the option.
         */
        double computePrice(Method method) override;


    private:
        int m;   /*> The number of steps. */
        double S_avg;   /*> Average price of S over the price path. */

        /**
         * Compute the underlying asset price path.
         */
        void computePricePath() override;

        /**
         * Compute the payoff of the geometric option.
         */
        double computeGeometricPayoff();

        /**
         * Private helper function to moment-match log normal distribution to the S_path.
         * 
         * @return Pair of <µ, σ^2>
         */
        pair<double, double> momentMatchLogNormal();

        /**
         * Private helper function to compute the mean of the vector.
         * 
         * @param v The vector to compute the mean of.
         * @return The mean of the vector.
         */
        double vectorMean(vector<double> v);

        /**
         * Private helper function to compute the variance of the vector.
         * 
         * @param v The vector to compute the variance of.
         * @param mean The mean of the vector.
         * @return The variance of the vector.
         */
        double vectorVariance(vector<double> v, double mean);
};