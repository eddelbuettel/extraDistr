#include <Rcpp.h>
#include "shared.h"

using std::pow;
using std::sqrt;
using std::abs;
using std::exp;
using std::log;
using std::floor;
using std::ceil;
using Rcpp::NumericVector;


/*
 *  Rayleigh distribution
 *
 *  Values:
 *  x >= 0
 *
 *  Parameters:
 *  sigma > 0
 *
 *  f(x)    = x/sigma^2 * exp(-(x^2 / 2*sigma^2))
 *  F(x)    = 1 - exp(-x^2 / 2*sigma^2)
 *  F^-1(p) = sigma * sqrt(-2 * log(1-p))
 *
 */

double pdf_rayleigh(double x, double sigma,
                    bool& throw_warning) {
  if (ISNAN(x) || ISNAN(sigma))
    return x+sigma;
  if (sigma <= 0.0) {
    throw_warning = true;
    return NAN;
  }
  if (x < 0.0 || !R_FINITE(x))
    return 0.0;
  return x/pow(sigma, 2.0) * exp(-pow(x, 2.0) / (2.0*pow(sigma, 2.0)));
}

double cdf_rayleigh(double x, double sigma,
                    bool& throw_warning) {
  if (ISNAN(x) || ISNAN(sigma))
    return x+sigma;
  if (sigma <= 0.0) {
    throw_warning = true;
    return NAN;
  }
  if (x < 0)
    return 0.0;
  if (!R_FINITE(x))
    return 1.0;
  return 1.0 - exp(-pow(x, 2.0) / (2.0*pow(sigma, 2.0)));
}

double invcdf_rayleigh(double p, double sigma,
                       bool& throw_warning) {
  if (ISNAN(p) || ISNAN(sigma))
    return p+sigma;
  if (p < 0.0 || p > 1.0 || sigma <= 0.0) {
    throw_warning = true;
    return NAN;
  }
  return sqrt(-2.0*pow(sigma, 2.0) * log(1.0-p));
}

double rng_rayleigh(double sigma, bool& throw_warning) {
  if (ISNAN(sigma) || sigma <= 0.0) {
    throw_warning = true;
    return NA_REAL;
  }
  double u = rng_unif();
  return sqrt(-2.0*pow(sigma, 2.0) * log(u));
}


// [[Rcpp::export]]
NumericVector cpp_drayleigh(
    const NumericVector& x,
    const NumericVector& sigma,
    const bool& log_prob = false
  ) {

  std::vector<int> dims;
  dims.push_back(x.length());
  dims.push_back(sigma.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector p(Nmax);
  
  bool throw_warning = false;

  for (int i = 0; i < Nmax; i++)
    p[i] = pdf_rayleigh(x[i % dims[0]], sigma[i % dims[1]],
                        throw_warning);

  if (log_prob)
    p = Rcpp::log(p);
  
  if (throw_warning)
    Rcpp::warning("NaNs produced");

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_prayleigh(
    const NumericVector& x,
    const NumericVector& sigma,
    const bool& lower_tail = true,
    const bool& log_prob = false
  ) {

  std::vector<int> dims;
  dims.push_back(x.length());
  dims.push_back(sigma.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector p(Nmax);
  
  bool throw_warning = false;

  for (int i = 0; i < Nmax; i++)
    p[i] = cdf_rayleigh(x[i % dims[0]], sigma[i % dims[1]],
                        throw_warning);

  if (!lower_tail)
    p = 1.0 - p;
  
  if (log_prob)
    p = Rcpp::log(p);
  
  if (throw_warning)
    Rcpp::warning("NaNs produced");

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_qrayleigh(
    const NumericVector& p,
    const NumericVector& sigma,
    const bool& lower_tail = true,
    const bool& log_prob = false
  ) {

  std::vector<int> dims;
  dims.push_back(p.length());
  dims.push_back(sigma.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector q(Nmax);
  NumericVector pp = Rcpp::clone(p);
  
  bool throw_warning = false;

  if (log_prob)
    pp = Rcpp::exp(pp);
  
  if (!lower_tail)
    pp = 1.0 - pp;

  for (int i = 0; i < Nmax; i++)
    q[i] = invcdf_rayleigh(pp[i % dims[0]], sigma[i % dims[1]],
                           throw_warning);
  
  if (throw_warning)
    Rcpp::warning("NaNs produced");

  return q;
}


// [[Rcpp::export]]
NumericVector cpp_rrayleigh(
    const int& n,
    const NumericVector& sigma
  ) {

  int dims = sigma.length();
  NumericVector x(n);
  
  bool throw_warning = false;

  for (int i = 0; i < n; i++)
    x[i] = rng_rayleigh(sigma[i % dims], throw_warning);
  
  if (throw_warning)
    Rcpp::warning("NAs produced");

  return x;
}

