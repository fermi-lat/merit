//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/mainpage.h,v 1.4 2002/05/30 02:30:33 burnett Exp $
// (Special "header" just for doxygen)

/*! @mainpage  package merit
  <br>
  Implements an algorithm, meritAlg, that does a complete PSF and Aeff analysis.
  <br>

  \section Keys
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

Sample output, the first sensible Gleam-related results, from Gleam v2r1p5. 
<hr>
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
  \section notes release notes
  \include release.notes
  \section requirements requirements
  \include requirements

*/