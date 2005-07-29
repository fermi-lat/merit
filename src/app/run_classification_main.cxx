/** @file run_classification_main.cxx 
@brief Application that applies decision trees to the tuple

$Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/run_classification_main.cxx,v 1.5 2005/07/29 01:10:24 burnett Exp $
*/

#include "app/RootTuple.h"

#include "../ClassificationTree.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <stdexcept>

#include "TFile.h"
#include "TTree.h"

/** 
@page run_classification application run_classification 
format: run_classification [input_file] [output_file]

@param input_file if not present look at env var MERIT_INPUT_FILE
@param output_file if not present, and MERIT_OUTPUT_FILE is not defined, just append "_new" to the file name.

Copies the root tree MeritTuple (or "1") from input_file to output_file, but recalculates the CT variables,
Expects the env var CTREE_PATH to point to a folder containing the trees.


*/

int main(int argc, char* argv[])
{
    int rc = 0;
    try {

        std::string  input_filename(""), output_filename(""), tree_name("MeritTuple");
        int n=0;
        if( argc>++n ) input_filename = argv[n];		// required
        if( argc>++n ) output_filename = argv[n];		// required

        if( input_filename=="" ) {
            const char * env = ::getenv("MERIT_INPUT_FILE");
            if( env ) input_filename=env;
            else {
                throw std::invalid_argument( "No input file specified");
            }
        }
        if( output_filename=="" ) {
            const char * env = ::getenv("MERIT_OUTPUT_FILE");
            if( env ) output_filename=env;
            else {
                // make up output file from input
                int find = input_filename.find(".root");
                output_filename = input_filename.substr(0,find)+"_new.root";
            }
        }


        std::cerr << "Converting CT variables from: \"" << input_filename << "\" to\n\t\"" 
            << output_filename << "\"" << std::endl;

        RootTuple* tuple = new RootTuple("unknown", input_filename, tree_name);

        std::stringstream title; 
        title << "gen(" << tuple->numEvents() << ")";
        tuple->setTitle(title.str());
        tuple->loadBranches();

        // set up the output Root file, branch

        TFile out_file(output_filename.c_str(), "recreate");
        TTree* out_tree = new TTree(tree_name.c_str(), tree_name.c_str());
        for(Tuple::iterator tit =tuple->begin(); tit != tuple->end(); ++tit){
            TupleItem& item = **tit;
            double * val = &item.value();
            std::string leaf_name = item.isFloat() ? item.name() : item.name()+"/D";
            out_tree->Branch(item.name().c_str(), (void*)val, leaf_name.c_str());
        }

        const char* ctree = ::getenv("CTREE_PATH");
        // create the ct: pass in the tuple.
        ClassificationTree pct(*tuple, std::cout, ctree!=0? std::string(ctree) : "");

        int k=0;

        while ( tuple->nextEvent() ) { 
            pct.execute();   // fill in the classification (testing here)
            out_file.cd();
            out_tree->Fill();
            ++k;
        }
        out_file.Write();
        std::cout << "Wrote " << k << " entries" << std::endl;

    }catch(std::exception& e){
        std::cerr << "Caught: " << e.what( ) << std::endl;
        std::cerr << "Type: " << typeid( e ).name( ) << std::endl;
        rc=1;
    }catch(...) {
        std::cerr << "Unknown exception from classification " << std::endl;
        rc=2;
    }
    return rc;
}


