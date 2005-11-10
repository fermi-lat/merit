//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/mainpage.h,v 1.12 2005/08/11 21:56:00 burnett Exp $
// (Special "header" just for doxygen)

/*! @mainpage  package merit

<hr>
  \section meritAlg  meritAlg

  Implements an algorithm, meritAlg, that:
  
  - does a PSF and Aeff analysis. 
  - runs the classification trees and adds the variables to the tuple. (see the MeritTuple page)
  - adds FT1 variables to the tuple (see the MeritTuple page)

  <h3> Properties for meritAlg </h3> 
    @param  meritAlg.cuts ["LntA"]  remove the "t" to disable tree-based tail cuts
    @param  meritAlg.generated [10000]  number of events that were generated, for Aeff estimate
    @param  meritAlg.EventTreeName  ["MeritTuple"]  Name to give the event tree [if null, turn off]

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

<h3> Sample output from meritAlg for GlastRelase HEAD1.617 </h3>

<pre>
        Generated events :   1000
=======================================================
Analysis cuts: LntA
          Found in tuple :    299
-------------------------------------------------------
  Generated energy--mean :   1000
                     rms :      0
                     min :   1000
                     max :   1000
     Elapsed time (sec): :   2464
          TkrNumTracks>0 :    185
CT PSF tail cuts
          CTgoodCal>0.25 :    156
          Tkr1Zdir<-0.25 :    156
-------------------------------------------------------
   Accepted for analysis :    156
-------------------------------------------------------
 Layers 6-15  All
             Events used :     68
        eff. proj. sigma :  0.284 deg = 17 arc-min
           68% contained :  0.662 deg = 1.55*(1.51*sigma)
           95% contained :   6.68 deg = 9.62*(2.45*sigma)
        Energy: meas/gen :  0.892
                     std :  0.263
       events w/ no data :      0
          effective area :   4080 cm^2
         Figure of merit :  12900 cm
-------------------------------------------------------
 Layers 0-5  All
             Events used :     73
        eff. proj. sigma :  0.491 deg = 29.5 arc-min
           68% contained :   0.98 deg = 1.32*(1.51*sigma)
           95% contained :   5.37 deg = 4.46*(2.45*sigma)
        Energy: meas/gen :  0.973
                     std :  0.183
       events w/ no data :      0
          effective area :   4380 cm^2
         Figure of merit :   7716 cm
-------------------------------------------------------
    total effective area :   9360 cm^2
            Combined FOM :  15031 cm



</pre>

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
