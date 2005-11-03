/** @file ClassificationTree.h
@brief  Declare class ClassificationTree

$Header: /nfs/slac/g/glast/ground/cvs/merit/src/ClassificationTree.h,v 1.21 2005/10/31 17:29:23 burnett Exp $
*/
#ifndef CLASSIFICATIONTREE_H
#define  CLASSIFICATIONTREE_H

#include <string>
#include <vector>
#include <iostream>

class TupleItem;
class Tuple;
namespace GlastClassify { class TreeFactory; }

/** @class ClassificationTree
    @brief Manage Atwood-inspired classification trees, creating new tuple variables
    based on values found in the tuple

*/
class ClassificationTree 
{
public:
    /** set up the trees:
    * @param t The input tuple -- and will create new columns with the output
    * @param log -- optional stream for output [std::cout]
    * @param treepath -- file path to the root of the tree definitions
    */
    ClassificationTree( Tuple&t, std::ostream& log=std::cout, std::string treepath="");

    /** run the prediction nodes on the current tuple instance
    */
    void execute();  

    ~ClassificationTree();

    //! true if the vertex measurment of the gamma direction is better than one-track
    //! must be called after the execute method.
    bool useVertex()const;

private:
 
    GlastClassify::TreeFactory* m_factory;
    const TupleItem*  m_firstLayer; /// access to the first layer in the tuple
    const TupleItem*  m_calTotRLn; 
    const TupleItem*  m_calEnergyRaw;
    const TupleItem*  m_acdTileCount;
    const TupleItem*  m_evtTkrEComptonRatio;
    const TupleItem*  m_evtTkrComptonRatio;
    const TupleItem*  m_calMIPDiff;
    const TupleItem*  m_evtEnergySumOpt;
    const TupleItem*  m_calLRmsRatio;
    const TupleItem*  m_vtxAngle;

    const TupleItem * m_EvtEnergyCorr;
    const TupleItem * m_CalCfpEnergy;
    const TupleItem  *m_CalLllEnergy;
    const TupleItem * m_CalTklEnergy;

    // output quantities: corresponding tuple items
    float m_goodCalProb; 
    float m_goodPsfProb; 
    float m_vtxProb ; // vertex or track choice
    float m_gammaProb ;
    float m_gammaType;
    float m_BestEnergy;
    

    std::ostream& m_log;
};

#endif
