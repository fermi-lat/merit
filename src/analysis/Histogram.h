// $Header: /nfs/slac/g/glast/ground/cvs/analysis/analysis/Histogram.h,v 1.1.1.1 1999/12/20 22:27:04 burnett Exp $

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "smplstat.h"

#include <string>
#include <iostream>
#include <vector>


class Histogram : public SampleStatistic
{
public:

    Histogram(std::string title="default", double from=0, double to=1, double step=0.1);
    // constructor

    ~Histogram();

    void fill(double x, double w=1.0);
    void operator+=(double x){fill(x, 1.0);}
    // increment the histogram bin associated with the datum

    virtual void clear();
    // clear histogram

    void setRange(double from=0, double to=1, double step=0.1);

    void print(std::ostream& out=std::cout);
    // basic print

    float operator[](int n)const{return m_hist[n];}
    // access to a bin

    float step()const{return m_step;}
    float to()const{return m_to;}
    float from()const{return m_from;}
    // the histogram parameters

    float total()const{return m_total;}
    float under()const{return m_under;}
    float over()const {return m_over;}
    // statistics

    typedef std::vector<float>::const_iterator const_iterator;
    const_iterator begin()const{return m_hist.begin();}
    const_iterator end()const{return m_hist.end();}

    Histogram* next() { return m_next;}

    void    setTitle ( std::string& s ) { m_title = s; }

private:

    std::string m_title;
    double m_from, m_to, m_step;
    unsigned m_bins;
    std::vector <float> m_hist;
    double  m_total, m_under, m_over;

    Histogram * m_next;
};

Histogram* firstHist(); // allow access to start of list
void printAllHists(std::ostream& out =std::cout);
void clearAllHists();
  // operation on the list of all defined histograms
#endif
