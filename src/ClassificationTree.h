/** @file ClassificationTree.h
@brief 


$Header: /nfs/slac/g/glast/ground/cvs/merit/src/ClassificationTree.h,v 1.4 2003/05/15 18:00:23 burnett Exp $
*/
#ifndef CLASSIFICATIONTREE_H
#define  CLASSIFICATIONTREE_H

#include <string>
#include <vector>
#include "classification/Tree.h"

class TupleItem;
class Tuple;

class ClassificationTree 
{
public:
    /** set up the tree:
    * @param t The input tuple -- and will create new columns with the output
    * @param log -- optional stream for ouptut [std::cout]
    * @param xml_file -- IM file containing descriptions of the predict nodes 
    */
    ClassificationTree( Tuple&t, std::ostream& log=std::cout, std::string xml_file="");

    /** run the prediction nodes on the current tuple instance
    */
    void execute();  

    ~ClassificationTree();

private:

    classification::Tree * m_classifier;
    const TupleItem*  m_firstLayer; /// access to the first layer in the tuple
    double m_goodCalProb;
    double m_coreProb; 
    double m_vtxProb ;
    double m_psfErrPred; 

    std::ostream& m_log;
};

#endif