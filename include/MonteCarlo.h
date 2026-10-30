#pragma once

#include "Option.h"
#include <boost/math/distributions/normal.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

using namespace boost;
using namespace std;

/**
 * @brief MonteCarlo class for simulating option values using Monte Carlo methods.
 * 
 * @tparam T The style of option (American/Asian/European) that is being simulated in the Monte Carlo methods.
 * @tparam Args Types of arguments that are passed to the constructor of T.
 */
template <typename T, typename... Args>
class MonteCarlo {
public:
    /**
     * Constructor for the Monte Carlo Simulation class.
     * 
     * @param N_ sample size (number of simulations).
     * @param args_ arguments for the options class constructor.
     */
    MonteCarlo(long N_, Args&&... args_)
        : N(N_), args(make_tuple(std::forward<Args>(args_)...)) {
            cout << "====================" << endl;
            cout << (std::is_same<T, AmericanOption>::value ? "American " : (std::is_same<T, AsianOption>::value ? 
                    "Asian " : (std::is_same<T, EuropeanOption>::value ? "European " : "Unknown "))) << std::get<2>(args) << " " << std::get<0>(args) << endl;
            cout << "S0: " << std::get<1>(args) << endl;
            cout << "T: " << std::get<3>(args) << endl;
            cout << "r: " << std::get<4>(args) << endl;
            cout << "q: " << std::get<5>(args) << endl;
            cout << "σ: " << std::get<6>(args) << endl;

            if constexpr (sizeof...(Args) > 7) {
                cout << "m: " << std::get<7>(args) << endl;
            }

            if constexpr (sizeof...(Args) > 9) {
                cout << "N: " << std::get<8>(args) << endl;
                cout << "Regressor degree: " << std::get<9>(args) << endl;
            } else {
                cout << "N: " << N_ << endl;
            }
            cout << "====================\n" << endl;
        }


    /**
     * Default destructor.
     */
    ~MonteCarlo() = default;


    /**
     * Run Monte Carlo Simulation.
     */
    void runSimulation(double alpha, Method method) const {
        // Record start time.
        auto startTime = chrono::steady_clock::now();

        // Prepare for multithreading
        mutex mtx;  // For locks
        int numThread = thread::hardware_concurrency();
        while (numThread > 1 && (N % numThread != 0)) {numThread--;}
        long simulationsPerThread = N / numThread;

        vector<thread> threads;
        vector<double> partialSum(numThread, 0.0);  // Vector to store sums of prices per thread.
        vector<double> partialSquaredSum(numThread, 0.0);   // Vector to store sums of squared prices per thread.
        double AmericanOptionSE = 0.0;

        // Define lambda expression for each thread to run.
        auto threadSimulation = [&](int threadId) {
            double threadSum = 0.0; // Sum of prices within the thread.
            double threadSquaredSum = 0.0;  // Sum of squared prices within the thread.
            
            for (long i = 0; i < simulationsPerThread; i++) {
                // Create instance of option and compute price.
                auto option = createOptionInstance();
                double price = option->computePrice(method);

                // Update sample mean and squared mean.
                threadSum += price;
                threadSquaredSum += price * price;

                if constexpr (std::is_same<T, AmericanOption>::value) {AmericanOptionSE = option->getSE();}
            }

            // Lock to safely update the partial results
            lock_guard<mutex> lock(mtx);
            partialSum[threadId] = threadSum;
            partialSquaredSum[threadId] = threadSquaredSum;
        };

        // Launch threads
        for (int i = 0; i < numThread; ++i) {
            threads.emplace_back(threadSimulation, i);
        }

        // Wait for all threads to complete
        for (auto& it : threads) {
            it.join();
        }

        // Update the multiprocessing results and compute the sample and squared mean.
        double sample_mean = 0.0; 
        double squared_mean = 0.0;

        for (int i = 0; i < numThread; ++i) {
            sample_mean += partialSum[i];
            squared_mean += partialSquaredSum[i];
        }

        sample_mean /= N;
        squared_mean /= N;

        // Compute standard error using the method from pg. 78 from the lecture note.
        double se = !std::is_same<T, AmericanOption>::value ? sqrt((squared_mean - sample_mean * sample_mean)/(N - 1)) : AmericanOptionSE;
        

        // Compute confidence interval
        math::normal normal_dist;
        double z = quantile(normal_dist, 1 - alpha / 2);
        double lower_bound = sample_mean - z * se;
        double upper_bound = sample_mean + z * se;

        // Get absolute pricing error
        double closed_form_price;
        if constexpr (std::is_same<T, EuropeanOption>::value) {closed_form_price = createOptionInstance()->BlackScholesPrice();} 
        else {closed_form_price = -1;}

        // Compute time it took
        auto endTime = chrono::steady_clock::now();
        chrono::duration<double> computationTime = endTime - startTime;

        // Output the result
        cout << "##### " << method << " Method Result #####" << endl;
        cout << "Option price: " << sample_mean << endl;
        cout << "Estimated SE: " << se << endl;
        cout << 100 - alpha * 100 << "% Confidence Interval: [" << lower_bound << ", " << upper_bound << "]" << endl;
        cout << "Absolute pricing error: " << (std::is_same<T, EuropeanOption>::value ? to_string(abs(sample_mean - closed_form_price)) : "N/A")  << endl;
        cout << "Computation time: " << computationTime.count() << "s" << endl;
    }


private:
    long N;  // Sample size
    tuple<Args...> args;  // Holds arguments for the options class constructor

    /**
     * Helper function to create option instance.
     * 
     * @return unique pointer to the option instance.
     */
    unique_ptr<T> createOptionInstance() const {
        return apply([](auto&&... args) {
            return make_unique<T>(std::forward<decltype(args)>(args)...);
        }, args);
    }
};
