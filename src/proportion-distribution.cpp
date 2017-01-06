#include <Rcpp.h>

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
*  Re-parametrized beta distribution
*
*  Values:
*  x
*
*  Parameters:
*  0 <= mean <= 1
*  size > 0
*
*/

double pdf_prop(double x, double size, double mean) {
  if (ISNAN(x) || ISNAN(size) || ISNAN(mean))
    return NA_REAL;
  if (size <= 0.0 || mean < 0.0 || mean > 1.0) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  return R::dbeta(x, size*mean+1.0, size*(1.0-mean)+1.0, false);
}

double cdf_prop(double x, double size, double mean) {
  if (ISNAN(x) || ISNAN(size) || ISNAN(mean))
    return NA_REAL;
  if (size <= 0.0 || mean < 0.0 || mean > 1.0) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  return R::pbeta(x, size*mean+1.0, size*(1.0-mean)+1.0, true, false);
}

double invcdf_prop(double p, double size, double mean) {
  if (ISNAN(p) || ISNAN(size) || ISNAN(mean))
    return NA_REAL;
  if (size <= 0.0 || mean < 0.0 || mean > 1.0 || p < 0.0 || p > 1.0) {
    Rcpp::warning("NaNs produced");
    return NAN;
  }
  return R::qbeta(p, size*mean+1.0, size*(1.0-mean)+1.0, true, false);
}

double rng_prop(double size, double mean) {
  if (ISNAN(size) || ISNAN(mean) ||
      size <= 0.0 || mean < 0.0 || mean > 1.0) {
    Rcpp::warning("NAs produced");
    return NA_REAL;
  }
  return R::rbeta(size*mean+1.0, size*(1.0-mean)+1.0);
}


// [[Rcpp::export]]
NumericVector cpp_dprop(
    const NumericVector& x,
    const NumericVector& size,
    const NumericVector& mean,
    const bool& log_prob = false
  ) {
  
  std::vector<int> dims;
  dims.push_back(x.length());
  dims.push_back(mean.length());
  dims.push_back(size.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector p(Nmax);
  
  for (int i = 0; i < Nmax; i++)
    p[i] = pdf_prop(x[i % dims[0]], size[i % dims[1]], mean[i % dims[2]]);
  
  if (log_prob)
    p = Rcpp::log(p);

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_pprop(
    const NumericVector& x,
    const NumericVector& size,
    const NumericVector& mean,
    const bool& lower_tail = true,
    const bool& log_prob = false
  ) {
  
  std::vector<int> dims;
  dims.push_back(x.length());
  dims.push_back(mean.length());
  dims.push_back(size.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector p(Nmax);
  
  for (int i = 0; i < Nmax; i++)
    p[i] = cdf_prop(x[i % dims[0]], size[i % dims[1]], mean[i % dims[2]]);
  
  if (!lower_tail)
    p = 1.0 - p;
  
  if (log_prob)
    p = Rcpp::log(p);

  return p;
}


// [[Rcpp::export]]
NumericVector cpp_qprop(
    const NumericVector& p,
    const NumericVector& size,
    const NumericVector& mean,
    const bool& lower_tail = true,
    const bool& log_prob = false
  ) {
  
  std::vector<int> dims;
  dims.push_back(p.length());
  dims.push_back(mean.length());
  dims.push_back(size.length());
  int Nmax = *std::max_element(dims.begin(), dims.end());
  NumericVector x(Nmax);
  NumericVector pp = Rcpp::clone(p);
  
  if (log_prob)
    pp = Rcpp::exp(pp);
  
  if (!lower_tail)
    pp = 1.0 - pp;
  
  for (int i = 0; i < Nmax; i++)
    x[i] = invcdf_prop(pp[i % dims[0]], size[i % dims[1]], mean[i % dims[2]]);
  
  return x;
}


// [[Rcpp::export]]
NumericVector cpp_rprop(
    const int& n,
    const NumericVector& size,
    const NumericVector& mean
  ) {
  
  std::vector<int> dims;
  dims.push_back(mean.length());
  dims.push_back(size.length());
  NumericVector x(n);
  
  for (int i = 0; i < n; i++)
    x[i] = rng_prop(size[i % dims[0]], mean[i % dims[1]]);
  
  return x;
}

