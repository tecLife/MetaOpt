#include "Powell.h"
#include <iostream>
#include <cmath>
#include <random>
#include <ctime>
using namespace std;
Powell::Powell(){
	gen.seed(clock());
}

Powell::Powell(int _nv):nv(_nv){

}

Powell::Powell(int _nv, FUNC f):nv(_nv),func(f){

}

Powell::~Powell(){
	
}

void Powell::SetFx(FUNC f){
	func=f;
}

void Powell::InitDirec(double **direc){
	for(unsigned int i=0;i!=nv;++i){
		for (unsigned int j = 0; j<nv; j++){
			direc[i][j] = 0;
		}
		direc[i][i] = 1.0;
	}
}
void Powell::InitReverseDirec(double **direc){
	for(unsigned int i=0;i!=nv;++i){
		for (unsigned int j = 0; j<nv; j++){
			direc[i][j] = 0;
		}
		direc[i][i] = -1.0;
	}
}
void Powell::InitRandomDirec(double **direc){
	for(unsigned int i=0;i!=nv;++i){
		for (unsigned int j = 0; j<nv; j++){
			direc[i][j] = randomDouble();
		}
	}
}


double Powell::f1dim2(double alpha, double *x, double *p, double *temp){
	for (int j = 0; j<nv; j++){
		temp[j] = x[j] + alpha * p[j];
	}
	return func(temp);
}

double Powell::Brent(double xa,double xb,double xc,double tol,double& xmin, FUNC1D func1d){
	int done,maxiter = 100;
	const double mintol = 1.0e-11,cgold = 0.381966;
    double rat,fu,r,q,p,xmid,tol1,tol2,a,b;
    double u,etemp,dum,v,w,x,deltax,fx,fv,fw;
    x = w = v = xb;
	fw = fv = fx = func1d(x);
	if (xa < xc){
		a = xa;
		b = xc;
	}
	else{
		a = xc;
		b = xa;
	}
    deltax = 0.0;
    for (int iter = 1; iter<=maxiter; iter++){
		//cout<<" brent iter "<<iter<<endl;
        tol1 = tol * fabs(x) + mintol;
        tol2 = 2.0 * tol1;
		xmid = 0.5 * (a + b);
        if (fabs(x - xmid) <= tol2 - 0.5 * (b - a)){
			break;
		}
        done = -1;
        if (fabs(deltax) > tol1){
            r = (x - w) * (fx - fv);
            q = (x - v) * (fx - fw);
            p = (x - v) * q - (x - w) * r;
            q = 2.0 * (q - r);
            if (q > 0.0){
				p = -p;
			}
            q = fabs(q);
            etemp = deltax;
            deltax = rat;
            dum = fabs(0.5 * q * etemp);
            if (fabs(p) < dum && p > q * (a - x) && p < q * (b - x)){
                rat = p / q;
                u = x + rat;
                if (u - a < tol2 || b - u < tol2){
                    rat = fabs(tol1) * sgn(xmid - x);
                }
                done = 0;
            }
        }
        if (done){
            if (x >= xmid){
                deltax = a - x;
			}
            else{
                deltax = b - x;
            }
            rat = cgold * deltax;
        }
        if (fabs(rat) >= tol1){
            u = x + rat;
		}
        else{
            u = x + fabs(tol1) * sgn(rat);
        }
        fu = func1d(u);
        if (fu <= fx){
            if (u >= x){
                a = x;
			}
            else{
                b = x;
            }
            v = w;
            fv = fw;
            w = x;
            fw = fx;
            x = u;
            fx = fu;
		}
        else{
            if (u < x){
                a = u;
			}
            else{
                b = u;
            }
            if (fu <= fw || w == x){
                v = w;
                fv = fw;
                w = u;
                fw = fu;
			}
            else{
				if (fu <= fv || v == x || v == w){
					v = u;
					fv = fu;
				}
            }
        }
    }
    xmin = x;
    return fx;
}

void Powell::LineSearch(double *x, double *direc){
	int j;
	double tol = 1e-4;
	double fa,f,fb,xb,xa = 0.0;
	double xmin,xx = 1.0;
	double *tempx=new double[nv+1];
	FUNC1D func1d=tr1::bind<double>(&Powell::f1dim2,this,tr1::placeholders::_1,x,direc,tempx);
	Mnbrak(xa, xx, xb, fa, f, fb, func1d);
	x[nv] = Brent(xa, xx, xb, tol, xmin, func1d);
	for (j = 0; j<nv; j++){
		direc[j] = xmin * direc[j];
		x[j] = x[j] + direc[j];
	}
	delete [] tempx;
}

int Powell::evolve(double *x0,double **direc,int maxiter, double ftol, int iter, double terminalLine){
	func(x0);
	int ret=0;
	double *x1=new double[nv+1];
	double *x2=new double[nv+1];
	double *direc1=new double[nv];
	int bigind;
	double t,temp,fx,delta,fx2;
	double &fval=x0[nv];
	while(1){
		for (unsigned int j = 0; j<nv; j++){
			x1[j] = x0[j];
		}
		fx = fval;
		bigind = 0;
		delta = 0.0;
		for (unsigned int i = 0; i<nv; i++){
			fx2 = fval;
			LineSearch(x0, direc[i]);
			if(fval<terminalLine){
				break;
			}
			if (fabs(fx2 - fval) > delta){
				delta = fabs(fx2 - fval);
				bigind = i;
			}
			//cout<<" "<<i<<" vec search"<<fval<<endl;
		}
		if(fval<terminalLine){
			ret=iter;
			break;
		}
		// Construct the extrapolated point
		for (unsigned int j = 0; j<nv; j++){
			direc1[j] = x0[j] - x1[j];
			x2[j] = x0[j] + direc1[j];
		}
		fx2 = func(x2);
		if(fx > fx2){
			t = 2.0*(fx+fx2-2.0*fval);
			temp = fx - fval - delta;
			t *= temp*temp;
			temp = fx - fx2;
			t -= delta*temp*temp;
			if(t<0.0){
				LineSearch(x0, direc1);
				if(fval<terminalLine){
					ret=iter;
					break;
				}
				for (unsigned int j = 0; j<nv; j++){
					direc[bigind][j] = direc[nv-1][j];
					direc[nv-1][j]=direc1[j];
				}
			}
		}
		iter = iter + 1;
		cout<<"Iter = "<<iter<<", func = "<<fval<<endl;
		if (fabs(fx-fval)<ftol){
			ret=iter;
			break;
		}
		//if (2.0*fabs(fx-fval)<=ftol*(fabs(fx)+fabs(fval))+1e-20){
		//	break;
		//}
		if (iter >= maxiter){
			//cout<<"powell exceeding maximum iterations"<<endl;
			ret=0;
			break;
		}
	}
	delete [] direc1;
	delete [] x1;
	delete [] x2;
	return ret;
}


int Powell::Optimize(double *x0, double ftol, int maxiter, int iter, double terminalLine){
	double **direc=new double*[nv];
	direc[0]=new double[nv*nv];
	for(unsigned int i=1;i!=nv;++i)
		direc[i]=direc[i-1]+nv;
	double fbest;
	bool flip=true;
	for(int i=0;i<100;++i){
		if(flip)
			InitDirec(direc);
		else
			InitReverseDirec(direc);
		flip=!flip;
		iter=evolve(x0, direc, maxiter, ftol,iter,terminalLine);
		
		if(fabs(x0[nv]-fbest)<ftol){
			break;
		}
		fbest=x0[nv];
	}
	
	delete [] direc[0];
	delete [] direc;
	return iter;
}

void Powell::Mnbrak(double& xa, double& xb, double& xc, double& fa,
	double& fb, double& fc,FUNC1D func1d){
	double r,q,temp,gold = 1.618034;
	double maxiter=100;
	int glimit = 100;
	double u,ulim,fu,tiny = 1e-30;
	fa = func1d(xa);
	fb = func1d(xb);
	if (fb > fa){
		temp = xa;
		xa = xb;
		xb = temp;
		temp = fb;
		fb = fa;
		fa = temp;
	}
	xc = xb + gold * (xb - xa);
	fc = func1d(xc);
	int iter=0;
	while (fb >= fc){
		iter++;
		if(iter<maxiter)
			break;
		
		r = (xb - xa) * (fb - fc);
		q = (xb - xc) * (fb - fa);
		temp = q - r;
		//cout<<"xa "<<xa<<", xb "<<xb<<", xc "<<xb<<endl;
		//cout<<"fa "<<fa<<", fb "<<fb<<", fc "<<fb<<endl;
		//if(xa>1e10)
		//	system("pause");
		//cout<<"r "<<r<<", q "<<q<<", temp "<<temp<<endl;
		if (fabs(temp) < tiny){
			temp = tiny;
		}
		u = xb - ((xb - xc) * q - (xb - xa) * r) / (2 * temp);
		ulim = xb + glimit * (xc - xb);
		if ((xb - u) * (u - xc) > 0){
			fu = func1d(u);
			if (fu < fc){
				xa = xb;
				fa = fb;
				xb = u;
				fb = fu;
				return;
			}
			else{
				if (fu > fb){
					xc = u;
					fc = fu;
					return;
				}
			}
			u = xc + gold * (xc - xb);
			fu = func1d(u);
		}
		else{
			if ((xc - u) * (u - ulim) > 0){
				fu = func1d(u);
				if (fu < fc){
					xb = xc;
					xc = u;
					u = xc + gold * (xc - xb);
					fb = fc;
					fc = fu;
					fu = func1d(u);
				}
			}
			else{
				if ((u - ulim) * (ulim - xc) >= 0){
					u = ulim;
					fu = func1d(u);
				}
				else
				{
					u = xc + gold * (xc - xb);
					fu = func1d(u);
				}
			}
		}
		xa = xb;
		xb = xc;
		xc = u;
		fa = fb;
		fb = fc;
		fc = fu;
	}
} 