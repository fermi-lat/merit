/** @file ClassificationTree.cxx
@brief 

*/
#include "ClassificationTree.h"
#include "classification/Tree.h" 
#include "analysis/Tuple.h"

#include <sstream>
#include <cassert>

/* 
*/
namespace {

    // Convenient identifiers used for the nodes
    enum{  CALHIGH, CALMED, CALLOW, NOCAL,
       VTX_THIN, VTX_THICK,
       VTX_THIN_TAIL,     VTX_THIN_BEST,
       ONE_TRK_THIN_TAIL, ONE_TRK_THIN_BEST,
       VTX_THICK_TAIL,    VTX_THICK_BEST, 
       ONE_TRK_THICK_TAIL, ONE_TRK_THICK_BEST,
       BKG_VTX_HI, BKG_VTX_LO, BKG_1TRK_HI, BKG_1TRK_LO,
       NODE_COUNT
    };

    /** table of information about nodes to expect in the IM file.

    */
    class IMnodeInfo { 
    public:
        int id;           // unique ID for local identification
        const char* name; // the name of the Preciction node in the IM XML file
        int index;        // index of the classification type within the list of probabilites
    };
    // these have to correspond to the IM file, derived from v3r3p7 in this case
    IMnodeInfo imNodeInfo[] = {
        { CALHIGH,           "CT Cal High",  1 },
        { CALMED,            "CT Cal Med",   1 },
        { CALLOW,            "CT Cal Low",   1 },
        { NOCAL,             "CT No Cal",    1 },
        { VTX_THIN,          "CT VTX Thin",     1},
        { VTX_THICK,         "CT Thick VTX",     0 },
        { VTX_THIN_TAIL,     "CT VTX Thin Tail" ,1}, 
        { VTX_THIN_BEST,     "RT VTX Thin Core", 0},
        { ONE_TRK_THIN_TAIL, "CT 1Tkr Thin Tail",0},
        { ONE_TRK_THIN_BEST, "RT 1Tkr Thin Core",0},
        { VTX_THICK_TAIL,    "CT VTX Thick Tail", 1},
        { VTX_THICK_BEST,    "RT VTX Thick Core", 0},
        { ONE_TRK_THICK_TAIL,"CT 1Tkr Thick Tail", 1},
        { ONE_TRK_THICK_BEST,"RT 1Tkr Thick Core", 0},
        { BKG_VTX_HI,        "CT VTX-HI", 1},
        { BKG_VTX_LO,        "CT VTX-LO", 1},
        { BKG_1TRK_HI,       "CT 1TRK-HI", 1},
        { BKG_1TRK_LO,       "CT 1TRK-LO", 0}
    };

    /** Manage interface to one of the prediction nodes

    */
    class IMpredictNode { 
    public:
        IMpredictNode(const IMnodeInfo& info, const classification::Tree* tree)
            :  m_offset(info.index), m_tree(tree)
        {
            m_node = m_tree->getPredictTree(info.name);
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
#if 0
  std::pair< string,  string> alias_pairs[]={
//example if have to add
                std::make_pair(   string( "VtxEAngle"),        string( "EvtVtxEAngle"))
            };
#endif

// create lookup class to make translations
class Lookup : public classification::Tree::ILookUpData {
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

double * getTupleItemPointer(Tuple& t, std::string name)
{
    // simple little function that either finds an existing, or creates a new TupleItem
    TupleItem* ti = const_cast<TupleItem*>(t.tupleItem(name));
    if( ti==0) ti = new TupleItem(name);
    return &(ti->value());
}

}// anonymous namespace

ClassificationTree::ClassificationTree( Tuple& t, std::ostream& log, std::string xml_file)
: m_log(log)
    {
        std::string default_file("/xml/PSF_Analysis.imw");

        if( xml_file.empty() ){
            const char *sPath = ::getenv("CLASSIFICATIONROOT");
            xml_file= std::string(sPath==0?  "../": sPath) + default_file;
        }

        // create lookup class to make translations
        class Lookup : public classification::Tree::ILookUpData {
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
        Lookup looker(t);
#if 0  // uncomment this if needed
        //add aliases to the tuple
        int npairs = sizeof(alias_pairs)/sizeof(std::pair< std::string, std::string>);
        for( int i=0; i< npairs; ++i) {
            // create a tuple item first
            std::string tname = std::string(alias_pairs[i].second);
            t.add_alias(tname, alias_pairs[i].first);
        }
#endif

        // special tuple items we want on the output: make sure they are included in the list
        t.tupleItem("AcdActiveDist");
        t.tupleItem("CalEnergySum");
        t.tupleItem("TkrRadLength");
        t.tupleItem("CalCsIRLn");
        t.tupleItem("CalBkHalfRatio");
        t.tupleItem("CalLyr7Ratio");
        t.tupleItem("CalLyr0Ratio");
        t.tupleItem("CalXtalRatio");
        t.tupleItem("EvtLogESum");
        t.tupleItem("EvtCalETrackDoca");
        t.tupleItem("EvtCalETrackSep");
        t.tupleItem("EvtCalEXtalTrunc");
        t.tupleItem("EvtCalEXtalRatio");
        t.tupleItem("EvtCalETLRatio");
        t.tupleItem("CalDeltaT");
        t.tupleItem("CalLATEdge");
        t.tupleItem("TkrBlankHits");
        t.tupleItem("TkrThickHits");
        t.tupleItem("EvtCalETrackDoca");
        t.tupleItem("TkrThinHits");
        t.tupleItem("Tkr1ZDir");
        t.tupleItem("TkrSumKalEne");
        t.tupleItem("TkrTotalHits");
        t.tupleItem("Tkr1FirstGaps");
        t.tupleItem("Tkr2KalEne"); // no cal only?
        t.tupleItem("Tkr1Qual");  
        t.tupleItem("Tkr1FirstChisq"); 
        t.tupleItem("Tkr2Qual");
        t.tupleItem("Tkr2Chisq");
        t.tupleItem("Tkr1Hits");
        t.tupleItem("Tkr1KalEne");

        // these are used for preliminary cuts to select the tree to use
        m_firstLayer = t.tupleItem("Tkr1FirstLayer");
        m_calEnergySum = t.tupleItem("CalEnergySum");
        m_calTotRLn =t.tupleItem("CalTotRLn");
        m_evtEnergySumOpt = t.tupleItem("EvtEnergySumOpt");
        m_evtTkrEComptonRatio = t.tupleItem("EvtTkrEComptonRatio");
        m_evtTkrComptonRatio = t.tupleItem("EvtTkrComptonRatio");
        m_calMIPDiff = t.tupleItem("CalMIPDiff");
        m_calLRmsRatio = t.tupleItem("CalLRmsRatio");
        m_acdTileCount = t.tupleItem("AcdTileCount");
        m_vtxAngle = t.tupleItem("VtxAngle");

        // New items to create or override
        m_goodCalProb = getTupleItemPointer(t,"IMgoodCalProb");
        m_vtxProb=      getTupleItemPointer(t,"IMvertexProb");
        m_coreProb =    getTupleItemPointer(t,"IMcoreProb");
        m_psfErrPred =  getTupleItemPointer(t,"IMpsfErrPred");
        m_gammaProb=    getTupleItemPointer(t,"IMgammaProb");

        // since at least one of the background rejection trees needs to know about the core prob
        t.add_alias( "IMcoreProb", "Pr(CORE)");

        m_classifier = new classification::Tree(looker, log, 0); // verbosity
        // translate the Tuple map
  
        m_classifier->load(xml_file);

        // get the list of root prediction tree nodes
        imnodes.reserve(NODE_COUNT);
        for( unsigned int i=0; i<NODE_COUNT; ++i){
            IMpredictNode n(imNodeInfo[i],m_classifier);
           imnodes[imNodeInfo[i].id]=n;
        }

    }
 
    int ClassificationTree::backgroundRejection(){
    
        // Sets m_gammaProb from one of 4 different trees, corresponding to an initial split on vertex,
        // then energy
        // return the cut on this quantity that is used by the analysis
        int path=0; // path through the maze
        double  cut=0.5;


        // First apply the background check
        if( *m_vtxAngle>0 ) {
            if( *m_evtEnergySumOpt>350){
                //vTX-HI
                if( *m_evtTkrEComptonRatio > 0.60 &&
                    *m_calMIPDiff > 60. ){
                        path = BKG_VTX_HI;
                    }
                
            }else{
                // VTX-LO
                if( *m_acdTileCount==0 &&
                    *m_calMIPDiff> -125 &&
                    *m_evtTkrEComptonRatio > 0.8){
                        path = BKG_VTX_LO;
                        cut = 0.9;
                    }
            }
        }else{
            if( *m_evtEnergySumOpt>450.){ 
                //1TRK-HI
                if( *m_evtTkrComptonRatio > 0.70 &&
                    *m_calMIPDiff> 80. &&
                    *m_calLRmsRatio < 20.){
                        path = BKG_1TRK_HI;
                    }
            }else{
                //1TRK-LO
                if( *m_acdTileCount==0 &&
                    *m_evtTkrComptonRatio > 1. &&
                    *m_calLRmsRatio >5.0 &&
                    *m_firstLayer !=0 &&
                    *m_firstLayer <15 ){
                        path = BKG_1TRK_LO;
                        cut = 0.9;
                    }
            }
        }
        return path;

    }
    void ClassificationTree::execute()
    {

        // initialize IM output variables
        *m_coreProb=*m_psfErrPred=0;
        *m_vtxProb=*m_gammaProb=0;

        // select cal energy type:
        //ifelse((CalEnergySum< 100. | CalTotRLn < 2), ifelse((CalTotRLn < 2 | CalEnergySum <  5), "NoCal", "LowCal"),"HighCal")
/*      ifelse((CalEnergySum< 3500. | CalTotRLn < 2), 
            ifelse((CalTotRLn < 2 | CalEnergySum <  350),
		ifelse((CalTotRLn < 2 | CalEnergySum <  5),
		"NoCal", "LowCal"),
              "MedCal")
           ,"HighCal")
 */
        int cal_type=NOCAL;

        if(        *m_calEnergySum >3500. && *m_calTotRLn > 2.){ cal_type=CALHIGH;
        }else if ( *m_calEnergySum > 350. && *m_calTotRLn > 2.){ cal_type=CALMED;
        }else if ( *m_calEnergySum >   5. && *m_calTotRLn > 2.){ cal_type=CALLOW;
        } 
        // evalue appropriate tree for good cal prob
        *m_goodCalProb= imnodes[cal_type].evaluate();

        // rest only if cal prob ok
        if( *m_goodCalProb<0.5 )   return;
 
        // select vertex-vs-1 track, corresponding tree for core, psf
        int core_type=0, psf_type=0;
        if( *m_firstLayer < 12 ) {
            *m_vtxProb = imnodes[VTX_THIN].evaluate(); 
            if( *m_vtxAngle>0 && *m_vtxProb >0.5) { 
                    core_type=VTX_THIN_TAIL;     psf_type= VTX_THIN_BEST;
            }else{
                    core_type=ONE_TRK_THIN_TAIL; psf_type= ONE_TRK_THIN_BEST;
            }
        }else {
            *m_vtxProb = imnodes[VTX_THICK].evaluate();
            if(  *m_vtxAngle>0 &&  *m_vtxProb >0.5) {
                core_type=VTX_THICK_TAIL;         psf_type= VTX_THICK_BEST;
            }else{
                core_type=ONE_TRK_THICK_TAIL;     psf_type= ONE_TRK_THICK_BEST;
            }
        }

        // now evalute the appropriate trees
        *m_coreProb =   imnodes[core_type].evaluate();
        *m_psfErrPred = imnodes[psf_type].evaluate();

        // background type to select appropriate tree for gamma prob.
        int bkg_type = backgroundRejection();
        *m_gammaProb= bkg_type==0? 0 : imnodes[bkg_type].evaluate();

    }
    ClassificationTree::~ClassificationTree()
    {
        delete m_classifier;
    }
