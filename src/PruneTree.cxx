/** @file PruneTree.cxx
@brief apply the set of Bill's cuts for pruning background 
*/
#include "PruneTree.h"
#include "analysis/Tuple.h"
#include "classification/Tree.h"   // package classification

#include <cassert>
/*  This is the output of classification::Tree::load() with verbosity set to 1.
--------------------------------------------------------------
Loading Trees from file "D:\Users\burnett\dev_197\merit\v6r14p5/xml/CTPruner_DC1.imw"
176                    Tiles&Hi-E       177           NoTiles&LowE&GoodXR       179                NoTiles&1Xtals
180                NoTiles&0Xtals       181        Tiles&ACD< -170&GoodXR       182          Tiles&ACD<-170&1Xtal
183          Tiles&ACD<-170&0Xtal       184           Tiles&ACD<-20&Med-E       185           Tiles&ACD>-20&Low-E
Found 9 prediction nodes

Combined used names:

"AcdActiveDist", "AcdDoca", "AcdTotalEnergy", "AcdUpperTileCnt", "CalBkHalfRatio", "CalCsIRLn", "CalDeadTotRat", "CalDel
taT", "CalEnergySum", "CalLATEdge", "CalLRmsRatio", "CalLyr0Ratio", "CalLyr7Ratio", "CalMIPDiff", "CalTotRLn", "CalTotSu
mCorr", "CalTrackDoca", "CalTrackSep", "CalTwrEdge", "CalTwrGap", "CalXtalRatio", "CalXtalsTrunc", "EvtCalETLRatio", "Ev
tCalETrackDoca", "EvtCalETrackSep", "EvtCalEXtalRatio", "EvtCalEXtalTrunc", "EvtCalEdgeAngle", "EvtEnergySumOpt", "EvtLo
gESum", "EvtTkr1EChisq", "EvtTkr1EFirstChisq", "EvtTkr1EFrac", "EvtTkr1EQual", "EvtTkr1PSFMdRat", "EvtTkr2EChisq", "EvtT
kr2EFirstChisq", "EvtTkrComptonRatio", "EvtTkrEComptonRatio", "EvtTkrEdgeAngle", "EvtVtxEAngle", "EvtVtxEDoca", "EvtVtxE
EAngle", "EvtVtxEHeadSep", "Pr(CORE)", "Tkr1Chisq", "Tkr1DieEdge", "Tkr1FirstChisq", "Tkr1FirstLayer", "Tkr1Hits", "Tkr1
KalEne", "Tkr1PrjTwrEdge", "Tkr1Qual", "Tkr1TwrEdge", "Tkr1ZDir", "Tkr2Chisq", "Tkr2KalEne", "TkrBlankHits", "TkrEnergy"
, "TkrHDCount", "TkrNumTracks", "TkrRadLength", "TkrSumKalEne", "TkrThickHits", "TkrThinHits", "TkrTotalHits", "TkrTrack
Length", "TkrTwrEdge", "VtxAngle", "VtxDOCA", "VtxHeadSep", "VtxS1", "VtxTotalWgt", "VtxZDir",

Following names were needed by nodes, but not supplied in the map:

"AcdUpperTileCnt",

*/
// need to add: AcdUpperTileCnt=AcdTileCount-AcdNoSideRow2 -AcdNoSideRow1

namespace {

    // Convenient identifiers used for the nodes
    typedef enum{ ONE, TWO, THREE,  FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, 
       NODE_COUNT, NONE
    } Category;


    /** table of information about nodes to expect in the IM file.
    */
    class IMnodeInfo { 
    public:
        int id;           // unique ID for local identification
        const char* name; // the name of the Prediction node in the IM XML file
        int index;        // index of the classification type within the list of probabilites
    };

    // these have to correspond to the IM file.
    IMnodeInfo imNodeInfo[] = {
        { ONE,   "Tiles&Hi-E",             1 }, 
        { TWO,   "NoTiles&LowE&GoodXR",    1 }, 
        { THREE, "NoTiles&1Xtals",         1 },
        { FOUR,  "NoTiles&0Xtals",         1 },
        { FIVE,  "Tiles&ACD< -170&GoodXR", 1 },
        { SIX,   "Tiles&ACD<-170&1Xtal",   0 },
        { SEVEN, "Tiles&ACD<-170&0Xtal",   1 }, 
        { EIGHT, "Tiles&ACD<-20&Med-E",    0 },
        { NINE,  "Tiles&ACD>-20&Low-E",    1 },
      
    };


    /** Manage interface to one of the prediction nodes
    */
    //___________________________________________________________________________

    class IMpredictNode { 
    public:
        IMpredictNode(const IMnodeInfo& info, const classification::Tree* tree )
            :  m_offset(info.index), m_tree(tree)
        {
            m_node = m_tree->getPredictTree(info.name);
            if( m_node==0) std::cerr << "IMpredictNode: Tree " << info.name << " not found in tree" << std::endl;
            assert(m_node);
        }

        double evaluate() const{
            return m_tree->navigate(m_node)[m_offset];
        }

        int m_offset;
        const classification::Tree* m_tree;
        const classification::Tree::Node* m_node;
    };

  std::vector<IMpredictNode> imnodes;

} //anonymous namespace



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
    PreClassify( PruneTree::Lookup& lookup ) 
        : CalEnergySum  (*lookup( "CalEnergySum" )) 
        , CalXtalRatio  (*lookup( "CalXtalRatio" ))
        , CalXtalsTrunc (*lookup( "CalXtalsTrunc")) 
        , AcdActiveDist (*lookup( "AcdActiveDist"))
        , AcdTileCount  (*lookup( "AcdTileCount" ))
        , AcdNoSideRow1 (*lookup( "AcdNoSideRow1"))
        , AcdNoSideRow2 (*lookup( "AcdNoSideRow2"))
	, AcdRibbonActDist (*lookup( "AcdRibbonActDist"))
	, Tkr1SSDVeto   (*lookup( "Tkr1SSDVeto"))
    { 
        // need a new tuple item
        new TupleItem("AcdUpperTileCnt", AcdUpperTileCnt);
    }

    //! return truth value, for current TTree position
    operator Category()
    {
        AcdUpperTileCnt = AcdTileCount - AcdNoSideRow2 - AcdNoSideRow1;

        bool NoTiles = AcdTileCount == 0.0 ;
	bool Tiles   = !NoTiles;
        
        bool lowE    = CalEnergySum < 350.;
        bool medE    = CalEnergySum >= 350. && CalEnergySum < 3500.;
        bool hiE     = !(lowE || medE);

	// Tkr1SSDVeto  number of hit layers before start of track
        bool Acdlt20  = (AcdActiveDist < -20.0 && AcdRibbonActDist < -20.0) || Tkr1SSDVeto > 1 ;
        bool Acdgt20  = !Acdlt20;      //?
        bool Acdlt150 =  AcdActiveDist < -150. ;

        bool Xtals    = CalXtalsTrunc > 0.0 ;  // ?
        bool noXtal   = !Xtals ;
        bool GoodXR   = CalXtalRatio > 0.02 && CalXtalRatio < 0.98 ;  // ?


       

        // todo: apply pre-categorization logic
        /*
        CalEnergySum > 5 & CalCsIRLn > 2
(AcdActiveDist < -20 & AcdRibbonActDist < -20) | Tkr1SSDVeto > 1

MeasEnergyType	categorical	ifelse((CalEnergySum < 350),"Low-E",ifelse((CalEnergySum < 3500),"Med-E","Hi-E"))
MeasEnergyType == "Hi-E"
AcdTileCount == 0
CalXtalRatio > .02 & CalXtalRatio < .98
CalXtalsTrunc > 0
AcdActiveDist <-150
CalXtalRatio > .02 & CalXtalRatio < .98
AcdActiveDist < -20 &MeasEnergyType == "Med-E"
*/

        return NONE;
    }

private:
    // references to the values read in from ROOT, or set directly, in the tuple
    const double &   CalEnergySum; 
    const double &   CalXtalRatio;
    const double &   CalXtalsTrunc;
    const double &   AcdActiveDist; 
    const double &   AcdTileCount;
    const double &   AcdNoSideRow1 ;
    const double &   AcdNoSideRow2 ;
    const double &   AcdRibbonActDist;
    const double &   Tkr1SSDVeto;   
    // output
    double  AcdUpperTileCnt;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PruneTree::PruneTree( Tuple& t,  std::string xml_file)
{
    // create a lookup object, pass it to the preclassifier and the classification tree code
    PruneTree::Lookup looker(t) ;
    m_preclassify = new PruneTree::PreClassify(looker);
    m_classifier = new classification::Tree(looker, std::cout, 0); // verbosity

   // accept, or generate filename 
    std::string default_file("/xml/CTPruner_DC1.imw");
    if( xml_file.empty() ){
        const char *sPath = ::getenv("MERITROOT");
        xml_file= std::string(sPath==0?  "../": sPath) + default_file;
    }
    // analyze the file
    m_classifier->load(xml_file);

    // get the list of root prediction tree nodes
    imnodes.reserve(NODE_COUNT);
    for( unsigned int i=0; i<NODE_COUNT; ++i){
        IMpredictNode n(imNodeInfo[i],m_classifier);
        imnodes[imNodeInfo[i].id]=n;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PruneTree::~PruneTree(){
    delete m_preclassify;
    delete m_classifier;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
double PruneTree::operator()()
{
   Category  cat = *m_preclassify;
   return  (cat==NONE) ?  0 :  imnodes[cat].evaluate();
}
