//  $Header: /cvs/glastsim/merit/Resolution.h,v 1.6 1999/04/26 17:16:32 pfkeb Exp $
//  Project: glast analysis
//   Author: Sawyer Gillespie
//
//  Master class for GLAST instrument resolution calculations.
//	Integrates the effective area and FOM over the entire visible
//	sky (from instrument perspective) to yield an overall resolution
//	measurement for the instrument. This class will also evaluate and
//	output (to a file) the FOM for given solid angle elements of the
//	visible sky, giving a relative 'vision' factor and FOM for each
//	element. If m_out is not initialized, then this latter output is skipped.
//	This class handles some of the more global calculation (summing)
//	but relies on ResElement objects to determine FOM and Aeff.



#ifndef RESOLUTION_H
#define RESOLUTION_H

#include <iostream>
#include <vector>
#include "FigureOfMerit.h"
#include <math.h>

//============================================================================
typedef	std::vector< FigureOfMerit >	_Elements;

inline double sqr(double x) {return x*x;};

class Resolution {
public:
    Resolution(const Tuple&, std::ostream*, std::string);
    ~Resolution();

    void clear();
    void execute();
    // data manipulation

    Resolution*	instance() const {return s_instance;}

    void report(std::ostream&);
	// output

private:
    static const TupleItem* findEntry(const Tuple& t, const char* name);

    const Tuple& m_tuple;
    // reference to the Glast tuple

	std::ostream*	m_out;
	// output file stream

	static Resolution*			s_instance;
	// last instance of this class

	static FigureOfMerit			*s_master;
	static	_Elements*			s_elements;	
	static double				s_min,	s_max;
	static unsigned				s_bins;
	static double				s_step;
	static const TupleItem*			s_xdir;
	static const TupleItem*			s_ydir;
	static const TupleItem*			s_zdir;
	static double				s_area;
	static float				s_Aeff;
	static unsigned				s_generated;
	static unsigned				s_under, s_over;
	// private class statics
	// note: elements, bins, etc. declared static to allow multiple tuples to be
	//		 placed into a single data storage element.

	double	p_costheta() const {return - *s_zdir/sqrt(sqr(*s_xdir)+sqr(*s_ydir)+sqr(*s_zdir));}
};
#endif

