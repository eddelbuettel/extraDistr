#include <Rcpp.h>
#include "namespace.h"


/*
 *  Laplace distribution
 *
 *  Values:
 *  x
 *
 *  Parameters:
 *  mu
 *  sigma > 0
 *
 *  z = (x-mu)/sigma
 *  f(x)    = 1/(2*sigma) * exp(-|z|)
 *  F(x)    = { 1/2 * exp(z)                 if   x < mu
 *            { 1 - 1/2 * exp(z)             otherwise
 *  F^-1(p) = { mu + sigma * log(2*p)        if p <= 0.5
 *            { mu + sigma * log(2*(1-p))    otherwise
 *
 */

double pdf_laplace(double x, double mu, double sigma) {
  if (sigma <= 0) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  double z = abs(x-mu)/sigma;
  return 1/(2*sigma) * exp(-z);
}

double cdf_laplace(double x, double mu, double sigma) {
  if (sigma <= 0) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  double z = (x-mu)/sigma;
  if (x < mu)
    return exp(z)/2;
  else
    return 1 - exp(-z)/2;
}

double invcdf_laplace(double p, double mu, double sigma) {
  if (sigma <= 0 || p < 0 || p > 1) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  if (p < 0.5)
    return mu + sigma * log(2*p);
  else
    return mu - sigma * log(2*(1-p));
}

double rng_laplace(double mu, double sigma) {
  if (sigma <= 0) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  double u = R::runif(-0.5, 0.5);
  return mu + sigma * R::sign(u) * log(1 - 2*abs(u));
}


// [[Rcpp::export]]
NumericVector cpp_dlaplace(
    const NumericVector& x,
    const NumericVector& mu,
    const NumericVector& sigma,
    bool log_prob = false
  ) {

  int n  = x.length();
  int nm = mu.length();
  int ns = sigma.length();
  int Nmax = Rcpp::max(IntegerVector::create(n, nm, ns));
  NumericVector p(Nmax);

  for (int i = 0; i < Nmax; i++)
    p[i] = pdf_laplace(x[i % n], mu[i % nm], sigma[i % ns]);

  if (log_prob)
    for (int i = 0; i < Nmax; i++)
      p[i] = log(p[i]);

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_plaplace(
    const NumericVector& x,
    const NumericVector& mu,
    const NumericVector& sigma,
    bool lower_tail = true, bool log_prob = false
  ) {

  int n  = x.length();
  int nm = mu.length();
  int ns = sigma.length();
  int Nmax = Rcpp::max(IntegerVector::create(n, nm, ns));
  NumericVector p(Nmax);

  for (int i = 0; i < Nmax; i++)
    p[i] = cdf_laplace(x[i % n], mu[i % nm], sigma[i % ns]);

  if (!lower_tail)
    for (int i = 0; i < Nmax; i++)
      p[i] = 1-p[i];

  if (log_prob)
    for (int i = 0; i < Nmax; i++)
      p[i] = log(p[i]);

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_qlaplace(
    const NumericVector& p,
    const NumericVector& mu,
    const NumericVector& sigma,
    bool lower_tail = true, bool log_prob = false
  ) {

  int n  = p.length();
  int nm = mu.length();
  int ns = sigma.length();
  int Nmax = Rcpp::max(IntegerVector::create(n, nm, ns));
  NumericVector q(Nmax);
  NumericVector pp = Rcpp::clone(p);

  if (log_prob)
    for (int i = 0; i < n; i++)
      pp[i] = exp(pp[i]);

  if (!lower_tail)
    for (int i = 0; i < n; i++)
      pp[i] = 1-pp[i];

  for (int i = 0; i < Nmax; i++)
    q[i] = invcdf_laplace(pp[i % n], mu[i % nm], sigma[i % ns]);

  return q;
}


// [[Rcpp::export]]
NumericVector cpp_rlaplace(
    const int n,
    const NumericVector& mu,
    const NumericVector& sigma
  ) {

  int nm = mu.length();
  int ns = sigma.length();
  NumericVector x(n);

  for (int i = 0; i < n; i++)
    x[i] = rng_laplace(mu[i % nm], sigma[i % ns]);

  return x;
}

