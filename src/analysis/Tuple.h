//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/analysis/Tuple.h,v 1.8 2003/11/22 15:34:55 burnett Exp $

#ifndef TUPLE_H
#define TUPLE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

class TupleItem  {
private:
   friend class Tuple;
public:

    TupleItem():m_isFloat(false){}
    TupleItem(const std::string& name, double x=0);

    //! alternate constructor uses pointer to other buffer
    TupleItem(const std::string& name, double* x);

    //! alternate constructor uses pointer to float buffer
    TupleItem(const std::string& name, float* x);

    double value() const{ return m_isFloat? *(const float*)(m_pdatum) : *m_pdatum;}
    
    // for making a root output tuple: must be  double?
    double & value() { return * m_pdatum;}
    // root output
    const float * pvalue() const{ return reinterpret_cast<const float*> (m_pdatum);}

    double operator()()const{      return value(); }

    operator double()const{     return value();    }

    std::string name()const{return m_name;}
    // extract the value


    friend std::ostream& operator << (std::ostream&, const TupleItem& );
    // print "name=value" for the tupleitem
    friend std::istream& operator >> (std::istream&,  Tuple& );

    bool isFloat() const {return m_isFloat;}

private:

    //double& operator()();
    std::string m_name;
    double datum;
    double* m_pdatum; // this might point to a float. Klugy
    bool m_isFloat; // flag for pointer: if float, have to cast.
};


class Tuple : public std::vector<TupleItem* >
{
private:
 friend class TupleItem;

 public:

   Tuple(const std::string& title);
   Tuple(std::istream& in);
   // constructors

   void nextStream(std::istream& in);
   // start reading from a new stream. columns must be the same as currently
    
   void setTitle(const std::string&);
   // set title after the fact

   double operator[](unsigned i)const;
   // access to value of the ith element (start from 0)

   const char* name(unsigned i)const;
   // access to name of the ith element

   int index(const std::string& name)const;
   const_iterator find(const std::string& name)const;
   // return index of element by name, if found (-1 otherwise)
   // return iterator for element with name, end() otherwise

   void fillArray(double array[])const;
   // fill the given array from the tuple

   void writeHeader(std::ostream&)const;

   /**
    @param name1 the value
    @param name2 the key
   */
   void add_alias(std::string name1, std::string name2);

    /// allow ROOT subclass to specify float storage.
    virtual bool isFloat()const{return false;}


   // make a list of the names of the tupleitems

   friend std::istream& operator >> (std::istream&, Tuple&);


   virtual ~Tuple();
   // dtor

   const std::string& title() const {
     return m_title;
   }

   virtual const TupleItem* tupleItem(const std::string& name)const;
   // return pointer to the tuple item by name. Error if not found

   virtual bool nextEvent(){return false;};  // for subclasses
 private:

    std::string m_title;

    static Tuple* s_currentTuple;

    std::map<std::string, std::string> m_alias_list;


};

std::ostream& operator << ( std::ostream&, const Tuple&);

std::istream& operator >> (std::istream&, Tuple&);
// read in values from open stream

#endif
