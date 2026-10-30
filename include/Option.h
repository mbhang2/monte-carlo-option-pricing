#pragma once

#include <iostream>
#include <vector>

using namespace std;

/**
 * @brief Enumerated data type for option styles.
 */
enum OptionStyle {
    AMERICAN,
    ASIAN,
    EUROPEAN
};

// Overload operator<< for OptionStyle
inline ostream& operator<<(ostream& os, const OptionStyle& style) {
    switch (style) {
        case AMERICAN: os << "American"; break;
        case ASIAN: os << "Asian"; break;
        case EUROPEAN: os << "European"; break;
        default: os << "Unknown OptionStyle"; break;
    }
    return os;
}

/**
 * @brief Enumerated data type for option types.
 */
enum OptionType {
    CALL,
    PUT
};

// Overload operator<< for OptionType
inline ostream& operator<<(ostream& os, const OptionType& type) {
    switch (type) {
        case CALL: os << "Call"; break;
        case PUT: os << "Put"; break;
        default: os << "Unknown OptionType"; break;
    }
    return os;
}

/**
 * @brief Enumerated data type for variance reduction method.
 */
enum Method {
    ANTITHETIC,
    BBSR,
    CONTROLVARIATE,
    MOMENTMATCHING,
    STANDARD
};

// Overload operator<< for Method
inline ostream& operator<<(ostream& os, const Method& method) {
    switch (method) {
        case ANTITHETIC: os << "Antithetic"; break;
        case BBSR: os << "BBSR"; break;
        case CONTROLVARIATE: os << "Control Variate"; break;
        case MOMENTMATCHING: os << "Moment Matching"; break;
        case STANDARD: os << "Standard"; break;
        default: os << "Unknown Method"; break;
    }
    return os;
}


/**
 * @brief Abstract class for options.
 */
class Option {
    public:
        /**
         * Constructor for the abstract option class.
         * 
         * @param optionStyle_ Style of the option (American/Asian/European)
         * @param optionType_ Type of the option (Call/Put)
         * @param S0 Initial underlying asset price.
         * @param K_ Strike price.
         * @param T_ Maturity.
         * @param r_ Continuously compounded risk free interest rate.
         * @param q_ Continous yield of the asset.
         * @param sigma_ Volatility per year for the underlying asset.
         */
        Option(OptionStyle optionStyle_, OptionType optionType_, double S0_, double K_, double T_, double r_, double q_, double sigma_):
                optionStyle(optionStyle_), optionType(optionType_), S0(S0_), K(K_), T(T_), r(r_), q(q_), sigma(sigma_) {};


        /**
         * Default virtual destructor. 
         */
        virtual ~Option() = default;


        /**
         * Pure virtual function to compute the price of the option contract.
         */
        virtual double computePrice(Method method) = 0;


    protected:
        OptionStyle optionStyle;    /*> Option style. */
        OptionType optionType;      /*> Option type. */
        double S0;                  /*> Underlying asset price. */
        double K;                   /*> Strike price. */
        double T;                   /*> Maturity. */
        double r;                   /*> Risk free interest rate with continuous compounding. */
        double q;                   /*> Continuous yield. */
        double sigma;               /*> Volatility. */
        vector<double> S_path;      /*> Vector of underlying asset price path. */

        /**
         * Pure virtual helper function to compute the underlying asset price path.
         */
        virtual void computePricePath() = 0;
};