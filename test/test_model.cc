#include "model/gp.h"
#include "doe.h"
#include <chrono>
// #include <range/v3/all.hpp>
using namespace std;
using namespace MetaOpt;
using namespace chrono;


template < typename Real, int DIM >
void rosenbrock(Real *x, Real *y, Real *c) {
  y[0] = 0;
  for(int i=0;i!=DIM-1;++i){
    y[0] += 100 * pow((x[i+1] - x[i] * x[i]), 2) +
          pow((1 - x[i]), 2);
  }
}
template < typename Real>
void G9(Real *x, Real *y, Real *c) {
  c[0] = -(2 * pow(x[0], 2) + 3 * pow(x[1], 4) + x[2] + 4 * pow(x[3], 2) +
           5 * x[4] - 127);
  c[1] = -(7 * x[0] + 3 * x[1] + 10 * pow(x[2], 2) + x[3] - x[4] - 282);
  c[2] = -(23 * x[0] + pow(x[1], 2) + 6 * pow(x[5], 2) - 8 * x[6] - 196);
  c[3] = -(4 * pow(x[0], 2) + pow(x[1], 2) - 3 * x[0] * x[1] +
           2 * pow(x[2], 2) + 5 * x[5] - 11 * x[6]);
  y[0] = pow((x[0] - 10), 2) + 5 * pow((x[1] - 12), 2) + pow(x[2], 4) +
         3 * pow((x[3] - 11), 2) + 10 * pow(x[4], 6) + 7 * pow(x[5], 2) +
         pow(x[6], 4) - 4 * x[5] * x[6] - 10 * x[5] - 8 * x[6];
}
template < typename Real>
void test_fun(Real *x, Real *y, Real *c){
  y[0] = exp(-pow(x[0] - 2, 2)) + exp(-pow(x[0] - 6, 2)/10) + 1 / (x[0]*x[0] + 1);
}

void test_to_sklearn(){
  const int n = 13;
  const double l = -2, u = 10;
  double dx = (u-l)/(n-1);
  Samples<double> ss(n);
  double begin = l;
  for(auto& s: ss){
    s = Sample<double>(1, 1, 0);
    s.x()[0] = begin;
    s.evaluate(test_fun<double>);
    begin += dx;
    cout << s << endl;
  }
  MetaOpt::Gp<double> m;
  m.fit(ss);

  const int nt = 100;
  dx = (u-l)/(nt-1);
  Samples<double> st(nt);
  begin = l;
  double error = 0;
  double mx = 0;
  cout << "[";
  for(auto& s: st){
    s = Sample<double>(1, 1, 0);
    s.x()[0] = begin;
    begin += dx;
    s.evaluate(test_fun<double>);
    double accu = s.obj()[0];
    vector<double> std(1);
    m.evaluate(s, std);
    double pred = s.obj()[0];
    error += fabs(accu - pred);
    mx = max(fabs(accu - pred), mx);
    cout << "[" << pred << "," << std[0] << "],";
  }
  cout << "\b]" << endl;
  cout << error / nt << endl;
  cout << mx << endl;
}


int main() {
  // (n_dim, n_obj, n_con, func, upper, lower)
  // MetaOpt::Ga<Real> a(2, 1, 0, rosenbrock, vector<Real>{-2, -2},
  // vector<Real>{2, 2}, 80, 100);
  // MetaOpt::Ga<Real> a(7, 1, 4, G9, vector<Real>(7, 0),
  //                       vector<Real>(7, 10), 100, 500);
  // a.opt();
  // auto s = a.sample();
  // std::cout << s << std::endl;
  // a.evaluate(s);
  // std::cout << s << std::endl;
  // MetaOpt::Ga<Real> a(2, 1, 0, rosenbrock, vector<Real>{0, 0},
  // vector<Real>{2, 0}, 10,0.1,0.1);
  // MetaOpt::<float> a(2);


  // constexpr int DIM = 1;
  // using Real = double;

  // auto ss = MetaOpt::Doe::gen<Real>(10, DIM, 1, 0);
  // // cout << ss << endl;
  // // cout << std::numeric_limits<Real>::min() << " : "<< std::numeric_limits<Real>::max()<< endl;
  // for(auto& s: ss)
  //   s.evaluate(test_fun<Real>);
  // // MetaOpt::Gp<Real> m(ss);
  // auto start = system_clock::now();
  // MetaOpt::Gp<Real> m;
  // m.fit(ss);
  // for(auto& s: m.samples_)
  //   cout << s << endl;
  // m.evaluate(ss[0]);
  // auto end = system_clock::now();
  // auto duration = duration_cast<microseconds>(end - start);
  // cout << " using "
  //      << Real(duration.count()) * microseconds::period::num /
  //             microseconds::period::den
  //      << " s." << endl;

  test_to_sklearn();


  // auto b = ranges::view::ints(0, 6);
  // std::cout << b << std::endl;
  // vector<Real> v{1,2,3,4};
  // Random::shuffle(b.begin(), b.end());
  // Random::shuffle(b.begin(), b.end());
  // Random::shuffle(v);
  // std::cout << v << std::endl;

  return 0;
}