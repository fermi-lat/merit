//$Header: /nfs/slac/g/glast/ground/cvs/analysis/analysis/Tuple.h,v 1.3 2001/12/18 23:15:50 usher Exp $

#ifndef TUPLE_H
#define TUPLE_H

#include <iostream>
#include <string>
#include <vector>

class TupleItem  {
private:
   friend class Tuple;
public:

    TupleItem(){}
    TupleItem(const std::string& name, float x=0);

    //! alternate constructor uses pointer to other buffer
    TupleItem(const std::string& name, float* x);

    void operator=(float x){
      datum=x;

    }
    // assign value to the tuple element
    
    float operator()()const{
      return *m_pdatum;
}

    operator float()const{
      return *m_pdatum;
    }

public:
    std::string name()const{return m_name;}
    // extract the value


    friend std::ostream& operator << (std::ostream&, const TupleItem& );
    // print "name=value" for the tupleitem
    friend std::istream& operator >> (std::istream&,  Tuple& );

   private:

    float& operator()();
    std::string m_name;
    float datum;
    float* m_pdatum;
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

   float operator[](unsigned i)const;
   // access to value of the ith element (start from 0)

   const char* name(unsigned i)const;
   // access to name of the ith element

   int index(const std::string& name)const;
   const_iterator find(const std::string& name)const;
   // return index of element by name, if found (-1 otherwise)
   // return iterator for element with name, end() otherwise

   void fillArray(float array[])const;
   // fill the given array from the tuple

   void writeHeader(std::ostream&)const;

public:
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


};

std::ostream& operator << ( std::ostream&, const Tuple&);

std::istream& operator >> (std::istream&, Tuple&);
// read in values from open stream

#endif
