/** @file ClassificationTree.cxx
@brief 

*/
#include "ClassificationTree.h"
#include "classification/Tree.h" 
#include "analysis/Tuple.h"
/*  Output from analysis of the PSF_Analysis IM file, extracted to a UserLibrary

Loading Trees from file "\glast\IM\PSF_Analysis_14.imw"
Prediction Nodes:
        id                         label     nodes max depth
       385                 CT CAL Energy'        133        17
       507                CT  GoodEnergy'        209        18
       509                   CT VTX Thin'        273        22
       511                  CT Thick VTX'        205        17
       512              CT VTX Thin Tail'        181        15
       513              RT VTX Thin Core'         89        14
       514            CT  1Tkr Thin Tail'        235        20
       515             RT 1Tkr Thin Core'        131        18
       517             CT VTX Thick Tail'        165        15
       518             RT VTX Thick Core'        113        15
       520            CT 1Tkr Thick Tail'        283        31
       521            RT 1Tkr Thick Core'         91        25
Found 12 prediction nodes

Combined used names:

"CalCntRLn", "CalDeadCntRat", "CalDeadTotRat", "CalEdgeAngle", "CalLongRms", "CalTotRLn", "CalTotSumCorr",
"CalTrackDoca", "CalTrackSep", "CalTransRms", "CalTwrEdge", "CalTwrGap", "CalXtalRatio", "CalXtalsTrunc",
"Energy.Sum.Opt", "EvtCalEdgeAngle", "EvtEnergySumOpt", "EvtTkr1EChisq", "EvtTkr1EFirstChisq", "EvtTkr1EFrac",
"EvtTkr1EQual", "EvtTkr2EChisq", "EvtTkr2EFirstChisq", "EvtTkr2EQual", "EvtTkrComptonRatio", "EvtTkrEdgeAngle",
"EvtVtxEAngle", "Tkr1DieEdge", "Tkr1DifHits", "Tkr1Gaps", "Tkr1PrjTwrEdge", "Tkr1TwrEdge", "TkrHDCount",
"TkrNumTracks", "TkrTwrEdge", "TwrEdgeAngle", "VtxHeadSep",

*/
namespace {
  using std::string;
    const char *nodenames[]={
    "CT CAL Energy",
    "CT  GoodEnergy",
    "CT VTX Thin",
    "CT Thick VTX",
    "CT VTX Thin Tail",
    "RT VTX Thin Core",
    "CT  1Tkr Thin Tail",
    "RT 1Tkr Thin Core",
    "CT VTX Thick Tail",
    "RT VTX Thick Core",
    "CT 1Tkr Thick Tail",
    "RT 1Tkr Thick Core"};
  std::pair< string,  string> alias_pairs[]={
                std::make_pair(   string( "CalEdgeAngle"),     string( "EvtCalEdgeAngle")),
                std::make_pair(   string( "Energy.Sum.Opt"),   string( "EvtEnergyOpt")),
                std::make_pair(   string( "TKR.ComptonRatio"), string( "EvtTkrComptonRatio")),
                std::make_pair(   string( "Tkr1.1stGaps"),     string( "Tkr1FirstGaps")),
                std::make_pair(   string( "Tkr1E1stChisq"),    string( "EvtTkr1EFirstChisq")),
                std::make_pair(   string( "Tkr1EChisq"),       string( "EvtTkr1EChisq")),
                std::make_pair(   string( "Tkr1EFrac"),        string( "EvtTkr1EFrac")),
                std::make_pair(   string( "Tkr1EQual"),        string( "EvtTkr1EQual")),
                std::make_pair(   string( "Tkr2E1stChisq"),    string( "EvtTkr2EFirstChisq")),
                std::make_pair(   string( "Tkr2EChisq"),       string( "EvtTkr2EChisq")),
                std::make_pair(   string( "Tkr2EQual"),        string( "EvtTkr2EQual")),
                std::make_pair(   string( "TwrEdgeAngle"),     string( "EvtTkrEdgeAngle")),
                std::make_pair(   string( "VtxEAngle"),        string( "EvtVtxEAngle"))
            };
        
   enum{ CAL_ENERGY, GOODENERGY,
       VTX_THICK, VTX_THICK_TAIL, VTX_THICK_BEST, 
       ONE_TRK_THICK_TAIL, ONE_TRACK_THICK_BEST, 
       VTX_THIN, VTX_THIN_TAIL, VTX_THIN_BEST,
       ONE_TRK_THIN_TAIL, ONE_TRK_THIN_BEST};

   std::vector<const classification::Tree::Node*> rootNode;

}

ClassificationTree::ClassificationTree( Tuple& t, std::ostream& log, std::string xml_file)
: m_log(log)
    {

        if( xml_file.empty() ){
            const char *sPath = ::getenv("CLASSIFICATIONROOT");
            xml_file= (sPath==0)?  "../xml/PSF_Analysis.xml":  std::string(sPath) + "/xml/PSF_Analysis_14.imw";
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
            Tuple& m_t;
        };
        Lookup looker(t);

        //add aliases to the tuple
        int npairs = sizeof(alias_pairs)/sizeof(std::pair< std::string, std::string>);
        for( int i=0; i< npairs; ++i) {
            // create a tuple item first
            std::string tname = std::string(alias_pairs[i].second);
	    // const TupleItem* ti = t.tupleItem(std::string(tname));

            t.add_alias(tname, alias_pairs[i].first);
        }

        m_classifier = new classification::Tree(looker, log, 2);
        // translate the Tuple map
  
        m_classifier->load(xml_file);

        // now connect to output
        //
        for(unsigned int i = 0; i< sizeof(nodenames)/sizeof(void*); ++i) {
            classification::Tree::Node* node= m_classifier->getPredictTree(nodenames[i]);
            rootNode.push_back(node);
        }

    }

    void ClassificationTree::execute()
    {
        //call something that will pick up the values from the pointers,  save the result    m_classifier->navigateTree();
        double u[12];
        for( int i = 0; i<12; ++i){
         u[i] = m_classifier->navigate(rootNode[i])[0];
        }

    }
    ClassificationTree::~ClassificationTree()
    {
        delete m_classifier;
    }
