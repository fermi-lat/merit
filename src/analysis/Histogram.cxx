// file: Histogram.cxx
// histogra.cpp,v 1.1.1.1 1994/04/18 18:12:27 burnett Exp

#include "analysis/Histogram.h"

#include <math.h>
#include <cstdio>
#include <assert.h>

using namespace std;

Histogram* first=0;
// pointer to the Histogram list

Histogram* firstHist(){return first;}

Histogram::Histogram(std::string title, double from, double to, double step)
  : SampleStatistic()
  , m_title(title)
  , m_from(from)
  , m_to(to)
  , m_step( step? step : fabs(to-from)/25.0)
{
    setRange(from, to , step);

    // setup linked list
     if( first==0 )
	first = this;
     else {
	 Histogram* h= first;
	 while( h->m_next)
	     h=h->m_next;
	 h->m_next = this;
     }

      m_next = 0;
}
void Histogram::setRange(double from, double to, double step)
{
  m_from = from;
  m_to = to;
  m_step =  step? step : fabs(to-from)/25.0;

  m_bins = unsigned(0.5+fabs(m_to-m_from)/m_step);
  m_hist.resize(m_bins);
  clear();
}


/*void
Histogram::printAll(const char* filename)
{
  ofstream file(filename);
//  if( file.bad() ){
//    WARNING("Histogram: could not open file");
    assert(~file.bad());

 //   return;
  //}
  printAll(file);
  file.close();
}
*/
void
clearAllHists()
{
    for(Histogram* h = first; h; h = h->next())
	h->clear();
}
Histogram::~Histogram()
{
}
void Histogram::clear()
{
     for(unsigned i=0; i< m_bins; i++)
	   m_hist[i]=(float)0.0;
    m_total=m_under=m_over=0;
    SampleStatistic::reset();
}
void
printAllHists(ostream& cout)
{
    for(Histogram* h = first; h; h = h->next())
	h->print(cout);
}

void
Histogram::fill(double x, double w)
{
    unsigned index ;
    m_total+=w;
    if( x<m_from) m_under+=w;
    else if( (index = (unsigned)((x-m_from)/m_step)) >= m_bins) m_over+=w;
    else (m_hist[index])+=(float)w;

    SampleStatistic::operator+=(x);
}
void Histogram::print(ostream& cout)
{
    ostream& os = cout;
    os << "---------------------------------\n";
    os << m_title << '\n';
    os << m_from << ',' << m_to << ',' << m_step << '\n';
    os << "Total: " << total()
       << ", under: " << under()
       << ", over: "  << over() << '\n';
    os <<  "Mean: " << mean()
       << ", RMS: " << stdDev()
       << ", Min: " << min()
		 << ", Max: " << max() << '\n';
    double x = m_from;
    os << "Left Edge   Contents\n";
    for(unsigned i = 0; i< m_bins ; i++) {
	char buff[40];
	if( fabs(x) < m_step*1.e-5) x=0;
	sprintf(buff,"%10.5g%10.0f\n", x, m_hist[i]);
	os << buff; x+=m_step;
    }
    os.flush();
}
