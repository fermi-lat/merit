// PSFanalysis.cxx
// June 12, 2001 - rename variables to TKR_...  TU

#include "PSFanalysis.h"

#include <cmath>
#include <iomanip>
#ifdef WIN32
#include <float.h>
#endif
using namespace std;

inline static double sqr(double x){return x*x;}

PSFanalysis::PSFanalysis()
	: Analyze()
	, RebinHist("PSF", 0, 3.0, 0.1 )
	, m_sigma(-1)
        , m_energy()
        , m_first_layer()
        , m_emin(0.1), m_emax(100.)
{}

PSFanalysis::PSFanalysis(const Tuple& t, double emin, double emax)
	: Analyze(t, "MC_Gamma_Err" , "PSF analysis" )
	, RebinHist("PSF", 0, 3.0, 0.1 )
	, m_sigma(-1)
        , m_energy(t, "MC_Energy", "energy")
        , m_first_layer(t, "TKR_First_XHit", "first hit layer")
        , m_emin(emin), m_emax(emax)
{}

bool  PSFanalysis::apply ()
{

    m_energy();
    double e = m_energy.item();
    if( m_emax > m_emin && (e < m_emin || e>m_emax)) return true;
    m_loge += log(e);

    double  theta_squared = sqr(item());
#ifdef WIN32 
    if(_finite(theta_squared)) {  // Win32 call available in float.h
#else
        if (isfinite(theta_squared)){ // gcc call available in math.h
#endif

        fill(theta_squared);
    return true;
        }
    else { return false; }

}
void PSFanalysis::clear()
{
    Analyze::clear();
    RebinHist::clear();
    m_sigma = -1;
}

unsigned PSFanalysis::count () const
{
    return static_cast<unsigned int>(Histogram::total()) ;
}


double PSFanalysis::percentile(double percent)
{
    return sqrt(RebinHist::percentile(percent));
}
double PSFanalysis::sigma()
{
    if( m_sigma >=0) return m_sigma;

    // rebin according to mean
    double mean = Histogram::mean();
    double binsize = mean/25;
    RebinHist::rebin(0, 200*binsize, binsize);
	//out << "Sigma from mean: " << 0.5*sqrt(mean) << '\n';

    // make sure first bin not too full (less than 20% of total)
    while( (*this)[0] > 0.2* Histogram::total() && binsize>1e-6) {
        binsize *= 0.5;
        RebinHist::rebin(0, 200*binsize, binsize);
    }
	//out << "\nHistogram (binsize= " << binsize << ')';
	//for(int i=0; i<20;i++) out << m_psf_hist[i] << ", "; out << "...\n";
	
    // form estimate of sigma (effective) from sum of bin contents squared
    Histogram::const_iterator h = Histogram::begin();
    double sum1=0, sum2=0;
    for(; h != Histogram::end(); ++h) {
	    sum1 += *h;
	    sum2 += sqr(*h);
    }
    return  (m_sigma =sum1*sqrt(Histogram::step() / sum2 )/2.0 );
}

void PSFanalysis::report(ostream& out)
{
    if( Histogram::total()==0 ) return;

    // calculate stuff we need
    double sig = sigma();
    double ang68 = percentile(68);
    double ang95 = percentile(95);

    if( m_emax > m_emin) {
        out << endl << Analyze::make_label("PSF:            Events")
            << setw(6)<< setprecision(6)<< Histogram::total() ;
        out << endl << Analyze::make_label("exp<log(E)>") 
            << setprecision(3)<<setw(6) << exp(m_loge.mean()) << " GeV";
    }

    out	<< "\n" << Analyze::make_label("eff. proj. sigma")
	<< setw(6) << setprecision(3)
	<< sig*180/3.14159 << " deg = "
	<< sigma()*180*60/3.14159 << " arc-min";

    out <<  "\n" << Analyze::make_label("68% contained")
	<< setw(6) << ang68*180/3.14159 << " deg = "
	<< setw(4) << ang68/sig/1.51 << "*(1.51*sigma)" ;

    out	<< "\n" << Analyze::make_label("95% contained")
	<< setw(6) << ang95*180/3.14159 << " deg = "
	<< setw(4) << ang95/sig/2.447 << "*(2.45*sigma)" ;

}
void PSFanalysis::row_report(ostream&out)
{
    out << '\n';
    out<< setprecision(6)<< Histogram::total()  << '\t' ;
    if( Histogram::total()>10) {
        out << setw(6) << setprecision(3)
            << exp(m_loge.mean()) << '\t' 
            << sigma()*180/3.14159    << '\t'
            << percentile(68)*180/3.14159  << '\t'
            << percentile(95)*180/3.14159;
    }
}
