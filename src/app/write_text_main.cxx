/** @file write_text_main.cxx 
$Header: /nfs/slac/g/glast/ground/cvs/merit/src/app/meritapp.cxx,v 1.23 2003/11/15 15:25:03 burnett Exp $
*/

#include "app/RootTuple.h"

#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <fstream>

#include "TFile.h"
#include "TTree.h"

/**

format: write_text [input_file] [output_file]

input_file: if not present look at env var MERIT_INPUT_FILE
output_file: if not present, and MERIT_OUTPUT_FILE is not defined, just append "_new" to the file name.

Copies the root tree MeritTuple (or "1") from a root input_file to a text (tab-delimeted) output_file 


*/

int main(int argc, char* argv[])
{
    std::string  input_filename(""), output_filename(""), tree_name("MeritTuple");
    int n=0;
    if( argc>++n ) input_filename = argv[n];		
    if( argc>++n ) output_filename = argv[n];	

    if( input_filename=="" ) {
        const char * env = ::getenv("MERIT_INPUT_FILE");
        if( env ) input_filename=env;
        else {
            std::cerr << "No input file specified" << std::endl;
            exit(1);
        }
    }
    if( output_filename=="" ) {
        const char * env = ::getenv("MERIT_OUTPUT_FILE");
        if( env ) output_filename=env;
        else {
            // make up output file from input
            int find = input_filename.find(".root");
            output_filename = input_filename.substr(0,find)+".txt";
        }
    }


    std::cerr << "ROOT to txt copy from \"" << input_filename << "\" \n\tto \"" 
        << output_filename << "\"" << std::endl;

    RootTuple tuple("", input_filename, tree_name); // no title line if no title

    std::stringstream title; 
      tuple.loadBranches();

    std::ofstream outfile(output_filename.c_str());
    tuple.writeHeader(outfile);

    int m=0;
    // set up the output Root file, branch
    while ( tuple.nextEvent() ) { 
        outfile << tuple; 
        ++m;
    }
    std::cerr << "Copied "<< m << " records" << std::endl;
    return 0;
}


