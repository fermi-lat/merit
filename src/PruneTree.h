/** @file PruneTree.h
 $Header$
 */
#ifndef  MERIT_PRUNETREE_H
#define  MERIT_PRUNETREE_H

#include <string>

// forward class declarations
namespace classification { class Tree;}
class Tuple;


/** @class PruneTree
 *  @brief Reduce number of rows of merit NTuple 
 *              applying IM selections defined in xml file.  
 *
 *  @authors Traudl Hansl-Kozanecka <hansl@slac.stanford.edu>
 *  @authors Toby Burnett <tburnett@u.washington.edu>
 *
 *  Usage:
 *     @verbatim
 *       Tuple tuple; 
 *       PruneTree pt(tuple);
 *       [...]
 *       double prob = pt();
 *     @endverbatim  
 */ // ________________________________________________________


class PruneTree {

    public:

    /** Set up the selection
     *  @param t           The input ROOT tuple
     *  @param xml_file    IM file containing descriptions of the predict nodes 
     */
    PruneTree( Tuple& t,  std::string xml_file = "");

    ~PruneTree();

    /** Apply the selection to the current tuple instance 
     *  @return  Probability assigned to this event
     */
    double operator()(); 

private:
    class Lookup;                // nested class
    class PreClassify;           // nested class 
    PreClassify * m_preclassify; // 
    classification::Tree * m_classifier;
};
#endif   //  MERIT_PRUNETREE_H



