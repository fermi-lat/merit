/** @file PruneTree.h
 $Header: /nfs/slac/g/glast/ground/cvs/merit/src/PruneTree.h,v 1.6 2004/01/07 17:47:34 hansl Exp $
 */
#ifndef  MERIT_PRUNETREE_H
#define  MERIT_PRUNETREE_H

#include <string>

// forward class declarations
namespace classification { class Tree;}
class Tuple;


/** @class PruneTree
 *  @brief Reduce number of rows of merit NTuple 
 *         applying Insightful Miner (IM) selections defined in xml file.  
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
     *  @param t           The Tuple ( = vector of TupleItems)
     *  @param xml_file    IM file containing descriptions of the predict nodes 
     */
    PruneTree( Tuple& t,  std::string xml_file = "");

    ~PruneTree();

    /** Apply the selection to the current TupleItem ( = event)
     *  @return   Probability assigned to this event
     */
    double operator()(); 

private:
    class Lookup;                // nested class
    class PreClassify;           // nested class

    PreClassify *          m_preclassify; //! association of ROOT branches to TupleItems
    classification::Tree * m_classifier;  //! interface to IM classification tree 
};
#endif   //  MERIT_PRUNETREE_H



