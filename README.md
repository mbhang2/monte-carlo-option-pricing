# Monte Carlo Option Pricing

This is a multithreaded C++20 Monte Carlo pricer for three options under Black–Scholes–Merton, with variance-reduction techniques. It was the final project for IE 522 (Statistical Methods in Finance) at UIUC, Fall 2024.

**Report:** [`IE522_Monte_Carlo_Option_Pricing_Report.pdf`](IE522_Monte_Carlo_Option_Pricing_Report.pdf) (30 pages). The LaTeX source is in [`report/`](report).

## What it prices

| Option | Method | Extensions |
|---|---|---|
| European put | Monte Carlo, checked against the Black–Scholes closed form | Antithetic variates |
| Arithmetic Asian call (m = 50 fixings) | Monte Carlo | Geometric-Asian control variate, moment matching |
| American put | Longstaff–Schwartz least-squares Monte Carlo | Benchmark: binomial tree with Richardson extrapolation (BBSR) |

Each run reports the price, standard error, 95% confidence interval and wall-clock time.

## Results

- **European put** (S₀ = K = 100, T = 0.5, r = 4%, q = 2%, σ = 20%): the estimate converges at the expected 1/√N rate. Antithetic variates halve the standard error at the same N: at N = 20M, the price is 5.07427 ± 0.00089, an absolute error of $0.0004 against Black–Scholes.
- **Asian call** (S₀ = K = 100, T = 1, r = 10%, σ = 20%): the standard estimator settles at **7.161 ± 0.004** at N = 4.1M, and the control-variate estimator agrees (7.162).
- **American put** (T = 1 month): BBSR is stable at about 2.22 for every grid size. Longstaff–Schwartz converges to it as paths and time steps grow, reaching 2.2245 against 2.2249 at N = 10k and M = 1000. With few paths it is sensitive to the regression degree.
- **Performance:** splitting simulations across threads, with mutex-guarded aggregation, cut a 10M-path European valuation from 40.7 s to 7.9 s, about 5×. Memoizing the binomial tree takes BBSR from exponential to quadratic cost.

### Known issues

- **Moment matching is biased.** It returns about 7.72 for the Asian call, $0.56 above the other two estimators at every N, so the path adjustment is likely wrong.
- **The control variate doesn't reduce variance as implemented.** Its standard error is no lower than plain Monte Carlo's (0.012 vs 0.009 at N = 1M in a test run), where a geometric-Asian control should shrink it by an order of magnitude.

## Build and run

Requires a C++20 compiler and CMake 3.10 or later. Boost and Eigen are header-only here.

```bash
brew install boost eigen            # macOS (or: sudo apt install libboost-dev libeigen3-dev)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./exec/main                         # runs every question in sequence (~2–3 min on 2 cores)
```

Parameters are set in [`src/main.cpp`](src/main.cpp). The pricers are in [`include/`](include): `MonteCarlo.h` is the generic multithreaded driver, and `EuropeanOption`, `AsianOption` and `AmericanOption` hold the payoffs and methods.

## Team

Aakash Bansal, Michael Bhang, Saurabh Jamadagni and Yifan Meng.
