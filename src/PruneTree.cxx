/** @file PruneTree.cxx
@brief apply the set of Bill's cuts for pruning background 
*/
#include "PruneTree.h"

#include "analysis/Tuple.h"

#include <sstream>
#include <cassert>

// create lookup class to make translations
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class PruneTree::Lookup : public classification::Tree::ILookUpData {
public:
    Lookup(Tuple& t):m_t(t){}
    const double * operator()(const std::string& name){
        TupleItem* ti = const_cast<TupleItem*>(m_t.tupleItem(name));
        if( ti==0) return 0;
        const double * f = & (ti->value());
        return f;
    }
    // note that float flag depends on how the tuple does it.
    bool isFloat()const{return m_t.isFloat();}
    Tuple& m_t;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class PruneTree::PreClassify {
public:
    //! ctor: just associate with tree and find the branches we need
    PreClassify(PruneTree::Lookup& lookup) 
        : VtxAngle (*lookup(                 "VtxAngle")) 
        , EvtEnergySumOpt (*lookup(      "EvtEnergySumOpt"))
        , EvtTkrComptonRatio (*lookup("EvtTkrComptonRatio")) 
        , Tkr1ToTFirst (*lookup(             "Tkr1ToTFirst"))
        , Tkr1ToTAve (*lookup(             "Tkr1ToTAve"))
        , AcdTotalEnergy (*lookup(        "AcdTotalEnergy"))  
        , AcdRibbonActDist (*lookup(    "AcdRibbonActDist")) 
        , AcdTileCount (*lookup(          "AcdTileCount"))
        , FilterStatus_HI (*lookup(         "FilterStatus_HI"))
    {  }

    typedef  enum {NONE, ONE, TWO}  Category;
    //! return truth value, for current TTree position
    operator Category()
    {
        return NONE;
    }
private:
    // references to the values read in from ROOT, or set directly, in the tuple
    const double &   VtxAngle; 
    const double &   EvtEnergySumOpt;
    const double &   EvtTkrComptonRatio;
    const double &   Tkr1ToTFirst;
    const double &   Tkr1ToTAve;
    const double &   AcdTotalEnergy;  
    const double &   AcdRibbonActDist; 
    const double &   AcdTileCount;
    const double &   FilterStatus_HI;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PruneTree::PruneTree( Tuple& t, std::ostream& log, std::string xml_file)
: m_log(log)
{
    PruneTree::Lookup looker(t) ;
    m_preclassify = new PruneTree::PreClassify(looker);
    m_classifier = new classification::Tree(looker, log, 0); // verbosity
    // translate the Tuple map

    m_classifier->load(xml_file);

    // TODO: get the list of root prediction tree nodes
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PruneTree::~PruneTree(){
    delete m_preclassify;
    delete m_classifier;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PruneTree::execute()
{
    PreClassify::Category  test = *m_preclassify;
}
