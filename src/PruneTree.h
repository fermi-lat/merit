/** @file PruneTree.h
     @brief apply the set of Bill's cuts for pruning background 
*/


#ifndef PRUNETREE_H
#define  PRUNETREE_H

#include <string>
#include <vector>
#include "classification/Tree.h"

class TupleItem;
class Tuple;

class PruneTree {

    public:
    /** set up the tree:
    * @param t The input tuple -- and will create new columns with the output
    * @param log -- optional stream for ouptut [std::cout]
    * @param xml_file -- IM file containing descriptions of the predict nodes 
    */
    PruneTree( Tuple&t, std::ostream& log=std::cout, std::string xml_file="");

    /** run the prediction nodes on the current tuple instance
    */
    void execute();  

    ~PruneTree();

private:
    classification::Tree * m_classifier;

    class Lookup;
    Lookup* m_lookup;

    class PreClassify;
    PreClassify * m_preclassify;

    std::ostream& m_log;
};
#endif



