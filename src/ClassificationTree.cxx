/** @file ClassificationTree.cxx
@brief 

*/
#include "ClassificationTree.h"
#include "classification/Tree.h" 
#include "analysis/Tuple.h"
/*  Output from analysis of the PSF_Analysis IM file, extracted to a UserLibrary

Loading Trees from file "C:\glast\DeltaRelease\classification\v0r3/xml/PSF_Analysis.xml"
Prediction Nodes:
        id                         label     nodes max depth
         0             CT CAL Energy (0)'        133        17
         0               CT VTX Thin (0)'        267        27
         0              CT VTX Thick (0)'        207        25
         0          CT VTX Thin Tail (0)'        179        19
         0          RT VTX Thin Core (0)'         81        14
         0         RT 1Tkr Thin Core (0)'         95        17
         0         CT 1Tkr Thin Tail (0)'        279        19
         0         CT VTX Thick Tail (0)'        151        12
         0         RT VTX Thick Core (0)'        107        14
         0       RT 1`Tkr Thick Core (0)'         77        18
         0        CT 1Tkr Thick Tail (0)'        313        24
Found 11 prediction nodes

Combined used names:

"CalCntRLn", "CalDeadCntRat", "CalDeadTotRat", "CalEdgeAngle", "CalTotRLn", "Energy.Sum.Opt", "TKR.ComptonRatio", "Tkr1.
1stGaps", "Tkr1DieEdge", "Tkr1DifHits", "Tkr1E1stChisq", "Tkr1EChisq", "Tkr1EFrac", "Tkr1EQual", "Tkr1PrjTwrEdge", "Tkr2
E1stChisq", "Tkr2EChisq", "Tkr2EQual", "TkrHDCount", "TkrNumTracks", "TwrEdgeAngle", "VtxEAngle", "VtxHeadSep",

*/
namespace {
    const char *nodenames[]={
            "CT CAL Energy (0)",
            "CT VTX Thin (0)",
            "CT VTX Thick (0)",
            "CT VTX Thin Tail (0)",
            "RT VTX Thin Core (0)",
            "RT 1Tkr Thin Core (0)",
            "CT 1Tkr Thin Tail (0)",
            "CT VTX Thick Tail (0)",
            "RT VTX Thick Core (0)",
            "RT 1`Tkr Thick Core (0)",
            "CT 1Tkr Thick Tail (0)"};
            std::pair<std::string,std::string> alias_pairs[]={
                std::make_pair(    "CalEdgeAngle",    "EvtCalEdgeAngle"),
                std::make_pair(    "Energy.Sum.Opt",  "EvtEnergyOpt"),
                std::make_pair(    "TKR.ComptonRatio","EvtTkrComptonRatio"),
                std::make_pair(    "Tkr1.1stGaps",    "Tkr1FirstGaps"),
                std::make_pair(    "Tkr1E1stChisq",   "EvtTkr1EFirstChisq"),
                std::make_pair(    "Tkr1EChisq",      "EvtTkr1EChisq"),
                std::make_pair(    "Tkr1EFrac",       "EvtTkr1EFrac"),
                std::make_pair(    "Tkr1EQual",       "EvtTkr1EQual"),
                std::make_pair(    "Tkr2E1stChisq",   "EvtTkr2EFirstChisq"),
                std::make_pair(    "Tkr2EChisq",      "EvtTkr2EChisq"),
                std::make_pair(    "Tkr2EQual",       "EvtTkr2EQual"),
                std::make_pair(    "TwrEdgeAngle",    "EvtTkrEdgeAngle"),
                std::make_pair(    "VtxEAngle",       "EvtVtxEAngle")
            };
        
   enum{ CAL_ENERGY, 
       VTX_THICK, VTX_THICK_TAIL, VTX_THICK_BEST, 
       ONE_TRK_THICK_TAIL, ONE_TRACK_THICK_BEST, 
       VTX_THIN, VTX_THIN_TAIL, VTX_THIN_BEST,
       ONE_TRK_THIN_TAIL, ONE_TRK_THIN_BEST};

   std::vector<const classification::Tree::Node*> rootNode;

}

    ClassificationTree::ClassificationTree( Tuple& t, std::string xml_file)
    {

        if( xml_file.empty() ){
            const char *sPath = ::getenv("CLASSIFICATIONROOT");
            xml_file= (sPath==0)?  "../xml/PSF_Analysis.xml":  std::string(sPath) + "/xml/PSF_Analysis.xml";
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
        int npairs = sizeof(alias_pairs)/sizeof(std::pair<std::string, std::string>);
        for( int i=0; i< npairs; ++i) {
            // create a tuple item first
            std::string tname = alias_pairs[i].second;
            const TupleItem* ti = t.tupleItem(tname);

            t.add_alias(tname, alias_pairs[i].first);
        }

        m_classifier = new classification::Tree(looker, 2);
        // translate the Tuple map
  
        m_classifier->load(xml_file);

        // now connect to output
        //
        for( int i = 0; i< sizeof(nodenames)/sizeof(void*); ++i) {
            rootNode.push_back(m_classifier->getPredictTree(nodenames[i]) );
        }

    }

    void ClassificationTree::execute()
    {
        //call something that will pick up the values from the pointers,  save the result    m_classifier->navigateTree();

    }
    ClassificationTree::~ClassificationTree()
    {
        delete m_classifier;
    }
