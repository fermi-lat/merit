//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/mainpage.h,v 1.10 2004/12/22 16:06:46 burnett Exp $
// (Special "header" just for doxygen)

/*! @mainpage  package merit

<hr>
  \section meritAlg  meritAlg

  Implements an algorithm, meritAlg, that does a PSF and Aeff analysis.

  <h3> Properties for meritAlg </h3> 
    @param  meritAlg.cuts ["LntA"]  remove the "t" to disable tree-based tail cuts
    @param  meritAlg.generated [10000]
    @param  meritAlg.RootFilename  [""]
    @param  meritAlg.IM_filename ["$(MERITROOT)/xml/classification.imw"]  set this to null to not run the classification
    @param  meritAlg.PrimaryType ["RECO"] or "MC" (why not a bool?)
    @param  meritAlg.NbOfEvtsInFile [100000]  to define FT1 event ID. (see code)

  <h3> Keys Used in FigureOfMerit::setCuts </h3>
  
  <pre>
Cut keys are as follows:"
Gnnnn, : Set number generated to nnnnn, overriding gen(nnn) in tuple title"
1 : level 1 trigger: Track or LoCal or HiCal "
V : level 1 VETO throttle "
2 : level 2 trigger: track, no doca veto except if cal "
3 : level 3 trigger"
F : TKR FRONT Section only"
B : TKR BACK Section only"
n : number of tracks (N_tracks > 0)"
c : cosmic rejection cuts (old set)"
  \"(...)\" : cut expression, like Chisq<50 ( chars ()<>  must be enclosed in quotes)"
X : Xname, -- statistics on name"
A : Analysis of PSF, etc."                                                    \
W : write the (ascii) event to standard output (useful for filtering tuples!)"
L : elapsed time: last time found in variable 'elapsed_time'"
R : Rate: number of events/elapsed time"
s : size: report on event size (assuming various bits/hit)"
Mx: Multi-PSF for bin x, x=0,1,2,3,4: do PSF analysis for 6 dE/E bins from "
    31 MeV to 3.1 GeV, cos theta #n, bins are 0.2 wide"
    x=a: all bins from 1 to 0.2, 0.2 bins"
      b: all bins from 1 to 0.2, 0.1 bins"
      n: one bin  from 1 to 0, appropriate for normal incidence, or average"
      i: all costh bins but one energy bin, appropriate for single energy"

  </pre>

<h3> Sample output from meritAlg for Gleam v2r1p5 </h3>

<pre>
        Generated events :  10000
=======================================================
Analysis cuts: nA
          Found in tuple :   2111
-------------------------------------------------------
  Generated energy--mean :    0.1
                     rms : 1.35473e-008
                     min :    0.1
                     max :    0.1
         TKR_No_Tracks>0 :   1734
   Accepted for analysis :   1734
-------------------------------------------------------
 Layers 0-11
             Events used :    920
        eff. proj. sigma :   1.94 deg = 116 arc-min
           68% contained :   3.97 deg = 1.36*(1.51*sigma)
           95% contained :   17.7 deg = 3.73*(2.45*sigma)
        Energy: meas/gen :  0.306
                     std :  0.172
       events w/ no data :     12
          effective area :   5520 cm^2
         Figure of merit :   2196 cm
-------------------------------------------------------
 Layers 12-15
             Events used :    814
        eff. proj. sigma :   3.97 deg = 238 arc-min
           68% contained :   7.69 deg = 1.28*(1.51*sigma)
           95% contained :   20.9 deg = 2.15*(2.45*sigma)
        Energy: meas/gen :  0.535
                     std :  0.193
       events w/ no data :     19
          effective area :   4884 cm^2
         Figure of merit :   1009 cm
-------------------------------------------------------
    total effective area :  10404 cm^2
            Combined FOM :   2416 cm

</pre>
<hr>
\section fastFilter Utility fastFilter: Event selection applying Insightful Miner selection 
 <h3>Usage </h3>
      @verbatim
      fastFilter.exe   [input_merit.root]   [output.root]
         input_merit.root    default   src/test/merit100.root
         output.root         default   <input_merit.root>_filt.root

         imfile              IM xml file defined by env var IM_FILE_FILTER
      @endverbatim
      Default path to input and output Root files and the path to the IM xml
      file are set in the requirements. 

 <h3> Purpose </h3>
      Read the input merit Root Ntuple and generate a new output Root Ntuple
      with selected events only. Additional branches are added, which contain
      the event category and gamma probability. The selection is tuned to reduce 
      the background (typically by a factor of 5) and keep the gamma signal. 
      The selection uses the IM xml file xml/CTPruner_DC1.imw (from Bill Atwood). 
      <p>
 <h3> Method </h3>
   <ul>
      <li>The exclusive leaves of the classification tree (CT), file CTPruner_DC1.imw,
      are implemented in 
      PruneTree::PreClassify. This subclass immitates the decision chain of the CT. 
      The result is per event the leave category, which in turn is used to evaluate the
      gamma probability of the event via the classification::Tree 
      (package classification). 
      <li>
      PruneTree creates an instance of PruneTree::PreClassify and  
      classification::Tree. 
      <li>
      The interface to the branches of the Root Tree is established  via RootTuple, a
      vector of TupleItem, each holding a pair of name and value. Additional 
      TupleItems are added, which store per event the resulting CT category and 
      gamma probability. PruneTree::Lookup is a helper class, which provides access 
      to the TupleItem value by name. 
      <li>
      fastFilter.cxx (main) defines the input and output root files and the 
      IM xml file. It creates the interface RootTuple. It then loops over the events 
      of the input Root tuple, extracts the event probability and fills the new
      Root tuple depending on this probability (value now hard coded). 
      <li> Additional documentation:
           <ul><li> 
           <a href=http://www.slac.stanford.edu/~hansl/soft/glastSw/fastFilter/CTPruner.jpg>
           IM Classification Tree </a> 
           <li>Plot of 
           <a href=http://www.slac.stanford.edu/~hansl/soft/glastSw/fastFilter/cat-prob-merit100.ps>
           Leaf category versus probability </a> for test file merit100.root 
           (196 gammas of 100 MeV). 
           </ul>
      <li> It would be easy to limit in addition the number of branches written to
      the output tuple, to further reduce the size of the output files.
      </ul></p>

<h3> Updates of the classification Tree </h3>
     In case that the CT changes the following updates of the code are needed.
       <ul><li> If the IM tree structure changes, the declarations in PruneTree, 
                IMnodeInfo and typedef Category have to be revised. 
           <li> If the structure of the CT is unchanged, only an updated IM xml
                file is needed. The cuts in PruneTree::operator Category() may
                have to be revised. 
           <li> If the classification tree computations must be disabled, 
                  set the joboptions parameter IM_filename to null. Also remove the "t" in the cuts parameter.
       </ul>

  <hr>
  @section notes release notes
  release.notes
  <hr>
  @section requirements requirements
  @verbinclude requirements
  <hr>
    @todo Change name PruneTree to FastFilter and fastFilter to FastFilterApp
    @todo Separate the different applications into subpackages
    @todo Define base class for the common parts of PruneTree and ClassificationTree.
    @todo Waiting for feedback from Bill on some parts of the decision tree implemented
    in PruneTree::PreClassify. 

*/
