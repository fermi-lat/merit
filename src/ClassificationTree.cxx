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
      
   enum{ CAL_ENERGY, 
       VTX_THICK, VTX_THICK_TAIL, VTX_THICK_BEST, 
       ONE_TRK_THICK_TAIL, ONE_TRACK_THICK_BEST, 
       VTX_THIN, VTX_THIN_TAIL, VTX_THIN_BEST,
       ONE_TRK_THIN_TAIL, ONE_TRK_THIN_BEST};

   std::vector<const classification::Tree::Node*> rootNode;

}

    ClassificationTree::ClassificationTree( Tuple& t, std::string xml_file)
        : m_classifier( new classification::Tree(2))
    {

        if( xml_file.empty() ){
            const char *sPath = ::getenv("CLASSIFICATIONROOT");
            xml_file= (sPath==0)?  "../xml/PSF_Analysis.xml":  std::string(sPath) + "/xml/PSF_Analysis.xml";
        }

        // translate the Tuple map
        classification::Tree::VariableMap vmap;
        for( Tuple::iterator tit = t.begin(); tit != t.end(); ++tit){
            TupleItem & var = **tit;
            vmap[var.name()] = &var.value();
        }

        m_classifier->load(xml_file, vmap);

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
