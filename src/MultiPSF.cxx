// $Header: /nfs/slac/g/glast/ground/cvs/merit/src/MultiPSF.cxx,v 1.3 2002/12/21 21:44:57 srobinsn Exp $

#include "MultiPSF.h"
#include "FigureOfMerit.h"
#include <iomanip>
#include <cmath>
using std::ostream;
using std::endl;

static  int n_ebins=6;
static double sqr(double x){return x*x;}

MultiPSF::MultiPSF(const Tuple& t, char code)
:Analyze(t,"McZdir","accepted for multi-PSF")
, m_bin_size(0.) // default bins size
{
    static double factor = sqrt(10.);
    static double start=0.01*sqrt(factor);


    switch (code) {
    case '0': case '1': case '2': case '3': case'4': 
        {

            m_bin_size = 0.2;
            float b = 1.0-m_bin_size*(code-'0');
            m_costheta_bin.push_back(b);
            m_costheta_bin.push_back(b-m_bin_size);
            for(float e1=start; e1<10.1; e1*=factor) {
                push_back(new PSFanalysis(t,e1, e1*factor));
            }

        }break;
        case 'n':  // normal only, make one big bin so normalizes OK
            {
                m_bin_size=1.0;
                m_costheta_bin.push_back(1.0);
                m_costheta_bin.push_back(0);
                for(float e1=start; e1<10.1; e1*=factor) {
                    push_back(new PSFanalysis(t,e1, e1*factor));
                }
            }                
        break;
        case 'a': m_bin_size=0.1; // all bins 1 -> 0.2
        case 'b': m_bin_size+=0.1;// all bins 1 -> 0.2
            {
                float b = 1.0;
                for(; b>0.201; b-=m_bin_size) {
                    m_costheta_bin.push_back(b);
                    for(float e1=start; e1<10.1; e1*=factor) {
                        push_back(new PSFanalysis(t,e1, e1*factor));
                    }
                }
                m_costheta_bin.push_back(b);
                break;
            }
        case 'i': // isotropic case 
            {
                n_ebins=1;
                m_bin_size=0.1;
                float b = 1.0;
                for(; b>0.201; b-=m_bin_size) {
                    m_costheta_bin.push_back(b);
                    push_back(new PSFanalysis(t));
                }
                m_costheta_bin.push_back(b);
            }
        default: break;
    }
    
}


bool MultiPSF::apply()
{
    float zdir = -item(); // get costheta
    unsigned int n = 0;
    for(; n< m_costheta_bin.size()-1; ++n) {
        if( zdir <= m_costheta_bin[n] && zdir> m_costheta_bin[n+1]){
            for( unsigned int k = n_ebins*n; k< n_ebins*(n+1); ++k) 
                (*this)[k]->operator()();
            return true;
        }
    }
    return true;
}

void MultiPSF::report(ostream& out)
{
    Analyze::report(out);
    //for( iterator it = begin(); it != end(); ++it) (**it).report(out);
    iterator it= begin();
    double sum=0, sumw=0;
    if( n_ebins==1){
        // setup if one energy
        Analyze::separator(out);
        out << "\nEvents\tenergy\tsigma\t68%\t95%\tAeff\tcosth";
        Analyze::separator(out);
    }

    for(unsigned int n=0; n< m_costheta_bin.size()-1; ++n) {
        
        double areaPerEvent = 
            FigureOfMerit::area()/1e4   // area in meter**2
            /FigureOfMerit::generated(); 
        // now adjust for bining. (Would be better to use actually generated numbers in each bin)
        areaPerEvent *=
            n_ebins // n e-bins
            /(m_costheta_bin[n]-m_costheta_bin[n+1]); // fraction of solid angle
        if( n_ebins>1) {
            Analyze::separator(out);
            out << "\ncostheta Range: " << m_costheta_bin[n] 
                << " - "<<  m_costheta_bin[n+1];
            out << "\nEvents\tenergy\tsigma\t68%\t95%\tAeff(m^2)";
        }
        for(int m=0; m<n_ebins; ++m){
            PSFanalysis& psf = **it++;
            psf.row_report(out);
            out << '\t' << psf.count()*areaPerEvent;
            if( n_ebins==1) {
                out << '\t' << 0.5*(m_costheta_bin[n]+m_costheta_bin[n+1]);
                sum += psf.count();
                sumw += psf.count()/sqr(psf.sigma());
            }
        }
    }
    if( n_ebins ==1) { 
        // analysis of FOV for unique energy
        Analyze::separator(out);
        out << " \nField of View";
        out << endl << Analyze::make_label("Total events") 
            << static_cast<int>(sum);
        double intercept = (3.*(*this)[0]->count()-(*this)[1]->count())/2.;
        out << endl << Analyze::make_label("FOV") << sum/intercept*m_bin_size*2*3.14159 << " sr";
        out << endl << Analyze::make_label("Weighted FOV") 
            << sumw/(intercept/sqr((*this)[0]->sigma())) * m_bin_size *2.*3.14159 << " sr";

    }
}

