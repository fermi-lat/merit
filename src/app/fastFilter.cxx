/** @file  fastFilter.cxx 
$Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/fastFilter.cxx,v 1.5 2004/01/06 18:44:00 hansl Exp $
*/

#include "app/RootTuple.h"
#include "../PruneTree.h"

#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cassert>



/** Application: fastFilter
 *                                                     
 *   - Read merit input, apply loose filter selection.                           
 *   - Add filter category + probability to Tuple.                               
 *   - Write out new ROOT Ntuple with filtered events add the branches 
 *     gamma category and probability.                                           
 * 
 *  Usage: fastFilter  [input_file] [output_file]                                
 *
 *   - input_file  : if not provided, use env var MERIT_INPUT_FILE               
 *   - output_file : if not provided, and MERIT_OUTPUT_FILE is not defined, 
 *                   append "_filt" to the input_file name                       
 *
 *   - imfile      : IM xml file defined by env var IM_FILE_FILTER, 
 *                   if not defined, use default $MERITROOT/xml/CTPruner_DC1.imw 
 *   - gammaProbCut: cut on probability of event being a gamma candidate, 
 *                   at present set in the code.        
 * 
 *  Environment variables are defined in the requirements
 * _____________________________________________________________________________
 */

int main(int argc, char* argv[])
{
    int    debug = 1;           // debug level increasing with value
    double gammaProbCut (0.0);  // cut on probability that event is a gamma candidate
                                // = 0.0 for test of filter 

    // Set up input, output files and IM XML file
    //_________________________________________________________________________

    std::string  input_filename(""), output_filename(""), tree_name("MeritTuple");
    int n=0;
    if( argc>++n ) input_filename  = argv[n];
    if( argc>++n ) output_filename = argv[n];

    if( input_filename == "" ) {
        const char * env = ::getenv("MERIT_INPUT_FILE");
        if( env ) input_filename = env;
        else {
            std::cerr << "No input file specified" << std::endl;
            exit(1);
        }
    }
    if( output_filename == "" ) {
        const char * env = ::getenv( "MERIT_OUTPUT_FILE" );
        if( env ) output_filename = env;
        else {
            // make up output file from input
            int find = input_filename.find(".root");
            output_filename = input_filename.substr(0,find)+"_filt.root";
        }
    }

    // Get or generate the IM xml filename 
    std::string xml_file = ::getenv("IM_FILE_FILTER");
    if ( xml_file.empty() ) { 
      std::string  default_file( "/xml/CTPruner_DC1.imw" );
      const char *sPath = ::getenv("MERITROOT");
      xml_file = std::string( sPath == 0 ?  "../" : sPath) + default_file;
    }
    std::cout << "fastFilter: IM xml file " 
              << "\n\t" << xml_file << std::endl;


    //_________________________________________________________________________

    //  Create "vector" of input TupleItems
    RootTuple* intuple = new RootTuple( "unknown", input_filename, tree_name );

    std::stringstream title; 
    title << "gen(" << intuple->numEvents() << ")";
    intuple->setTitle(title.str());
    intuple->loadBranches(); // create TupleItems from input tree

    std::cout << "fastFilter: Input merit Tuple "
              << "\n\t number of events "  << intuple->numEvents()
              << "\n\t " << input_filename << "\"" << std::endl;


    // create the PruneTree, which adds more TupleItems to the input RootTuple 
    PruneTree pruner( *intuple,  xml_file ); 


    
    // Set up the output file, tree and branches
    //_________________________________________________________________________

    std::cout << "FastFilter: Output enhanced merit Tuple: "
              << "\n\t " << output_filename   << " \"" << __DATE__ << "\""
              <<std::endl;

    // set up the output Root file and Root Tree
    std::string outTitle( "Filtered " );
    outTitle += __DATE__;
    TFile  out_file( output_filename.c_str(), "recreate",  outTitle.c_str() );
    TTree* out_tree = new TTree(tree_name.c_str(), outTitle.c_str() ); //name,title 

    // Create branches of out_tree from the TupleItems of the (enhanced) input Tuple
    for( Tuple::iterator tit = intuple->begin(); tit != intuple->end(); ++tit) {
         TupleItem& item = **tit;
         Double_t * val  = &item.value();
	 std::string leaf_name = item.isFloat() ? item.name() : item.name()+"/D";
         out_tree->Branch( item.name().c_str(), val, leaf_name.c_str() );
         if ( debug >3 ) {
           std::cout << " name      " << item.name() 
                     << " value     " << val 
                     << " leaf name " << leaf_name << std::endl; }
    }

    std::cout << "fastFilter: Number of branches "<<out_tree->GetNbranches() <<std::endl;
    


    //  Loop over events, select and update output Tuple 
    //_________________________________________________________________________

    int nBytes   = 0;
    int nEntries = 0;
    try {
	// event loop
          while ( intuple->nextEvent() ) { 
	    double prob = pruner();
            if (prob >= gammaProbCut ) {  
              
              out_file.cd();
              nBytes = out_tree->Fill();
              ++nEntries;
            }  // check prob  
          }  // loop events

        }catch(std::exception& e){
            std::cerr << "Caught: " << e.what( ) << std::endl;
            std::cerr << "Type:   " << typeid(e).name( ) << std::endl;
        }catch(...) {
            std::cerr << "Unknown exception from classification " << std::endl;
        }

       out_file.ls();
       //out_file.Map();

       if (debug>2) { out_tree->Print();}
       out_tree->ls();

       // out_tree->BuildIndex("EvtRun", "EvtEventID");

     out_file.Write();
     out_file.Close();
     std::cout << "fastFilter: " << nEntries 
               << " events selected for filtered file " << std::endl;
        
     return 0;
}
