//$Header: /nfs/slac/g/glast/ground/cvs/merit/src/mainpage.h,v 1.2 2002/05/28 04:02:50 burnett Exp $
// (Special "header" just for doxygen)

/*! @mainpage  package merit
  <br>
  Implements an algorithm, meritAlg, that does a complete PSF and Aeff analysis.
  <br>


Sample output, the first sensible Gleam-related results, from Gleam v2r1p5. 
<hr>
<pre>
         Generated events :   1000
=======================================================
Analysis cuts: nA
          Found in tuple :    221
-------------------------------------------------------
  Generated energy--mean :    0.1
                     rms :      0
                     min :    0.1
                     max :    0.1
         TKR_No_Tracks>0 :    190
   Accepted for analysis :    190
-------------------------------------------------------
 Layers 0-11
             Events used :    103
        eff. proj. sigma :   1.85 deg = 111 arc-min
           68% contained :   3.88 deg = 1.39*(1.51*sigma)
           95% contained :   26.4 deg = 5.85*(2.45*sigma)
        Energy: meas/gen :  0.314
                     std :  0.172
       events w/ no data :      0
          effective area :   6180 cm^2
         Figure of merit :   2440 cm
-------------------------------------------------------
 Layers 12-15
             Events used :     87
        eff. proj. sigma :    3.3 deg = 198 arc-min
           68% contained :   6.18 deg = 1.24*(1.51*sigma)
           95% contained :   17.2 deg = 2.13*(2.45*sigma)
        Energy: meas/gen :  0.528
                     std :  0.197
       events w/ no data :      1
          effective area :   5220 cm^2
         Figure of merit :   1254 cm
-------------------------------------------------------
    total effective area :  11400 cm^2
            Combined FOM :   2743 cm
</pre>

  <hr>
  \section notes release notes
  \include release.notes
  \section requirements requirements
  \include requirements

*/