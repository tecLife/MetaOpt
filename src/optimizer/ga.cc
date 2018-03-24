#include "ga.h"
using namespace std;
namespace MetaOpt {
template <typename Real>
Ga<Real>::Ga(const int               n_dim,
             const int               n_obj,
             const int               n_con,
             const Func<Real>        func,
             const std::vector<Real> upper,
             const std::vector<Real> lower,
             const int               n_population,
             const int               n_generation,
             const Real              p_crossover,
             const Real              p_mutation)
    : Optimizer<Real>(n_dim, n_obj, n_con, func, upper, lower) {
  n_population_ = n_population > 0 ? n_population : 10 * n_dim;
  n_generation_ = n_generation > 0 ? n_generation : 20 * n_dim;
  population_ = Samples<Real>(n_population_);
  for (auto& p : population_)
    p = this->sample();
  newpop_ = Samples<Real>(n_population_);
  for (auto& p : newpop_)
    p = this->sample();
  cons_p_ = vector<Real>(n_population_, 0);
  p_crossover_ = 1 - pow(1 - p_crossover, 1.0 / n_dim);
  p_mutation_ = 1 - pow(1 - p_mutation, 1.0 / n_dim);
}
template <typename Real>
Ga<Real>::~Ga() {}
template <typename Real>
void Ga<Real>::opt() {
  tourlist_ = vector<int>(n_population_);
  int i = 0;
  for (auto& t : tourlist_)
    t = i++;
  for (i_generation_ = 0; i_generation_ < n_generation_; ++i_generation_) {
    statistics();
    Random::shuffle(tourlist_);
    for (int k = 0; k != n_population_; ++k) {
      newpop_[k] = select(population_[k], population_[tourlist_[k]]);
    }
    for (int k = 0; k != n_population_; k += 2) {
      crossover(newpop_[k], newpop_[k + 1]);
    }
    for (auto& p : newpop_) {
      mutation(p);
    }
    for (int k = 0; k < n_population_; ++k) {
      population_[k].swap(newpop_[k]);
    }
    if (i_generation_ % 10 == 0) {
      LOG(INFO) << "Generation " << i_generation_ + 1
                << ", best: " << this->best_;
    }
  }
  LOG(INFO) << "Ga opt finished, best:  " << this->best_;
  LOG(INFO) << "n_evaluation = " << this->n_evaluation_
            << ", n_mutation = " << i_mutation_
            << ", n_crossover = " << i_crossover_;
}

template <typename Real>
void Ga<Real>::statistics() {
  for (auto& p : population_) {
    this->evaluate(p);
  }
}

template <typename Real>
Sample<Real>& Ga<Real>::select(Sample<Real>& s1, Sample<Real>& s2) {
  Real s1_con = 0, s2_con = 0;
  for (auto& c : s1.con())
    if (c < 0) s1_con += c;
  for (auto& c : s2.con())
    if (c < 0) s2_con += c;
  auto& better =
      ((s1_con == s2_con ? s1.obj()[0] < s2.obj()[0] : s1_con > s2_con)
           ? s1
           : (s1_con = s2_con, s2));
  if (better.obj()[0] < this->best_.obj()[0] && s1_con == 0)
    this->best_ = better;
  return better;
}

template <typename Real>
void Ga<Real>::crossover(Sample<Real>& s1, Sample<Real>& s2) {
  switch (1) {
    case 0:
      for (int i = 0; i != this->n_dim_; ++i) {
        if (Random::get<Real>(0, 1) < p_crossover_) {
          Real dif = s2.x()[i] - s1.x()[i];
          s2.x()[i] = s1.x()[i] + Random::get<Real>(0, 1) * dif;
          s1.x()[i] = s1.x()[i] + Random::get<Real>(0, 1) * dif;
          ++i_crossover_;
        }
      }
      break;
    case 1:

      for (int i = 0; i != this->n_dim_; ++i) {
        if (Random::get<Real>(0, 1) < p_crossover_) {
          auto& x1i = s1.x()[i];
          auto& x2i = s2.x()[i];
          if (x1i > x2i) swap(x1i, x2i);
          Real mid = (x2i + x1i) / 2;
          Real dif = x2i - x1i;
          if (dif < 1e-6) {
            dif = 1e-6;
          }
          Real beta =
              1 + 2 / dif * min(x1i - this->lower_[i], this->upper_[i] - x2i);
          Real alpha = 2 - pow(beta, -(nc + 1));
          if (alpha > 2 - 1e-6) alpha = 2 - 1e-6;
          alpha *= Random::get<Real>(0, 1);
          if (alpha <= 1)
            beta = pow(alpha, 1.0 / (nc + 1));
          else
            beta = 1.0 / pow(2 - alpha, 1.0 / (nc + 1));

          x1i = mid - 0.5 * beta * dif;
          x2i = mid + 0.5 * beta * dif;
          if (isnan(x1i) || isinf(x1i))
            x1i = Random::get<Real>(this->lower_[i], this->upper_[i]);
          if (x1i > this->upper_[i]) x1i = this->upper_[i];
          if (x1i < this->lower_[i]) x1i = this->lower_[i];
          if (isnan(x2i) || isinf(x2i))
            x2i = Random::get<Real>(this->lower_[i], this->upper_[i]);
          if (x2i > this->upper_[i]) x2i = this->upper_[i];
          if (x2i < this->lower_[i]) x2i = this->lower_[i];
          ++i_crossover_;
        }
      }
      break;
  }
}

template <typename Real>
void Ga<Real>::mutation(Sample<Real>& s) {
  switch (0) {
    case 0:
      for (int i = 0; i != this->n_dim_; ++i) {
        if (Random::get<Real>(0, 1) < p_mutation_) {
          s.x()[i] = Random::get<Real>(this->lower_[i], this->upper_[i]);
          ++i_mutation_;
        }
      }
    case 1:
      for (int i = 0; i != this->n_dim_; ++i) {
        if (Random::get<Real>(0, 1) < p_mutation_) {
          auto& xi = s.x()[i];
          Real  u = Random::get<Real>(0, 1);
          Real  deltamax = this->upper_[i] - this->lower_[i];
          Real  delta =
              min(xi - this->lower_[i], this->upper_[i] - xi) / deltamax;
          if (u > 0.5) {
            u = 1 - u;
            delta = 1 - pow((2 * u + (1 - 2 * u) * pow(1 - delta, nm + 1)),
                            1 / (nm + 1));
          } else {
            delta = pow((2 * u + (1 - 2 * u) * pow(1 - delta, nm + 1)),
                        1 / (nm + 1)) -
                    1;
          }
          xi += delta * deltamax;
          if (isnan(xi) || isinf(xi))
            xi = Random::get<Real>(this->lower_[i], this->upper_[i]);
          if (xi > this->upper_[i])
            xi = this->upper_[i];
          else if (xi < this->lower_[i])
            xi = this->lower_[i];
          ++i_mutation_;
        }
      }
      break;
  }
}
template class Ga<double>;
}  // namespace MetaOpt
