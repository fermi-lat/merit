// Statistic.h

#ifndef STATISTIC_H
#define STATISTIC_H

#include "Analyze.h"

#include "analysis/smplstat.h"

class Statistic : public Analyze {
	// basic Analyze class that makes a simple table of statistics
public:
    Statistic(const Tuple& t, const std::string& item_name, std::string title)
    	: Analyze(t, item_name, title){}
	
    Statistic(const Tuple&t, std::string::const_iterator& it, std::string::iterator end);

    virtual void    report(std::ostream& out);
    void clear() { m_stat.reset(); }
    const SampleStatistic& stat() const {return m_stat;}
private:
    virtual bool    apply ()
    {
	m_stat += item();
	return true;
    }
    SampleStatistic m_stat;
};

#endif
