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


/*
*  Categorical distribution
*
*  Values:
*  x
*
*  Parameters:
*  0 <= p <= 1
*  sum(p) = 1ISNAN(prob_n(i % np, 0)) ? NAN : 0.0
*
*/


// [[Rcpp::export]]
NumericVector cpp_dcat(
    const NumericVector& x,
    const NumericMatrix& prob,
    bool log_prob = false
  ) {
  
  int n  = x.length();
  int np = prob.nrow();
  int Nmax = Rcpp::max(IntegerVector::create(n, np));
  int k = prob.ncol();
  NumericVector p(Nmax);
  NumericMatrix prob_n = normalize_prob(prob); 
  
  for (int i = 0; i < Nmax; i++) {
    if (ISNAN(x[i])) {
      p[i] = NA_REAL;
    } else if (!isInteger(x[i]) || x[i] < 1.0 || x[i] > static_cast<double>(k)) {
      p[i] = ISNAN(prob_n(i % np, 0)) ? NAN : 0.0;
    } else {
      p[i] = prob_n(i % np, static_cast<int>(x[i] - 1.0));
    }
  }

  if (log_prob)
    for (int i = 0; i < Nmax; i++)
      p[i] = log(p[i]);
    
    return p;
}


// [[Rcpp::export]]
NumericVector cpp_pcat(
    const NumericVector& x,
    const NumericMatrix& prob,
    bool lower_tail = true, bool log_prob = false
  ) {
  
  int n  = x.length();
  int np = prob.nrow();
  int Nmax = Rcpp::max(IntegerVector::create(n, np));
  int k = prob.ncol();
  NumericVector p(Nmax);
  NumericMatrix prob_n = normalize_prob(prob); 
  
  for (int i = 0; i < Nmax; i++) {
    if (ISNAN(x[i])) {
      p[i] = NA_REAL;
    } else if (x[i] < 1.0) {
      p[i] = ISNAN(prob_n(i % np, 0)) ? NAN : 0.0;
    } else if (x[i] > static_cast<double>(k)) {
      p[i] = ISNAN(prob_n(i % np, 0)) ? NAN : 1.0;
    } else {
      //bool wrong_param = false;
      p[i] = 0.0;
      int j = 0;
      while (j < static_cast<int>(x[i])) {
        p[i] += prob_n(i % np, j);
        j++;
      }
    }
  }

  if (!lower_tail)
    for (int i = 0; i < Nmax; i++)
      p[i] = 1.0 - p[i];
    
  if (log_prob)
    for (int i = 0; i < Nmax; i++)
      p[i] = log(p[i]);
      
  return p;
}


// [[Rcpp::export]]
NumericVector cpp_qcat(
    const NumericVector& p,
    const NumericMatrix& prob,
    bool lower_tail = true, bool log_prob = false
  ) {
  
  int n  = p.length();
  int np = prob.nrow();
  int Nmax = Rcpp::max(IntegerVector::create(n, np));
  int k = prob.ncol();
  NumericVector q(Nmax);
  NumericVector pp = Rcpp::clone(p);
  NumericMatrix prob_n = normalize_prob(prob); 
  
  if (log_prob)
    for (int i = 0; i < n; i++)
      pp[i] = exp(pp[i]);
    
  if (!lower_tail)
    for (int i = 0; i < n; i++)
      pp[i] = 1.0 - pp[i];
  
  int jj;
  double p_tmp;
    
  for (int i = 0; i < Nmax; i++) {
    if (ISNAN(pp[i])) {
      q[i] = NA_REAL;
    } else if (pp[i] < 0.0 || pp[i] > 1.0) {
      Rcpp::warning("NaNs produced");
      q[i] = NAN;
    } else if (pp[i] == 0.0) {
      q[i] = ISNAN(prob_n(i % np, 0)) ? NAN : 1.0; 
    } else {
      p_tmp = 1.0;
      jj = 0;
      
      for (int j = k-1; j >= 0; j--) {
        p_tmp -= prob_n(i % np, j);
        if (pp[i] > p_tmp) {
          jj = j;
          break;
        }
      }

      if (ISNAN(p_tmp)) {
        q[i] = NAN;
      } else {
        q[i] = static_cast<double>(jj+1);
      }
    }
  }
      
  return q;
}


// [[Rcpp::export]]
NumericVector cpp_rcat(
    const int n,
    const NumericMatrix& prob
  ) {
  
  int np = prob.nrow();
  int k = prob.ncol();
  NumericVector x(n);
  NumericMatrix prob_n = normalize_prob(prob); 
  
  int jj;
  double u, p_tmp;

  for (int i = 0; i < n; i++) {
    u = rng_unif();
    p_tmp = 1.0;
    jj = 0;
    
    for (int j = k-1; j >= 0; j--) {
      p_tmp -= prob_n(i % np, j);
      if (u > p_tmp) {
        jj = j;
        break;
      }
    }

    if (ISNAN(p_tmp)) {
      x[i] = NAN;
    } else {
      x[i] = static_cast<double>(jj+1);
    }
  }
  
  return x;
}

