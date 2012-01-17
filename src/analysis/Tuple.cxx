// $Id: Tuple.cxx,v 1.20 2006/10/24 08:01:57 burnett Exp $
//
#include "analysis/Tuple.h"

#include <algorithm> // for count
#include <cassert>
#include <cmath>
#include <cstring>
#include <cfloat>
#include <sstream>
#include <iomanip>
#include <stdexcept>

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

double
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
namespace {
    // comparison predicate
    class uncase_compare{
    public:
        uncase_compare(const std::string& s):m_left(s){}
        
        bool operator==( const std::string& right){
            int s1=m_left.size(), s2 = right.size();
            if( s1!=s2) return false;
            for(int i = 0; i< s1; ++i){
                if( ::tolower(m_left[i]) != ::tolower(right[i]) ) return false;
            }
            return true;
        }
        
        const std::string& m_left;
    };
}


Tuple::const_iterator
Tuple::find(const std::string& nam)const
{
    uncase_compare check(nam);
    const_iterator it=begin();
    for(;  it !=end(); ++it) {
        if( check==(*it)->name() )break;
        //if( nam==(*it)->name() )break;
    }
    if( it== end()){    // try alias
        std::map<std::string, std::string>::const_iterator sit=m_alias_list.find(nam);
        if( sit != m_alias_list.end() ) {
            const std::string& alias= sit->second;
            it = find( alias );
        }
    }
    return it;
}

void
Tuple::fillArray(double array[])const
{
    for( const_iterator it=begin(); it !=end(); ++it)
        *array++ = (*it)->datum;
}

void
Tuple::writeHeader(std::ostream& os)const
{
    
    if( !size() ){
        WARNING("Tuple::writeHeader--no items in tuple!");
        return;
    }
    if(! m_title.empty()){ os << m_title << '\n'; }
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
        out << std::setprecision(6);
        for( Tuple::const_iterator it=t.begin(); it !=t.end(); ++it);
#if 0
            out << double(**it) << '\t'; //<<<<<<
#endif
    }
    out << '\n';
    return out;
}

std::istream& operator >> (std::istream& in, Tuple& t)
{
    double x;
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
                    throw("merit cannot read");
                }
            }
        }
        (*it)->datum = x;
    }
    return in;
}

TupleItem::TupleItem(const std::string& iname, double x)
: m_name(iname), datum(x), m_pdatum(&datum), m_type(DOUBLE)
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

TupleItem::TupleItem(const std::string& name, float* x)
: m_name(name), m_pdatum((double*)x),m_type(FLOAT) //m_isFloat(true)
{
    Tuple::s_currentTuple->replaceOrAdd(this);
//    Tuple::s_currentTuple->push_back(this);
}

TupleItem::TupleItem(const std::string& name, int* x)
: m_name(name), m_pdatum((double*)x), m_type(INT) //m_isFloat(true)
{
    Tuple::s_currentTuple->replaceOrAdd(this); //<===== fixme
}



TupleItem::TupleItem(const std::string& iname, double* px)
: m_name(iname), m_pdatum(px), m_type(DOUBLE) //m_isFloat(false)
{
    Tuple::s_currentTuple->replaceOrAdd(this);
//    Tuple::s_currentTuple->push_back(this);
}
double TupleItem::value() const
{
    switch( m_type) {
        case FLOAT:  return *(const float*)(m_pdatum);
        case INT:    return *(const int*)(m_pdatum);
        case DOUBLE: return *m_pdatum;
        default:    throw std::runtime_error("TupleItem::value: unexpected type");
    }
    return 0;
}


std::ostream& operator<< (std::ostream& out, const TupleItem& t)
{
    out << t.name() << '=' << *t.m_pdatum;
    return out;
}

void Tuple::add_alias(std::string name1, std::string name2)
{
    // create, or find, the tuple item
    //? const TupleItem * ti = tupleItem(name1);
    if( find( name1) == end() ){
        std::stringstream  errmsg;
        errmsg << "Tuple::add_alias: did not find '" << name1 << "' in the tuple\n";
        std::cerr << errmsg.str() << std::endl;
        throw std::runtime_error(errmsg.str());
    }
    m_alias_list[name2]=name1;
}

const TupleItem*
Tuple::tupleItem(const std::string& name)const
{
    // make lowercase version
    Tuple::const_iterator it = find(name);
    // compare lowercase only
    if( it != end() ) return *it;
    
    // try alias
    std::map<std::string, std::string>::const_iterator sit=m_alias_list.find(name);
    if( sit != m_alias_list.end() ) {
        const std::string& alias= sit->second;
        it = find( alias );
        if( it != end() ) return *it;
    }
    
    
    // here if not found
    std::stringstream  errmsg;
    errmsg << "Did not find '" << name << "' in the tuple\n";
    throw std::runtime_error(errmsg.str());
    return *it;
}

void Tuple::replaceOrAdd(TupleItem * item)
{

    int i = index(item->name());
    if( i == -1 ) {
        push_back(item);
    }else{
        //? delete (*this)[i];
        this->at(i) = item;
    }
}



Tuple::~Tuple()
{
    iterator it = begin();
    while( it != end() )
        delete *it++;
}
