/** @file PruneTree.cxx
@brief  Apply cuts to filter the background (cuts from Bill Atwood)
*/
#include "PruneTree.h"
#include "analysis/Tuple.h"        // Tuple, TupleItem
#include "classification/Tree.h"   // package classification

#include <cassert>



//___________________________________________________________________________

namespace {

    // Identifiers used for the terminating leaves
    //   Nodes before NODE_COUNT are associated to the vector imNodeInfo and   
    //   method IMpredictNode.evaluate can be used to extract the probability.
    //   Nodes after  NODE_COUNT have no IMnodeInfo and must be evaluated by 
    //   dedicated code, see PruneTree::operator()(). 
   
    typedef enum{ ONE, TWO, THREE,  FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, 
       NODE_COUNT, CAT_HiE, NONE
    } Category;


    // Information per terminating leaf
    //      ( similar to ClassificationTree.cxx )
    //___________________________________________________________________________

    class IMnodeInfo { 
    public:
        int         id;    // unique ID for local identification
        const char* name;  // the name of the Prediction node in the IM xml file
        int         index; // index of the classification type within the list of probabilities
    };

    // The vector imNodeInfo lists the terminating leaves of the IM xml file   
    IMnodeInfo imNodeInfo[] = {
        { ONE,   "Tiles&Hi-E",             1 }, 
        { TWO,   "NoTiles&LowE&GoodXR",    1 }, 
        { THREE, "NoTiles&1Xtals",         1 },
        { FOUR,  "NoTiles&0Xtals",         0 },
        { FIVE,  "Tiles&ACD< -170&GoodXR", 1 },
        { SIX,   "Tiles&ACD<-170&1Xtal",   1 },
        { SEVEN, "Tiles&ACD<-170&0Xtal",   0 }, 
        { EIGHT, "Tiles&ACD<-20&Med-E",    0 },
        { NINE,  "Tiles&ACD>-20&Low-E",    0 }
      
    };


    // Interface to IM node
    //    (same as in ClassificationTree.cxx )
    //___________________________________________________________________________

    class IMpredictNode { 
    public:
        // Locate the node using the node info and ptr to the classification tree 
        //_______________________________________________________________________

        IMpredictNode( const IMnodeInfo& info, const classification::Tree* ctree )
            :  m_offset(info.index), m_tree(ctree)
        {
            m_node = m_tree->getPredictTree(info.name);
            if( m_node==0) std::cerr << "IMpredictNode: Tree " << info.name 
                                     << " not found in classification tree" << std::endl;
            assert(m_node);
        }

        //  Evaluate the probability for this node or leaf
        //______________________________________________________________________

        double evaluate() const {
            return m_tree->navigate(m_node)[m_offset];
        }

     private:
        int m_offset;
        const classification::Tree*        m_tree;
        const classification::Tree::Node*  m_node;
    };

    std::vector<IMpredictNode> imnodes;

}  // anonymous namespace



// Interface to RootTuple and access RootItem_values by name
//    (same as in ClassificationTree.cxx )
//___________________________________________________________________________

class PruneTree::Lookup : 
      public classification::Tree::ILookUpData {

public:

    Lookup( Tuple& t ): m_t(t){}

    // Return pointer to TupleItem value, given the RootItem name
    //_______________________________________________________________________

    const double * operator()(const std::string& name) {
        TupleItem* ti = const_cast<TupleItem*>(m_t.tupleItem(name));
        if( ti==0 ) return 0;
        const double * f = &(ti->value());
        return f;
    }

    // Default type for all TupleItem values is double, 
    // isFloat allows the use of float type
    //_______________________________________________________________________

    bool isFloat() const { return  m_t.isFloat(); }

private:
    Tuple&  m_t;
};


//  Classify the event 
//  for fast filter (eg reduce background events before further analysis)
//  
//     (similar to ClassificationTree::BackgroundCut or 
//       Classificationtree::ClassificationTree  plus  ::execute)
//___________________________________________________________________________

class PruneTree::PreClassify {
public:
    // Constructor: 
    //   associate TupleItems to the branches of the ROOT tree and
    //   add additional TupleItems 
    //_______________________________________________________________________

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
	, CalCsIRLn     (*lookup( "CalCsIRLn"))
    { 
      // Add to current RootTuple
      //   additional item needed for decision
      new TupleItem( "AcdUpperTileCnt",  &AcdUpperTileCnt  ); // must use pointer 
 
      //   tuple items with result of decision
      new TupleItem( "IMFilterCategory", &IMFilterCategory );
      new TupleItem( "IMFilterProb",     &IMFilterProb     );

      std::cout << "PruneTree::PreClassify: RootTuple set up "<< std::endl;
    }

    //  Return Category for current event (ie current row in TTree)
    //_______________________________________________________________________

    operator Category()
    {
      // Bill's IM node names are shown within quotes  "..."

        AcdUpperTileCnt = AcdTileCount - AcdNoSideRow2 - AcdNoSideRow1; // calc always 
	// precondition for accepting the event
        bool NoCal  = !( CalEnergySum > 5. && CalCsIRLn > 2.);
	if ( NoCal ) {  return NONE; }


        // Prepare quantities for decisions 

        bool NoTiles  = AcdTileCount == 0 ; // AcdUpperTileCnt == 0 ?

        bool lowE     = CalEnergySum < 350.;
        bool medE     = CalEnergySum >= 350. && CalEnergySum < 3500.;
        bool HiE      = !(lowE || medE);

        bool GoodXR   = CalXtalRatio > 0.02 && CalXtalRatio < 0.98 ;  // ?
        bool Xtals    = CalXtalsTrunc > 0.0 ;  // ?

        bool Acdlt150 =  AcdActiveDist < -150. ;
	// Tkr1SSDVeto  is the number of hit layers before start of track
        bool Acdlt20  = (AcdActiveDist < -20.0 && AcdRibbonActDist < -20.0) 
                	  || Tkr1SSDVeto > 1 ;  //  ?


        // Simulate the classification tree
        // node "No ACD Tiles"
        if ( NoTiles ) {
	  // node "HiE"
          if ( HiE )       { return CAT_HiE ; }
     
          else {
	    // node  "CalXtalratio"
            if ( GoodXR )  { return ONE; }   // graph uses LowE (= !HiE ?)
	    
            else {
	      // node "Xtals > 0"
              if ( Xtals ) { return TWO;   }
              else         { return THREE; }             
            }
	  }   // end !HiE
        }   // end NoTiles

        
        //  active ACD Tiles
        else { 
          if ( HiE ) { return FOUR; }

          // branch !HiE
          // node "AcdActiveDist < -170" (should be 150)
          else { 
            if ( Acdlt150 ) {
	      // node "CalXtalRatio (23)"
              if ( GoodXR )  { return FIVE; }

              else {
                // node "Xtals > 0"
                if ( Xtals ) { return SIX;   }
		else         { return SEVEN; }
              } 
            }   // end Acdlt150
 
            
            else { 
              // node "AcdActiveDist<-20 & Med-E"
              if ( Acdlt20 ) { return EIGHT; }
              else           { return NINE;  }

            }   // end !Acdlt150
          }   // end !HiE        
	}   // end active ACD Tiles

        return NONE;


    }  // operator PruneTree::PreClassify.Category()

    // set-methods for additional TupleItem values
    void setCategory( double cat )     { IMFilterCategory = cat; }
    void setProbability( double prob ) { IMFilterProb     = prob;}


private:
    // References to the TupleItem values, which were filled from 
    // the Root Tree or added to the Tuple
    const double &   CalEnergySum; 
    const double &   CalXtalRatio;
    const double &   CalXtalsTrunc;
    const double &   AcdActiveDist; 
    const double &   AcdTileCount;
    const double &   AcdNoSideRow1 ;
    const double &   AcdNoSideRow2 ;
    const double &   AcdRibbonActDist;
    const double &   Tkr1SSDVeto; 
    const double &   CalCsIRLn;
  
    //  Additional TupleItems (will be written to the new ROOT tuple)
    double   AcdUpperTileCnt;     // redundant 
    double   IMFilterCategory;    // Category of event
    double   IMFilterProb;        // Probability for gamma  

};  // class PruneTree::PreClassify


/**  Constructor PruneTree
 *
 *  @brief  Set up access to the RootTuple and 
 *          the classification using the IM xml file. 
 *
 *  @param  t           Tuple input file ( = vector of TupleItems)
 *  @param  xml_file    IM xml file, or default $MERITROOT/xml/CTPruner_DC1.imw 
 */ // ___________________________________________________________________________

PruneTree::PruneTree( Tuple& t, std::string xml_file )
{
    // create a lookup object and 
    // pass it to the PreClassifier and the classification tree code
    PruneTree::Lookup looker(t);
    m_preclassify = new PruneTree::PreClassify( looker );
    m_classifier  = new classification::Tree( looker, std::cout, 2); // verbosity=2

    // Analyze the IM xml file
    m_classifier->load( xml_file );

    // Set the list of prediction tree nodes
    imnodes.reserve( NODE_COUNT );
    for( unsigned int i=0; i<NODE_COUNT; ++i){
        IMpredictNode n( imNodeInfo[i], m_classifier );
        imnodes[imNodeInfo[i].id] = n;  
    }
}


//  Destructor 
//___________________________________________________________________________

PruneTree::~PruneTree(){
    delete m_preclassify;
    delete m_classifier;
}



//  Get the probability for the current event
//___________________________________________________________________________

double PruneTree::operator()()
{
   Category  cat = *m_preclassify;

   double prob;
   if      (cat == CAT_HiE) { prob = 1.0; }
   else if (cat == NONE )   { prob = 0.0; }
   else                     { prob = imnodes[cat].evaluate(); }

   m_preclassify->setCategory( (double)cat );
   m_preclassify->setProbability( prob );

   //std::cout <<"PruneTree(): category  "<<cat <<"\t  probability  "<<prob <<std::endl;
   return prob; 
}
