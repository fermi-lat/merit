// $Id: Tuple.cxx,v 1.1 2002/05/26 03:35:10 burnett Exp $
//
#include "analysis/Tuple.h"

#include <algorithm> // for count
#include <cassert>
#include <cmath>
#include <cfloat>
#include <sstream>


static inline void WARNING(const char* text){ std::cerr << text;}
static inline void FATAL(const char* text) {
    std::cerr << text; throw(text);
}

Tuple::Tuple(const std::string& tname)
: m_title(tname)
{

   s_currentTuple = this;
}
void Tuple::setTitle(const std::string& name)
{
    m_title = name;
}

Tuple::Tuple(std::istream& infile)
{
    s_currentTuple = this;
    char buffer[2000];

    // assume first line is title
    infile.getline(buffer, sizeof(buffer) );
    m_title = buffer;
    if( std::count(m_title.begin(), m_title.end(), '\t')>2 ) {
	// no, has column names
	m_title = "";
    } else {
        // and second line is a list of space or tab-delimited names
	infile.getline(buffer, sizeof(buffer) );
    }
    char* token = strtok( buffer, " \t" );
    while( token != NULL )   {
	new TupleItem(token);  // note that there is no logic to delete these
	token = strtok( 0, " \t" );
    }
}
void Tuple::nextStream(std::istream& infile)
{
    char buffer[2000];

    // assume first line is title
    infile.getline(buffer, sizeof(buffer) );
    if( std::count(m_title.begin(), m_title.end(), '\t')>2 ) {
	// no, has column names
	m_title = "";
    } else {
        // and second line is a list of space or tab-delimited names
        m_title =std::string(buffer);
	infile.getline(buffer, sizeof(buffer) );
    }
    char* token = strtok( buffer, " \t" );
    int i=0;

    while( token != NULL )   {
        if( std::string(token) != name(i++) ){
            FATAL( "new tuple not identical to last");
        }
	token = strtok( 0, " \t" );
    }

}
Tuple*
Tuple::s_currentTuple=0;

float
Tuple::operator[](unsigned i)const
{
    return (*(begin()+i))->datum;
}
const char*
Tuple::name(unsigned i)const
{
    return (*(begin()+i))->name().c_str();
}
int
Tuple::index(const std::string& nam)const
{
    const_iterator look;
    if ((look=find(nam)) == end() ) return -1;
    return look-begin();
}
Tuple::const_iterator
Tuple::find(const std::string& nam)const
{
//## begin Tuple::find%-650323776.body preserve=yes
    const_iterator it=begin();
    for(;  it !=end(); ++it) {
	if( nam==(*it)->name() )break;
    }
    return it;
//## end Tuple::find%-650323776.body
}
void
Tuple::fillArray(float array[])const
{
    for( const_iterator it=begin(); it !=end(); ++it)
      *array++ = (*it)->datum;
}

void
Tuple::writeHeader(std::ostream& os)const
{

    os << m_title << '\n';
    if( !size() ){
	WARNING("Tuple::writeHeader--no items in tuple!");
	return;
    }
    for( const_iterator it=begin(); it !=end(); ++it) {
	  os <<  (*it)->name() << " \t";
    }
    os << '\n';
}

std::ostream& operator << (std::ostream& out, const Tuple& t)
{
    unsigned len = t.size();
    if( !len) {
	WARNING("Attempt to write empty tuple!");
	
    }else {

	for( Tuple::const_iterator it=t.begin(); it !=t.end(); ++it)
	    out << float(**it) << '\t';
    }
    out << '\n';
    return out;
}

std::istream& operator >> (std::istream& in, Tuple& t)
{
    float x;
    for( Tuple::iterator it = t.begin(); it != t.end(); ++it) {
	in >> x;
	if( in.fail() ){  // protect against 1.#INF!
	    if( in.eof() ) break;
	    in.clear();
	    char temp[200];

	    if( it == t.begin()) {
		do {
		    // non-numeric at beginning of line. just read until OK
		    in.clear();
		    in.getline(temp, sizeof(temp) , '\n');
		    std::cerr << "Text found & skipped: \""
			      << temp << "\"" << std::endl;
		    in >> x;
		} while ( in.fail() && ! in.eof() );
	    } else {

		in.getline(temp, sizeof(temp), '\t');
		if( fabs(x)==1 && temp[0]=='#') { // what we get on the PC
		    --it; //back up because stopped at the #
		    std::cerr << "Bad value for \"" <<
		      (*it)->name() << "\": " << temp << std::endl;
		    x= FLT_MAX;
		}
		else if( temp[0]=='N' || temp[0]=='I' ||
			 (temp[0] =='-' && temp[1] == 'N') ){
		    x = FLT_MAX;
		}  // maybe it is 'NaN' or 'Infinity' (gcc)
		else  {
		    std::cerr << "Error message found: \"" << temp
			      << "\"" << std::endl;
		    std::cerr << "Can't recover, exiting" << std::endl;
		    exit(-1);		
		}
	    }
 	}
	(*it)->datum = x;
    }
    return in;
}

TupleItem::TupleItem(const std::string& iname, float x)
: m_name(iname), m_pdatum(&datum), datum(x)
{

    if( Tuple::s_currentTuple==0 )
	FATAL("TupleItem, but no Tuple defined");

    // replace one blank with an underscore
    std::string::size_type i = name().find(' ');
    if( i != std::string::npos){
        m_name[i] = '_';
    }
    // die if another
    i = m_name.find(' ');

    if( i != std::string::npos){
        std::string fatal("TupleItem: bad item name, \"");
        FATAL((fatal+iname+"\" ").c_str());
    }
    Tuple::s_currentTuple->push_back(this);
}

TupleItem::TupleItem(const std::string& iname, float* px)
: m_name(iname), m_pdatum(px)
{
    Tuple::s_currentTuple->push_back(this);
}


float &
TupleItem::operator()(){
    return *m_pdatum;
}


std::ostream& operator<< (std::ostream& out, const TupleItem& t)
{
   out << t.name() << '=' << *t.m_pdatum;
   return out;
}


const TupleItem*
Tuple::tupleItem(const std::string& name)const
{
    Tuple::const_iterator it = find(name);
    if( it != end() ) return *it;
#ifndef DEFECT_NO_STRINGSTREAM
    std::stringstream  errmsg;
    errmsg << "Sorry, did not find '" << name << "' in the tuple\n";
    std::cerr << errmsg << std::endl;
    throw (errmsg.str());
#else
    std::cerr << "Did not find a tuple item " << std::endl;
    throw("Tuple::tupleItem -- no tuple item");
#endif
    return *it;
}

Tuple::~Tuple()
{
    iterator it = begin();
    while( it != end() )
	delete *it++;
}
