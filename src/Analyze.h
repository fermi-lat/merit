// Analyze.h

#ifndef ANALYZE_H
#define ANALYZE_H

#include "analysis/Tuple.h"

#include <iosfwd>

//====================================================================================
class Analyze {
    // base class for analysis. Provides setup for simple analysis of a TupleItem
    // and formating functions for output
public:
    Analyze(std::string name="Generic count") : m_count(0),m_tupleItem(0),m_name(name)
                                                , m_seen(0) {}
    Analyze(const Tuple& t, const std::string& item_name, const std::string& cut_name="");

    Analyze(const TupleItem&  item, const std::string& cut_name="");

    virtual ~Analyze() {}

    bool	    operator() () { bool b = apply(); if (b) m_count++; m_seen++; return b; }
    //

    unsigned	         count ()  const { return m_count; }
    unsigned             seen ()   const { return m_seen; }
    float	             item()    const { return * m_tupleItem;}
    const std::string&   name()    const { return m_name; }

    const char*          getName()       { return m_name.data();}
    const char*          getItem();

    void set_name(std::string newname){m_name=newname;}
    void set_tuple_item(const Tuple&t, std::string item_name);

    virtual void         report(std::ostream& out);

    virtual void         clear() {m_count=0;}

    static void	         separator(std::ostream& out, char ='-');
    // make a separator line for output
    static std::string   make_label(const std::string&);
    // format a label for tablular output
    static bool          showperc () { return s_showperc; }
    // are percentages shown in readout?
    static void          showperc ( bool s ) { s_showperc = s; }
    // set the show percentages on display flag

private:
    virtual bool      apply() { return true; }
    // default analysis function. Note private, should only be accessed by operator()
    // subclass should make private also.

    unsigned	      m_count;	      // keep track of success
    const TupleItem*  m_tupleItem;    // the tuple item to analyze or cut.
    std::string	      m_name;	      // name to describe this

    unsigned          m_seen;         // number of events seen (for making ratios)

    static bool       s_showperc;     // flag to determine whether percentages are shown
};

#endif
