/** @file ClassificationTree.cxx
@brief 

*/
#include "ClassificationTree.h"
#include "classification/Tree.h" 
#include "analysis/Tuple.h"
/*  Output from analysis of the PSF_Analysis IM file

Loading Trees from file "/common/IM_files/PSF_Analysis_11.imw"
Prediction Nodes:
id                     label           nodes max depth
385                 CT CAL Energy'        133        17
410                  CT VTX Thick'        207        25
411             CT VTX Thick Tail'        117        13
413             CT VTX Thick BEST'        151        13
414            CT 1Tkr Thick Tail'        277        21
416            CT 1Tkr Thick BEST'        319        24
421                   CT VTX Thin'        267        27
422              CT VTX Thin Tail'        181        19
423              CT VTX Thin Best'        217        22
425             CT 1Tkr Thin Tail'        265        21
426             CT 1Tkr Thin BEST'        337        24
Found 11 prediction nodes

Combined used names:
"CalCntRLn", "CalDeadCntRat", "CalDeadTotRat", "CalEdgeAngle", "CalTotRLn", "Energy.Sum.Opt", 
"Tkr1.1stGaps", "Tkr1DieEdge", "Tkr1DifHits", "Tkr1E1stChisq", "Tkr1EChisq", "Tkr1EFrac", "Tkr1EQual", 
"Tkr1Gaps", "Tkr1PrjTwrEdge", "Tkr2E1stChisq", "Tkr2EChisq", "Tkr2EQual", "TwrEdgeAngle", "VtxEAngle", "VtxHeadSep",

*/
namespace {
   const char *nodenames[]={"CT CAL Energy", 
       " CT VTX Thick","CT VTX Thick Tail" ,"CT VTX Thick BEST", 
       "CT 1Tkr Thick Tail",  "CT 1Tkr Thick BEST", 
       "CT VTX Thin", "CT VTX Thin Tail", "CT VTX Thin Best", 
       "CT 1Tkr Thin Tail", "CT 1Tkr Thin BEST" };
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
            xml_file= (sPath==0)?  "..\\data\\UserLibrary.xml":  std::string(sPath) + "\\data\\UserLibrary.xml";
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
