/** @file PruneTree.h
     @brief declare PruneTree
*/
#ifndef PRUNETREE_H
#define  PRUNETREE_H

#include <string>

// forward class declarations
namespace classification { class Tree;}
class Tuple;

/** @class PruneTree
     @brief apply the set of Bill's cuts for pruning background 

     @authors Traudl Hansl-Kozanecka, Toby Burnett

     Usage:
     @verbatim
    Tuple tuple; 
     PruneTree pt(tuple);
     [...]
     double prob = pt();
     @endverbatim
  
*/

class PruneTree {

    public:
    /** set up the tree:
    * @param t The input tuple
    * @param xml_file -- IM file containing descriptions of the predict nodes 
    */
    PruneTree( Tuple&t,  std::string xml_file="");

    /** run the prediction nodes on the current tuple instance, return the value of the 
        selected CT, a gamma probability
    */
    double operator()(); 

    ~PruneTree();

private:
    classification::Tree * m_classifier;

    // nested helper classes,  only declare here
    class Lookup;

    class PreClassify;
    PreClassify * m_preclassify;

};
#endif



