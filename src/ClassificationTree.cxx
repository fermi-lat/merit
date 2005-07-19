/** @file ClassificationTree.cxx
@brief 
$Header: /nfs/slac/g/glast/ground/cvs/merit/src/ClassificationTree.cxx,v 1.29 2005/07/04 11:25:10 burnett Exp $

*/
#include "facilities/Util.h"
#include "ClassificationTree.h"
#include "classifier/DecisionTree.h"
#include "GlastClassify/TreeFactory.h"
#include "analysis/Tuple.h"

#include <sstream>
#include <cassert>

/* 
*/
namespace {

    // Convenient identifiers used for the nodes
    enum{
       GOODCAL,
       VTX_THIN, VTX_THICK,
       VTX_THIN_TAIL,    
       ONE_TRK_THIN_TAIL,
       VTX_THICK_TAIL,   
       ONE_TRK_THICK_TAIL,
       NODE_COUNT, // stop here
       NOCAL,
       BKG_VTX_HI, BKG_VTX_LO, BKG_1TRK_HI, BKG_1TRK_LO,
    };

    /** table to correlate indeces with 
      */
    //___________________________________________________________________________

    class CTinfo { 
    public:
        int id;           // unique ID for local identification
        std::string name; // the name of the DecisionTree
    };
    // these have to correspond to the folder names
    CTinfo imNodeInfo[] = {
        { GOODCAL,           "goodcal" },
        { VTX_THIN,          "vertex_thin"},
        { VTX_THICK,         "vertex_thick" },
        { VTX_THIN_TAIL,     "psf_thin_vertex"}, 
        { ONE_TRK_THIN_TAIL, "psf_thin_track"},
        { VTX_THICK_TAIL,    "psf_thick_vertex"},
        { ONE_TRK_THICK_TAIL,"psf_thick_track"}
#if 0 // wait for background
        { NOCAL,             "nocal"  },
        { BKG_VTX_HI,        "CT VTX-HI"},
        { BKG_VTX_LO,        "CT VTX-LO"},
        { BKG_1TRK_HI,       "CT 1TRK-HI"},
        { BKG_1TRK_LO,       "CT 1TRK-LO"}
#endif
    };

#if 0  // define aliases to deal with new values for now
  using std::string;
  std::pair< string,  string> alias_pairs[]=
      //
      {
  //     std::make_pair( string( "CalEnergySum"       ),string( "CalEnergyRaw"       ))
  //    ,std::make_pair( string( "EvtLogESum"         ),string( "EvtLogEnergy"       ))
  //    ,std::make_pair( string( "EvtCalETrackDoca"   ),string( "EvtECalTrackDoca"   ))
  //    ,std::make_pair( string( "EvtCalETrackSep"    ),string( "EvtECalTrackSep"    ))
  //    ,std::make_pair( string( "EvtCalEXtalTrunc"   ),string( "EvtECalXtalTrunc"   ))
   //   ,std::make_pair( string( "EvtCalEXtalRatio"   ),string( "EvtECalXtalRatio"   ))
  //    ,std::make_pair( string( "EvtTkrEComptonRatio"),string( "EvtETkrComptonRatio"))
      ,std::make_pair( string( "CalTotSumCorr"      ),string( "CalTotalCorr"       ))
      ,std::make_pair( string( "EvtVtxEEAngle"      ),string( "EvtEVtxAngle"       ))
      ,std::make_pair( string( "EvtTkr1EChisq"      ),string( "EvtETkr1Chisq"      ))
      ,std::make_pair( string( "EvtTkr1EFirstChisq" ),string( "EvtETkr1FirstChisq" ))

      // ================junk below here!===================================
      ,std::make_pair( string( "EvtTkr2EChisq"      ),string( "EvtETkr1Chisq"     ))
      ,std::make_pair( string( "EvtTkr2EFirstChisq" ),string( "EvtETkr1FirstChisq"))
      ,std::make_pair( string( "VtxTotalWgt"        ),string( "VtxChisq"          ))
      ,std::make_pair( string( "EvtVtxEHeadSep"     ),string( "VtxHeadSep"        ))
  };
#endif


// create lookup class to make translations
//___________________________________________________________________________

  class Lookup 
      : public GlastClassify::TreeFactory::ILookupData
{
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

//   simple function that either finds an existing, or creates a new TupleItem
//___________________________________________________________________________

double * getTupleItemPointer(Tuple& t, std::string name)
{
    // 
      TupleItem* ti;
      try
	{
	  ti = const_cast<TupleItem*>(t.tupleItem(name));
	} catch(...)
	  {
	    ti = new TupleItem(name);
	  }
      if( ti==0) ti = new TupleItem(name);
    return &(ti->value());
}
}  // anonymous namespace



/** @class BackgroundCut
    @brief set up and apply post-Rome cuts to the data in merit Tuple
    @author Toby Burnett
@verbatim
    Usage: 
        TTree& t;
        BackgroundCut bkg(t);

        [...]
        if( bkg ) { // this is background...
        }
@endverbatim
*/
//___________________________________________________________________________

class ClassificationTree::BackgroundCut {
public:
    //! ctor: just associate with tree and find the branches we need
    BackgroundCut(Tuple& t) 
        : VtxAngle          (*getTupleItemPointer(t,"VtxAngle")) 
        , EvtEnergySumOpt   (*getTupleItemPointer(t,"EvtEnergySumOpt"))
        , EvtTkrComptonRatio(*getTupleItemPointer(t,"EvtTkrComptonRatio")) 
        , Tkr1ToTFirst      (*getTupleItemPointer(t,"Tkr1ToTFirst"))
        , Tkr1ToTAve        (*getTupleItemPointer(t,"Tkr1ToTAve"))
        , AcdTotalEnergy    (*getTupleItemPointer(t,"AcdTotalEnergy"))  
        , AcdRibbonActDist  (*getTupleItemPointer(t,"AcdRibbonActDist")) 
        , AcdTileCount      (*getTupleItemPointer(t,"AcdTileCount"))
        , FilterStatus_HI   (*getTupleItemPointer(t,"FilterStatus_HI"))
        {  }


    //! return truth value, for current TTree position
    operator bool()
    {
        bool veto=false; // default no veto

        if(VtxAngle>0.0){
            // VERTEX 
            if(EvtEnergySumOpt<=350.0) {
                // LOCAL
                veto= Tkr1ToTFirst > 4.5 
                    || Tkr1ToTAve > 3.5
                    || AcdTotalEnergy > 0.25
                    || VtxAngle>0.4 ;
            }
            // MEDCAL, HICAL: pass
        }
        else{
            // 1 TRACK
            if(EvtEnergySumOpt <= 350.0) {
                // LOCAL
                veto = Tkr1ToTAve > 3.0
                    || AcdTileCount > 0.0
                    || AcdRibbonActDist >-300.0
                    || EvtTkrComptonRatio <1.05
                    || FilterStatus_HI >3.0 ;
            }else if( EvtEnergySumOpt <= 3500.0){
                // MEDCAL
                veto = Tkr1ToTAve >3.0
                    || AcdTotalEnergy >5.0
                    || EvtTkrComptonRatio <1.0 ;
            }
            // HICAL: pass
        }
        return veto;
    }
private:
    // references to the values read in from ROOT, or set directly, in the tuple
    double &   VtxAngle; 
    double &   EvtEnergySumOpt;
    double &   EvtTkrComptonRatio;
    double &   Tkr1ToTFirst;
    double &   Tkr1ToTAve;
    double &   AcdTotalEnergy;  
    double &   AcdRibbonActDist; 
    double &   AcdTileCount;
    double &   FilterStatus_HI;
};



//_________________________________________________________________________

ClassificationTree::ClassificationTree( Tuple& t, std::ostream& log, std::string treepath)
: m_background(*new BackgroundCut(t))
, m_log(log)
    {
        Lookup looker(t);

#if 0 // uncomment this if needed
        //add aliases to the tuple
        int npairs = sizeof(alias_pairs)/sizeof(std::pair< std::string, std::string>);
        for( int i=0; i< npairs; ++i) {
            // create a tuple item first
            std::string tname = std::string(alias_pairs[i].second);
            t.add_alias(tname, alias_pairs[i].first);
        }
#endif

        // special tuple items we want on the output: 
        // make sure they are included in the list
        t.tupleItem("AcdActiveDist");
        t.tupleItem("CalEnergyRaw");
        t.tupleItem("TkrRadLength");
        t.tupleItem("CalCsIRLn");
        t.tupleItem("CalBkHalfRatio");
        t.tupleItem("CalLyr7Ratio");
        t.tupleItem("CalLyr0Ratio");
        t.tupleItem("CalXtalRatio");
        t.tupleItem("EvtLogEnergy");
        t.tupleItem("EvtECalTrackDoca");
        t.tupleItem("EvtECalTrackSep");
        t.tupleItem("EvtECalXtalTrunc");
        t.tupleItem("EvtECalXtalRatio");
        t.tupleItem("CalDeltaT");
        t.tupleItem("CalLATEdge");
        t.tupleItem("TkrBlankHits");
        t.tupleItem("TkrThickHits");
        t.tupleItem("EvtECalTrackDoca");
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
        m_firstLayer          = t.tupleItem("Tkr1FirstLayer");
        m_calEnergySum        = t.tupleItem("CalEnergyRaw");
        m_calTotRLn           = t.tupleItem("CalTotRLn");
        m_evtEnergySumOpt     = t.tupleItem("EvtEnergyCorr");
        m_evtTkrEComptonRatio = t.tupleItem("EvtETkrComptonRatio");
        m_evtTkrComptonRatio  = t.tupleItem("EvtTkrComptonRatio");
        m_calMIPDiff          = t.tupleItem("CalMIPDiff");
        m_acdTileCount        = t.tupleItem("AcdTileCount");
        m_vtxAngle            = t.tupleItem("VtxAngle");

        // New items to create or override
        m_goodCalProb = getTupleItemPointer(t,"CTgoodCal");
        m_vtxProb=      getTupleItemPointer(t,"CTvertex");
        m_goodPsfProb = getTupleItemPointer(t,"CTgoodPsf");
        m_gammaProb=    getTupleItemPointer(t,"CTgamma");

        /** @page ctree Classification Tree variables

    - CTgoodCal  Good measurement of the Calorimeter  
    - CTvertex   The vertex measure of the incoming direction is better than the first track 
    - CTgoodPsf  The incoming direction is well measured  (PSF is good)    
    - CTgamma    The event is a gamma, not background
    */



        facilities::Util::expandEnvVar(&treepath);
        m_factory = new GlastClassify::TreeFactory(treepath, looker);

        for( unsigned int i=0; i<NODE_COUNT; ++i){
           (*m_factory)(imNodeInfo[i].name);
        }
    }

    //_________________________________________________________________________ 

    bool ClassificationTree::useVertex()const
    {
        return *m_vtxAngle>0 && *m_vtxProb >0.5;
    }

    //_________________________________________________________________________

    void ClassificationTree::execute()
    {

        // initialize CT output variables
        *m_goodPsfProb=0;
        *m_vtxProb  = *m_gammaProb=0;

        int cal_type = ( *m_calEnergySum >5. && *m_calTotRLn > 4.)? GOODCAL : NOCAL;
        // evalue appropriate tree for good cal prob
        if( cal_type==NOCAL) return;
        *m_goodCalProb = m_factory->evaluate(cal_type);

        // evaluate the rest only if cal prob ok
        if( *m_goodCalProb<0.25 )   return;

 
        // select vertex-vs-1 track, corresponding tree for core, psf

        int goodPsfType=0; 
        if( *m_firstLayer > 5 ) { 

            *m_vtxProb = m_factory->evaluate(VTX_THIN); 
            goodPsfType = useVertex() ? VTX_THIN_TAIL : ONE_TRK_THIN_TAIL;
        }else {
            *m_vtxProb = m_factory->evaluate(VTX_THICK);
            goodPsfType = useVertex() ? VTX_THICK_TAIL : ONE_TRK_THICK_TAIL;
        }

        // now evalute the appropriate trees
        *m_goodPsfProb   = m_factory->evaluate(goodPsfType);

        // gamma prob: for now just the preliminary cuts
        *m_gammaProb  = m_background?  0 : 1;

    }

    //_________________________________________________________________________

    ClassificationTree::~ClassificationTree()
    {

        delete m_factory;
        delete &m_background;
    }
