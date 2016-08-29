#include <Rcpp.h>
#include "const.h"
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


double pdf_huber(double x, double mu, double sigma, double c) {
  if (ISNAN(x) || ISNAN(mu) || ISNAN(sigma) || ISNAN(c))
    return NAN;
  // if (sigma <= 0.0 || c <= 0.0) {
  //   Rcpp::warning("NaNs produced");
  //   return NAN;
  // }
  
  double z, A, rho;
  z = abs((x - mu)/sigma);
  A = 2.0*SQRT_2_PI * (Phi(c) + phi(c)/c - 0.5);

  if (z <= c)
    rho = pow(z, 2.0)/2.0;
  else
    rho = c*z - pow(c, 2.0)/2.0;

  return exp(-rho)/A/sigma;
}

double cdf_huber(double x, double mu, double sigma, double c) {
  if (ISNAN(x) || ISNAN(mu) || ISNAN(sigma) || ISNAN(c))
    return NAN;
  // if (sigma <= 0.0 || c <= 0.0) {
  //   Rcpp::warning("NaNs produced");
  //   return NAN;
  // }

  double A, z, az, p;
  A = 2.0*(phi(c)/c - Phi(-c) + 0.5);
  z = (x - mu)/sigma;
  az = -abs(z);
  
  if (az <= -c) 
    p = exp(pow(c, 2.0)/2.0)/c * exp(c*az) / SQRT_2_PI/A;
  else
    p = (phi(c)/c + Phi(az) - Phi(-c))/A;
  
  if (z <= 0.0)
    return p;
  else
    return 1.0 - p;
}

double invcdf_huber(double p, double mu, double sigma, double c) {
  if (ISNAN(p) || ISNAN(mu) || ISNAN(sigma) || ISNAN(c))
    return NAN;
  // if (sigma <= 0.0 || c <= 0.0 || p < 0.0 || p > 1.0) {
  //   Rcpp::warning("NaNs produced");
  //   return NAN;
  // }

  double x, pm, A;
  A = 2.0*SQRT_2_PI * (Phi(c) + phi(c)/c - 0.5);
  pm = std::min(p, 1.0 - p);

  if (pm <= SQRT_2_PI * phi(c)/(c*A))
    x = log(c*pm*A)/c - c/2.0;
  else
    x = InvPhi(abs(1.0 - Phi(c) + pm*A/SQRT_2_PI - phi(c)/c));

  if (p < 0.5)
    return mu + x*sigma;
  else
    return mu - x*sigma;
}


// [[Rcpp::export]]
NumericVector cpp_dhuber(
    const NumericVector& x,
    const NumericVector& mu,
    const NumericVector& sigma,
    const NumericVector& epsilon,
    bool log_prob = false
  ) {
  
  int n  = x.length();
  int nm = mu.length();
  int ns = sigma.length();
  int ne = epsilon.length();
  int Nmax = Rcpp::max(IntegerVector::create(n, nm, ns, ne));
  NumericVector p(Nmax);
  NumericVector sigma_n = positive_or_nan(sigma);
  NumericVector epsilon_n = positive_or_nan(epsilon);
  
  for (int i = 0; i < Nmax; i++)
    p[i] = pdf_huber(x[i % n], mu[i % nm], sigma_n[i % ns], epsilon_n[i % ne]);
  
  if (log_prob)
    for (int i = 0; i < Nmax; i++)
      p[i] = log(p[i]);
  
  return p;
}


// [[Rcpp::export]]
NumericVector cpp_phuber(
    const NumericVector& x,
    const NumericVector& mu,
    const NumericVector& sigma,
    const NumericVector& epsilon,
    bool lower_tail = true, bool log_prob = false
  ) {
  
  int n  = x.length();
  int nm = mu.length();
  int ns = sigma.length();
  int ne = epsilon.length();
  int Nmax = Rcpp::max(IntegerVector::create(n, nm, ns, ne));
  NumericVector p(Nmax);
  NumericVector sigma_n = positive_or_nan(sigma);
  NumericVector epsilon_n = positive_or_nan(epsilon);
  
  for (int i = 0; i < Nmax; i++)
    p[i] = cdf_huber(x[i % n], mu[i % nm], sigma_n[i % ns], epsilon_n[i % ne]);
  
  if (!lower_tail)
    for (int i = 0; i < Nmax; i++)
      p[i] = 1.0 - p[i];
  
  if (log_prob)
    for (int i = 0; i < Nmax; i++)
      p[i] = log(p[i]);
  
  return p;
}


// [[Rcpp::export]]
NumericVector cpp_qhuber(
    const NumericVector& p,
    const NumericVector& mu,
    const NumericVector& sigma,
    const NumericVector& epsilon,
    bool lower_tail = true, bool log_prob = false
  ) {
  
  int n  = p.length();
  int nm = mu.length();
  int ns = sigma.length();
  int ne = epsilon.length();
  int Nmax = Rcpp::max(IntegerVector::create(n, nm, ns, ne));
  NumericVector q(Nmax);
  NumericVector pp = Rcpp::clone(p);
  NumericVector sigma_n = positive_or_nan(sigma);
  NumericVector epsilon_n = positive_or_nan(epsilon);
  
  if (log_prob)
    for (int i = 0; i < n; i++)
      pp[i] = exp(pp[i]);
  
  if (!lower_tail)
    for (int i = 0; i < n; i++)
      pp[i] = 1.0 - pp[i];
  
  pp = zeroone_or_nan(pp);
  
  for (int i = 0; i < Nmax; i++)
    q[i] = invcdf_huber(pp[i % n], mu[i % nm], sigma_n[i % ns], epsilon_n[i % ne]);
  
  return q;
}


// [[Rcpp::export]]
NumericVector cpp_rhuber(
    const int n,
    const NumericVector& mu,
    const NumericVector& sigma,
    const NumericVector& epsilon
  ) {
  
  double u;
  int nm = mu.length();
  int ns = sigma.length();
  int ne = epsilon.length();
  NumericVector x(n);
  NumericVector sigma_n = positive_or_nan(sigma);
  NumericVector epsilon_n = positive_or_nan(epsilon);
  
  for (int i = 0; i < n; i++) {
    u = rng_unif();
    x[i] = invcdf_huber(u, mu[i % nm], sigma_n[i % ns], epsilon_n[i % ne]);
  }
  
  return x;
}

