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
    enum{  CALHIGH, CALLOW, NOCAL,
       VTX_THIN, VTX_THICK,
       VTX_THIN_TAIL,     VTX_THIN_BEST,
       ONE_TRK_THIN_TAIL, ONE_TRK_THIN_BEST,
       VTX_THICK_TAIL,    VTX_THICK_BEST, 
       ONE_TRK_THICK_TAIL, ONE_TRK_THICK_BEST, 
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
    IMnodeInfo imNodeInfo[] = {
        { CALHIGH,           "CT Cal High",  0 },
        { CALLOW,            "CT Cal Low",  0 },
        { NOCAL,             "CT No Cal",  0 },
        { VTX_THIN,          "CT VTX Thin",     0},
        { VTX_THICK,         "CT Thick VTX",     0 },
        { VTX_THIN_TAIL,     "CT VTX Thin Tail" ,0}, 
        { VTX_THIN_BEST,     "RT VTX Thin Core", 0},
        { ONE_TRK_THIN_TAIL, "CT 1Tkr Thin Tail",0},
        { ONE_TRK_THIN_BEST, "RT 1Tkr Thin Core",0},
        { VTX_THICK_TAIL,    "CT VTX Thick Tail", 0},
        { VTX_THICK_BEST,    "RT VTX Thick Core", 0},
        { ONE_TRK_THICK_TAIL,"CT 1Tkr Thick Tail", 0},
        { ONE_TRK_THICK_BEST,"RT 1Tkr Thick Core", 0}
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
        t.tupleItem("VtxAngle"); 
        t.tupleItem("CalEnergySum");
        t.tupleItem("TkrRadLength");
        t.tupleItem("CalCsIRLn");
        t.tupleItem("CalBkHalfRatio");
        t.tupleItem("CalLyr7Ratio");
        t.tupleItem("CalLyr0Ratio");
        t.tupleItem("EvtTkrEComptonRatio");
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

        m_firstLayer = t.tupleItem("Tkr1FirstLayer");
        m_calEnergySum = t.tupleItem("CalEnergySum");
        m_calTotRLn =t.tupleItem("CalTotRLn");


        // New items to create

        new TupleItem("IMgoodCalProb",&m_goodCalProb);
        new TupleItem("IMvertexProb", &m_vtxProb);
        new TupleItem("IMcoreProb",   &m_coreProb);
        new TupleItem("IMpsfErrPred", &m_psfErrPred);
        m_classifier = new classification::Tree(looker, log, 0); // verbosity
        // translate the Tuple map
  
        m_classifier->load(xml_file);

        // get the list of root prediction tree nodes
        imnodes.reserve(20);
        for( unsigned int i=0; i<NODE_COUNT; ++i){
            IMpredictNode n(imNodeInfo[i],m_classifier);
           imnodes[imNodeInfo[i].id]=n;
        }

    }

    void ClassificationTree::execute()
    {

        // apply cuts on cal energy:
        //ifelse((CalEnergySum< 100. | CalTotRLn < 2), ifelse((CalTotRLn < 2 | CalEnergySum <  5), "NoCal", "LowCal"),"HighCal")

        if( *m_calEnergySum> 100. && *m_calTotRLn > 2. ) { 
            m_goodCalProb= imnodes[CALHIGH].evaluate();
        }else if ( *m_calEnergySum >5 && *m_calTotRLn >2.) {
            m_goodCalProb= imnodes[CALLOW].evaluate();
        }else {
            m_goodCalProb= imnodes[NOCAL].evaluate();
        } 
        if( m_goodCalProb<0.5) { 
            m_coreProb=m_psfErrPred=0;
            return;
        }

        if( *m_firstLayer < 12 ) {
            m_vtxProb = imnodes[VTX_THIN].evaluate(); 
            if( m_vtxProb >0.5) {
                m_coreProb =   imnodes[VTX_THIN_TAIL].evaluate();
                m_psfErrPred = imnodes[VTX_THIN_BEST].evaluate();
            }else{
                m_coreProb =   imnodes[ONE_TRK_THIN_TAIL].evaluate();
                m_psfErrPred = imnodes[ONE_TRK_THIN_BEST].evaluate();
            }
        }else {
            m_vtxProb = imnodes[VTX_THICK].evaluate();
            if( m_vtxProb >0.5) {
                m_coreProb =   imnodes[VTX_THICK_TAIL].evaluate();
                m_psfErrPred = imnodes[VTX_THICK_BEST].evaluate();
            }else{
                m_coreProb =   imnodes[ONE_TRK_THICK_TAIL].evaluate();
                m_psfErrPred = imnodes[ONE_TRK_THICK_BEST].evaluate();
            }
        }

    }
    ClassificationTree::~ClassificationTree()
    {
        delete m_classifier;
    }
