// PSFRootFit.cxx

#include "PSFRootFit.h"
#include <fstream>
#include <iomanip>
#ifdef WIN32
#include <float.h>
#endif
using namespace std;

PSFRootFit::PSFRootFit(const PSFanalysis& psfanal)
	:m_psfanal(psfanal) 
{}


void PSFRootFit::report(ostream& out)
{
    rootFit(out);
}

double PSFRootFit::rootFit(ostream& returnstr){
    //TODO: TAKE THE OUTPUT OUT OF THE ROOT FILE, 
    //AND SEND IT TO THE returnstr.
    //set up the PSF file.
    std::ostream* m_out;
    m_out = new std::ofstream("PSFHist.out");
        std::ostream& out = *m_out;
        // form an ntuple text file representing the histogram
        Histogram::const_iterator h = m_psfanal.Histogram::begin();
    for(; h != m_psfanal.Histogram::end(); ++h) {
        out<< *h <<'\t'<<std::endl;
    }


    std::ofstream out_file("psfmake.cxx");    
    out_file.clear();
    
    out_file << 
        "{\n"
        
        "  FILE *fp;\n"
        "  float ptmp,p[20];\n"
        "  int i, iline=0;\n"; 
    
    out_file <<
        "  TH1D *hist2 = new TH1D(" << '"' << "hist2" << '"' << "," << '"' << "PSF Output" << '"' << "," << (m_psfanal.Histogram::to()-m_psfanal.Histogram::from())/m_psfanal.Histogram::step() << "," << m_psfanal.Histogram::from() << "," << m_psfanal.Histogram::to() << ");\n";  
    
    out_file <<
        "int j=0;\n"
        "  fp = fopen(" << '"' <</* "../src/" <<*/ "PSFHist.out" << '"' << "," << '"' << "r" << '"' << ");\n"
        
        "  while ( fscanf(fp," << '"' << "%f" << '"' << ",&ptmp) != EOF ){\n"
        "    p[i++]=ptmp;\n"
        "    if (i==1){\n"
        "      i=0; \n"
        "      iline++;\n";
        out_file<<
            "  int fillNum = j * " << m_psfanal.Histogram::step() <<";\n"
        "     hist2->Fill(j * " << m_psfanal.Histogram::step() <<", p[0]);\n";

    out_file <<
        "      if(p[0]==1.0000000)break; \n"
        "      j++;\n"
        "    }\n"
        "  }\n"
        
        ///normalize the histogram:
        " Double_t scale=1/hist2->Integral();\n"
        " hist2->Scale(scale);\n"

        ///find the 68% and 95% levels
        " Double_t lev68 = 0;\n"
        " Double_t lev95 = 0;\n"
        " Double_t perc68 = 0;\n"
        " while(hist2->Integral(0,lev68)< 0.68*hist2->Integral() )lev68 += 1; \n"
        //" printf(" << '"' << "68 percent level is %f, integral is %f \n" << '"' << ",(lev68*180)/(100*3.14),hist2->Integral(0,lev68));} \n"
        " while(hist2->Integral(0,lev95)< 0.95*hist2->Integral() )lev95 += 1; \n"
        " while(hist2->Integral(0,perc68)< 0.68*hist2->Integral() )perc68 += 0.001; \n"
        

        ///set-up output:
        " printf(" << '"' << "Fitting Output:------------------------------------------\n" << '"' << "); \n"
        ///get the first bin of the histogram
        " Double_t bin1 = hist2->GetBinContent(1);\n"
        //" printf(" << '"' << "bin1 is %f \n" << '"' << ",bin1); \n"
        

        //and here's some output
        " printf(" << '"' << "68 percent level from hist is %f \n" << '"' << ",(lev68)*("<< m_psfanal.Histogram::step() <<")); \n"
        " printf(" << '"' << "95 percent level from hist is %f \n" << '"' << ",(lev95)*("<< m_psfanal.Histogram::step() <<")); \n"
        " if(lev68>0)printf(" << '"' << "95/68 ratio from hist is %f \n" << '"' << ",(lev95)/(lev68)); \n"


        //here's the bit where we define the function for fitting, and do it:
        //" TF1 *f1 = new TF1(" << '"' << "f1" << '"' << ", " << '"' << " [0]*(1/1)*exp((-x)/(2*[1]*[1]))" << '"' << ",0.0000,0.0005);\n"
        //" f1->SetParameters(100.0,0.001);\n"
        " TF1 *f2 = new TF1(" << '"' << "f2" << '"' << ", " << '"' << " [0]*exp((-x)/(2*[1]*[1])) + [2]*exp((-x)/(2*[3]*[3]))" << '"' << ",0.0,0.02);\n"
        " f2->SetParameters(bin1,0.00001, bin1/20.0, 0.0001);\n"
        //" TF1 *f3 = new TF1(" << '"' << "f3" << '"' << ", " << '"' << " [0]*(1/([1]*x)) + [2]*(x/([3]*[3]))*exp((-x*x)/(2*[3]*[3]))" << '"' << ",0,1);\n"
        //" f3->SetParameters(10,10, 0.1, 0.01);\n"
        //" TF1 *f4 = new TF1(" << '"' << "f4" << '"' << ", " << '"' << " [0]*(1/(x))" << '"' << ",0.0005,0.01);\n"
        //" f4->SetParameters(10,10);\n"


        " hist2->Fit(" <<'"'<<"f2"<<'"'<<", " << '"' << "0QR" << '"' << ");\n"
        

        ///find the 68% and 95% levels from the fitted function
        " Double_t fnlev68 = 0;\n"
        " Double_t fnlev95 = 0;\n"
        " while(f2->Integral(0,fnlev68)< 0.68*f2->Integral(0.0,0.01) )fnlev68 += 0.000001; \n"
        " while(f2->Integral(0,fnlev95)< 0.95*f2->Integral(0.0,0.01) )fnlev95 += 0.000001; \n"

        " printf(" << '"' << "68 percent level from fit is %f \n" << '"' << ",fnlev68); \n"
        " printf(" << '"' << "95 percent level from fit is %f \n" << '"' << ",fnlev95); \n"
        " if(fnlev68>0)printf(" << '"' << "95/68 ratio from fit is %f \n" << '"' << ",(fnlev95)/(fnlev68)); \n"

        ///then get the fit parameters:
        " TF1 *fit = hist2->GetFunction(" <<'"'<<"f2"<<'"'<<");\n"
        " Double_t param0=fit->GetParameter(0);\n"
        " Double_t param1=fit->GetParameter(1);\n"
        " Double_t param2=fit->GetParameter(2);\n"
        " Double_t param3=fit->GetParameter(3);\n"
        " printf(" << '"' << "parameter #0: %f\n" << '"' << ",param0); \n"
        " printf(" << '"' << "parameter #1: %f\n" << '"' << ",param1); \n"
        " printf(" << '"' << "parameter #2: %f\n" << '"' << ",param2); \n"
        " printf(" << '"' << "parameter #3: %f\n" << '"' << ",param3); \n"
        
        " printf(" << '"' << "-------------------------------------------------------\n" << '"' << "); \n"
        
        //draw the histogram
        //"  hist2.Draw(" << ");\n"
        
        "}\n";
        //".q;\n";
    out_file.close();
    system("root -b -q -n -l psfmake.cxx");
    return 0.0;

}