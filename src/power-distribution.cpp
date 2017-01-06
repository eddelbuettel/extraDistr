#include <Rcpp.h>
#include "shared.h"

using std::pow;
using std::sqrt;
using std::abs;
using std::exp;
using std::log;
using std::floor;
using std::ceil;
using std::sin;
using std::cos;
using std::tan;
using std::atan;
using Rcpp::IntegerVector;
using Rcpp::NumericVector;
using Rcpp::NumericMatrix;


/*
*  Power distribution
*
*  Values:
*  0 < x < alpha
*
*  Parameters:
*  alpha
*  beta
*
*  f(x)    = (beta*x^(beta-1)) / (alpha^beta)
*  F(x)    = x^beta / alpha^beta
*  F^-1(p) = alpha * p^(1/beta)
*
*/

double pdf_power(double x, double alpha, double beta) {
  if (ISNAN(x) || ISNAN(alpha) || ISNAN(beta))
    return NA_REAL;
  if (x <= 0.0 || x >= alpha)
    return 0.0;
  return beta * pow(x, beta-1.0) / pow(alpha, beta);
}

double cdf_power(double x, double alpha, double beta) {
  if (ISNAN(x) || ISNAN(alpha) || ISNAN(beta))
    return NA_REAL;
  if (x <= 0.0)
    return 0.0;
  if (x >= alpha)
    return 1.0;
  return pow(x, beta) / pow(alpha, beta);
}

double invcdf_power(double p, double alpha, double beta) {
  if (ISNAN(p) || ISNAN(alpha) || ISNAN(beta))
    return NA_REAL;
  if (p < 0.0 || p > 1.0) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  return alpha * pow(p, 1.0/beta);
}

double rng_power(double alpha, double beta) {
  if (ISNAN(alpha) || ISNAN(beta)) {
    Rcpp::warning("NAs produced");
    return NA_REAL;
  }
  double u = rng_unif();
  return alpha * pow(u, 1.0/beta);
}

double logpdf_power(double x, double alpha, double beta) {
  if (ISNAN(x) || ISNAN(alpha) || ISNAN(beta))
    return NA_REAL;
  if (x <= 0.0 || x >= alpha)
    return R_NegInf;
  return log(beta) + log(x)*(beta-1.0) - log(alpha)*beta;
}

double logcdf_power(double x, double alpha, double beta) {
  if (ISNAN(x) || ISNAN(alpha) || ISNAN(beta))
    return NA_REAL;
  if (x <= 0.0)
    return R_NegInf;
  if (x >= alpha)
    return 0.0;
  return log(x)*beta - log(alpha)*beta;
}


// [[Rcpp::export]]
NumericVector cpp_dpower(
    const NumericVector& x,
    const NumericVector& alpha,
    const NumericVector& beta,
    const bool& log_prob = false
  ) {

  std::vector<int> dims;
  dims.push_back(x.length());
  dims.push_back(alpha.length());
  dims.push_back(beta.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector p(Nmax);

  for (int i = 0; i < Nmax; i++)
    p[i] = logpdf_power(x[i % dims[0]], alpha[i % dims[1]], beta[i % dims[2]]);

  if (!log_prob)
    p = Rcpp::exp(p);

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_ppower(
    const NumericVector& x,
    const NumericVector& alpha,
    const NumericVector& beta,
    const bool& lower_tail = true,
    const bool& log_prob = false
  ) {

  std::vector<int> dims;
  dims.push_back(x.length());
  dims.push_back(alpha.length());
  dims.push_back(beta.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector p(Nmax);

  for (int i = 0; i < Nmax; i++)
    p[i] = logcdf_power(x[i % dims[0]], alpha[i % dims[1]], beta[i % dims[2]]);

  if (!lower_tail)
    p = 1.0 - p;

  if (!log_prob)
    p = Rcpp::exp(p);

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_qpower(
    const NumericVector& p,
    const NumericVector& alpha,
    const NumericVector& beta,
    const bool& lower_tail = true,
    const bool& log_prob = false
  ) {

  std::vector<int> dims;
  dims.push_back(p.length());
  dims.push_back(alpha.length());
  dims.push_back(beta.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector x(Nmax);
  NumericVector pp = Rcpp::clone(p);

  if (log_prob)
    pp = Rcpp::exp(pp);

  if (!lower_tail)
    pp = 1.0 - pp;

  for (int i = 0; i < Nmax; i++)
    x[i] = invcdf_power(pp[i % dims[0]], alpha[i % dims[1]], beta[i % dims[2]]);

  return x;
}


// [[Rcpp::export]]
NumericVector cpp_rpower(
    const int& n,
    const NumericVector& alpha,
    const NumericVector& beta
  ) {

  std::vector<int> dims;
  dims.push_back(alpha.length());
  dims.push_back(beta.length());
  NumericVector x(n);

  for (int i = 0; i < n; i++)
    x[i] = rng_power(alpha[i % dims[0]], beta[i % dims[1]]);

  return x;
}

