/** @file ClassificationTree.cxx
@brief 
$Header: /nfs/slac/g/glast/ground/cvs/merit/src/ClassificationTree.cxx,v 1.35 2005/10/29 20:45:40 burnett Exp $

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
        ENERGY_PARAM,     
        ENERGY_LASTLAYER, 
        ENERGY_PROFILE,   
        ENERGY_TRACKER,   
        VERTEX_THIN, VERTEX_THICK,
        PSF_VERTEX_THIN, 
        PSF_VERTEX_THICK,
        PSF_TRACK_THIN,  
        PSF_TRACK_THICK, 
        GAMMA_VERTEX_HIGH, GAMMA_VERTEX_MED, GAMMA_VERTEX_THIN, GAMMA_VERTEX_THICK,
        GAMMA_TRACK_HIGH,  GAMMA_TRACK_MED,  GAMMA_TRACK_THIN,  GAMMA_TRACK_THICK,
        NODE_COUNT, // stop here
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
        { ENERGY_PARAM,      "energy/param" },
        { ENERGY_LASTLAYER,  "energy/lastlayer" },
        { ENERGY_PROFILE,    "energy/profile" },
        { ENERGY_TRACKER,    "energy/tracker" },

        { VERTEX_THIN,       "vertex/thin"},
        { VERTEX_THICK,      "vertex/thick"},

        { PSF_VERTEX_THIN,   "psf/vertex/thin"}, 
        { PSF_VERTEX_THICK,  "psf/vertex/thick"},
        { PSF_TRACK_THIN,    "psf/track/thin"},
        { PSF_TRACK_THICK,   "psf/track/thick"},
        { GAMMA_VERTEX_HIGH, "gamma/vertex/highcal"},
        { GAMMA_VERTEX_MED,  "gamma/vertex/medcal"},
        { GAMMA_VERTEX_THIN, "gamma/vertex/thin"},
        { GAMMA_VERTEX_THICK,"gamma/vertex/thick"},
        { GAMMA_TRACK_HIGH,  "gamma/track/highcal"},
        { GAMMA_TRACK_MED,   "gamma/track/medcal"},
        { GAMMA_TRACK_THIN,  "gamma/track/thin"},
        { GAMMA_TRACK_THICK, "gamma/track/thick"},
    };

#if 0  // define aliases to deal with new values for now
  using std::string;
  std::pair< string,  string> alias_pairs[]=
      //
      {
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

    std::pair<bool, const void *> operator()(const std::string& name){
        TupleItem* ti = const_cast<TupleItem*>(m_t.tupleItem(name));
        if( ti==0) return std::make_pair(false, (const void*)(0));
        return std::make_pair(ti->isFloat(), &ti->value());
    }

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




//_________________________________________________________________________

ClassificationTree::ClassificationTree( Tuple& t, std::ostream& log, std::string treepath)
: m_log(log)
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
        m_calEnergyRaw        = t.tupleItem("CalEnergyRaw");
        m_calTotRLn           = t.tupleItem("CalTotRLn");
        m_evtTkrEComptonRatio = t.tupleItem("EvtETkrComptonRatio");
        m_evtTkrComptonRatio  = t.tupleItem("EvtTkrComptonRatio");
        m_calMIPDiff          = t.tupleItem("CalMIPDiff");
        m_acdTileCount        = t.tupleItem("AcdTileCount");
        m_vtxAngle            = t.tupleItem("VtxAngle");

        // the energy estimates
        m_CalEnergyCorr    = t.tupleItem("CalEnergyCorr");
        m_CalCfpEnergy     = t.tupleItem("CalCfpEnergy");
        m_CalLllEnergy     = t.tupleItem("CalLllEnergy");
        m_CalTklEnergy     = t.tupleItem("CalTklEnergy");

        // New items to create or override
        m_EvtEnergyCorr=getTupleItemPointer(t,"EvtEnergyCorr");
        m_goodCalProb = getTupleItemPointer(t,"CTgoodCal");
        m_vtxProb=      getTupleItemPointer(t,"CTvertex");
        m_goodPsfProb = getTupleItemPointer(t,"CTgoodPsf");
        m_gammaProb=    getTupleItemPointer(t,"CTgamma");
        m_gammaType=    getTupleItemPointer(t,"CTgammaType");

        /** @page MeritTuple MeritTuple variables
        
        @section CT Classification Tree

        These are generated in merit, calculated from other variables in the tuple, by 
        the decision trees generated from analysis of Monte Carlo data

        @param CTgoodCal  Good measurement of the Calorimeter  
        @param CTvertex   The vertex measure of the incoming direction is better than the first track 
        @param CTgoodPsf  The incoming direction is well measured  (PSF is good)    
        @param CTgamma    The event is a gamma, not background
        @param CTgammaType     integer type for the background rejection tree
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
        *m_vtxProb  = *m_gammaProb = *m_goodCalProb= 0;

        if( calenergy <5. || *m_calTotRLn < 4.) return;

        double energymeasure[] = {*m_CalEnergyCorr, *m_CalCfpEnergy, *m_CalLllEnergy, *m_CalTklEnergy};
        double ctree_index[] = {ENERGY_PARAM, ENERGY_PROFILE, ENERGY_LASTLAYER, ENERGY_TRACKER};
        double bestprob(0), bestenergy(0);
        for( int i =0; i<4; ++i){
            double prob = energymeasure[i]>0? m_factory->evaluate(ctree_index[i]) : 0;
            if( prob>bestprob){
                bestprob = prob;
                bestenergy = energymeasure[i];
            }
        }
        // assign tuple items
        *m_goodCalProb = bestprob;
        *m_EvtEnergyCorr = bestenergy;

        // evaluate the rest only if cal prob ok (should this be wired in???
        if( *m_goodCalProb<0.25 )   return;

        // selection of energy range for gamma trees
        enum {CAL_LOW, CAL_MED, CAL_HIGH};
        double calenergy = *m_calEnergyRaw;
        int cal_type;
        if(     calenergy <  350) cal_type = CAL_LOW;
        else if(calenergy < 3500) cal_type = CAL_MED;
        else                      cal_type = CAL_HIGH;


        // select vertex-vs-track, and corresponding trees for good psf, background rejection

        int psfType=0, gammaType=0;
        if( *m_firstLayer > 5 ) { // thin

            *m_vtxProb = m_factory->evaluate(VERTEX_THIN); 
            if( useVertex() ) {
                psfType = PSF_VERTEX_THIN ;
                if(      cal_type==CAL_HIGH) gammaType=GAMMA_VERTEX_HIGH;
                else if( cal_type==CAL_MED)  gammaType=GAMMA_VERTEX_MED;
                else                         gammaType=GAMMA_VERTEX_THIN;

            }else{ //track
                psfType = PSF_TRACK_THIN;
                if(      cal_type==CAL_HIGH) gammaType=GAMMA_TRACK_HIGH;
                else if( cal_type==CAL_MED)  gammaType=GAMMA_TRACK_MED;
                else                         gammaType=GAMMA_TRACK_THIN;
            }
        }else { //thick
            *m_vtxProb = m_factory->evaluate(VERTEX_THICK);
           if( useVertex() ) {
                psfType = PSF_VERTEX_THIN ;
                if(      cal_type==CAL_HIGH) gammaType=GAMMA_VERTEX_HIGH;
                else if( cal_type==CAL_MED)  gammaType=GAMMA_VERTEX_MED;
                else                         gammaType=GAMMA_VERTEX_THICK;

            }else{ //track
                psfType = PSF_TRACK_THIN;
                if(      cal_type==CAL_HIGH) gammaType=GAMMA_TRACK_HIGH;
                else if( cal_type==CAL_MED)  gammaType=GAMMA_TRACK_MED;
                else                         gammaType=GAMMA_TRACK_THICK;
            }
        }

        // now evalute the appropriate trees
        *m_goodPsfProb = m_factory->evaluate(psfType);

        *m_gammaProb   = m_factory->evaluate(gammaType);
        *m_gammaType   = gammaType-GAMMA_VERTEX_HIGH; // will be 0-7
    }

    //_________________________________________________________________________

    ClassificationTree::~ClassificationTree()
    {
        delete m_factory;
    }
