/** @file ClassificationTree.h
@brief 


$Header: /nfs/slac/g/glast/ground/cvs/merit/src/ClassificationTree.h,v 1.2 2003/05/09 22:32:51 burnett Exp $
*/
#ifndef CLASSIFICATIONTREE_H
#define  CLASSIFICATIONTREE_H

#include <string>
#include <vector>
#include "classification/Tree.h"

class Tuple;

class ClassificationTree 
{
public:
    /** set up the tree:
    * @param t The input tuple -- will create a new column with the output
    * @param xml_file 
    */
    ClassificationTree( Tuple&t, std::ostream& log, std::string xml_file="");
    /** run the classification
    */
    void execute();  

    ~ClassificationTree();

private:
    classification::Tree * m_classifier;
    std::ostream& m_log;
};

#endif