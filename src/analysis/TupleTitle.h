// $Header: /nfs/slac/g/glast/ground/cvs/analysis/analysis/TupleTitle.h,v 1.1.1.1 1999/12/20 22:27:04 burnett Exp $
#ifndef TUPLETITLE_H
#define TUPLETITLE_H 

#include <string>
#include <vector>
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Virtual base class for the templated header client class 
class TitleClientBase {
public:
    TitleClientBase(){}
    virtual std::string addToTitle()const=0;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Template class. If a client has a method, it can add
// itself with the statement
//
//   TupleTitle::instance()->add(new Titleclient<Client>(this));
//
template< class T >
class TitleClient : public TitleClientBase {
public:
    TitleClient(const T* t):m_t(t){}
    std::string addToTitle()const{ return m_t->addToTitle();}
private:
    const T* m_t;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class TupleTitle : private std::vector<TitleClientBase* > {
public:
    TupleTitle(std::string head);
    ~TupleTitle();

    void add(TitleClientBase* client);

    std::string operator()()const;
    // return the result

private:
    std::string m_head;
};
#endif
