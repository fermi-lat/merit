// This macro for executing merit in the root command window
// Assumes the merit library has been loaded!!
// 
// Tracy Usher (Dec 7, 2001) -- Pearl Harbor Day 60th 

#include "meritPlots.h"
#include "MultiPSF.h"

#include "TFrame.h"
#include "TGaxis.h"
#include "TAxis.h"
#include "TPaveLabel.h"
#include "TLegend.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"

MeritPlots::MeritPlots(const char* tupleFile, const char* cutString)
{
  merit    = new meritFoM(tupleFile, cutString);
  pCan     = 0;
  pads     = 0;
  numPadsX = 0;
  numPadsY = 0;
  nPads    = 0;
}

MeritPlots::~MeritPlots()
{
  if (merit) delete merit;
  if (nPads)
  {
    while(nPads--) delete pads[nPads];
    delete pads;
    numPadsX = 0;
    numPadsY = 0;
  }
  if (pCan)  delete pCan;

  return;
}

void MeritPlots::run()
{
  //if (merit) merit->execute();
  if (merit)
  {
    int numEvents = merit->numTupleEvents();
    int numPassed = 0;
    int idx       = 0;

    while(idx < numEvents)
    {
      if (merit->cutEvent(idx++)) numPassed++;
    }

    printf("** Number events: %i, Number passed: %i\n",numEvents,numPassed);
  }

  return;
}

void MeritPlots::report()
{
  if (merit) merit->report();
}

void MeritPlots::newCanvas(int nx, int ny)
{
  //Get rid of the old one, if it exists
  if (pCan) 
  {
    //First dump the old pads
    while(nPads--) delete pads[nPads];
    numPadsX = 0;
    numPadsY = 0;
    delete pads;
    delete pCan;
  }

  //Open a graphics canvas
  pCan = new TCanvas("MeritPlots","MeritPlots",0,0,700,850);

  pCan->cd();
  pCan->GetFrame()->SetFillColor(46);
  pCan->SetFillColor(10);
  pCan->SetBorderMode(28);

  //Divide into pads...
  numPadsX = nx;
  numPadsY = ny;
  nPads    = numPadsX * numPadsY;

  float  xDiv     = 0.95/numPadsX;
  float  yDiv     = 0.94/numPadsY;

  float  yHigh    = (float)0.95;
  float  yLow     = (float)0.95 - yDiv;
  int    idx      = 0;

  pads = new TPad*[nPads];

  for(int idxy=0; idxy<numPadsY; idxy++)
  {
    float xLow  = (float)0.025;
    float xHigh = xDiv;

    for(int idxx=0; idxx<numPadsX; idxx++)
    {
      char padTitle[6];
      sprintf(padTitle,"pad_%0i",idxx);
      TPad* pPad    = new TPad(padTitle,"Drawing Pad",xLow,yLow,xHigh,yHigh,19);
      pPad->SetFillColor(10);
      pads[idx++]   = pPad;
      xLow          = xHigh;
      xHigh        += xDiv;
    }
    yHigh  = yLow;
    yLow  -= yDiv;
  }

  for(idx=0; idx<nPads; idx++) pads[idx]->Draw();

  return;
}


void MeritPlots::plotPSFvals(int cosBin)
{
  //Do we need a new Canvas?
  if (numPadsX != 2 || numPadsY != 3) newCanvas(2,3);
  pCan->cd();

  //Ok, retrieve pointer to the last object in the AnalysisList
  //which will be the MultiPSF object containing the PSF objects
  AnalysisList* pAnal    = merit->getAnalysisList();
  int           listSize = pAnal->getListSize();

  //Get the last object in the list and make sure it is
  //what we want
  Analyze*  pLast = pAnal->getListItem(listSize-1);

  if (strcmp(pLast->getName(),"PSF analysis"))
  {
    MultiPSF* psf   = (MultiPSF*)pLast;
    int       nObjs = psf->getListSize();

    //Right now the one we want has 24 objects in the list
    if (nObjs == 24)
    {
      int nEneBins  = 6;
      int cosOffSet = 6*cosBin;

      while(nEneBins--)
      {
        PSFanalysis* pPsf     = psf->getListItem(cosOffSet+nEneBins);
        TPad*        pPad     = pads[nEneBins];
        char         newTtl[] = "PSF Analysis, Angle Bin: 0, Energy Bin: 0";

        sprintf(newTtl,"PSF Analysis, Angle Bin: %i, Energy Bin: %i",cosBin,nEneBins);

        pPsf->set_name(newTtl);

        pPad->cd();
        pPad->Clear();
        pPad->SetLogx(0);
        pPad->SetLogy(0);
        pPad->SetGrid(0,0);
        drawPSFplot(pPsf,pPad);
        pPad->Update();
      }
    }
  }  

  return;
}

void MeritPlots::drawPSFplot(PSFanalysis* pPsf, TPad* pPad)
{
  int listSize = pPsf->count();
  int numBins  = listSize / 10;

  if      (numBins < 10)  numBins = 10;
  else if (numBins > 100) numBins = 100;

  float loEdge  = 0.;
  float psf68   = pPsf->percentile(68.) * 180./3.14159;
  float psf95   = pPsf->percentile(95.) * 180./3.14159;

  TH1F* pHist   = new TH1F("PSFval",pPsf->getName(),numBins,loEdge,psf95);
  float* binVal = new float[numBins+2];
  float* binSum = new float[numBins+2];

  //Fill the psf histogram
  while(listSize--) 
  {
    float psfVal = sqrt(pPsf->getDataVal(listSize)) * 180/3.14159;

    pHist->Fill(psfVal,1.);
  }

  //Output the histogram
  pHist->SetStats(0);
  pHist->DrawCopy();
  pPad->Update();

  //Go through and integrate the psf histogram
  int   idx     =  0;
  float histSum = 0;

  binVal[idx]   = 0.;
  binSum[idx]   = 0.;

  while(idx++ <= numBins)
  {
    histSum += pHist->GetBinContent(idx);
    binVal[idx] = pHist->GetBinCenter(idx);
    binSum[idx] = histSum;
  }

  //If non-zero integral then scale the hist
  if (histSum > 0.)
  {
      float intScale = 1.0/histSum;
      idx = numBins+2;
      while(idx--) binSum[idx] *= intScale;
  }

  //Peg the final bin to 0.95 to insure it hits the axis properly...
  binVal[numBins+1] = psf95;
  binSum[numBins+1] = (float)0.95;

  //Set up to overlay the normalized integral histogram
  float yAxisMin = pPad->GetUymin();
  float yAxisMax = pPad->GetUymax();

  TGaxis* axis = new TGaxis(pPad->GetUxmax(), yAxisMin,
                            pPad->GetUxmax(), yAxisMax,
                            0, 1.1, 510, "+L");
  axis->SetLineColor(2); //Red
  axis->DrawClone();

  idx = numBins+2;
  while(idx--) binSum[idx] *= yAxisMax/1.1;

  TGraph* pSum = new TGraph(numBins+2, binVal, binSum);
  pSum->SetLineColor(2);
  pSum->SetBit(1);  //I think this allows pad to kill pSum
  pSum->Draw("l");

  //Extract information for sizing the labelling of the plot
  TAxis*   pAxisX  = pHist->GetXaxis();
  Axis_t   loXEdge = pAxisX->GetXmin();
  Axis_t   hiXEdge = pAxisX->GetXmax();
  Float_t  xDiff   = hiXEdge - loXEdge;

  loXEdge          = hiXEdge - 0.405 * xDiff;
  hiXEdge          = hiXEdge - 0.005 * xDiff;

  Float_t  loYEdge = pHist->GetMinimum();
  Float_t  hiYEdge = pHist->GetMaximum();
  Float_t  yDiff   = hiYEdge - loYEdge;

  loYEdge          = loYEdge + 0.700 * yDiff;
  hiYEdge          = loYEdge + 0.075 * yDiff;
    
  //Create the number of entries label
  char labelText[30];
  sprintf(labelText,"Entries: %6i", pPsf->count());
    
  //Label the overlaid plot
  TPaveLabel* pLabel1 = new TPaveLabel(loXEdge,loYEdge,hiXEdge,hiYEdge,labelText);
  pLabel1->SetBorderSize(0);
  pLabel1->SetFillColor(33);
  pLabel1->SetTextAlign(12);  //Left adjusted, vertically centered
    
  //Now create the 68% label to output
  sprintf(labelText,"68%% point:%6.3f",psf68);
    
  //Label the overlaid plot
  loYEdge -= 0.075 * yDiff;
  hiYEdge -= 0.075 * yDiff;
  TPaveLabel* pLabel2 = new TPaveLabel(loXEdge,loYEdge,hiXEdge,hiYEdge,labelText);
  pLabel2->SetBorderSize(0);
  pLabel2->SetFillColor(33);
  pLabel2->SetTextAlign(12);
    
  //Now create the 95% label to output
  sprintf(labelText,"95%% point:%6.3f",psf95);
    
  //Label the overlaid plot
  loYEdge -= 0.075 * yDiff;
  hiYEdge -= 0.075 * yDiff;
  TPaveLabel* pLabel3 = new TPaveLabel(loXEdge,loYEdge,hiXEdge,hiYEdge,labelText);
  pLabel3->SetBorderSize(0);
  pLabel3->SetFillColor(33);
  pLabel3->SetTextAlign(12);

  pLabel1->DrawClone();
  pLabel2->DrawClone();
  pLabel3->DrawClone();

  //Clean up 
  delete pHist;
  delete binVal;
  delete binSum;
  delete axis;
  delete pLabel1;
  delete pLabel2;
  delete pLabel3;

  return;
}



//This plots the PSF 68% and 95% contours as a function of energy
void MeritPlots::plotPSFvsE(TPad* pPad)
{
  //Do we need a new Canvas?
  if (pPad == 0)
  {
    if (numPadsX != 1 || numPadsY != 1) newCanvas(1,1);
    pPad = pads[0];
  }

  //Ok, retrieve pointer to the last object in the AnalysisList
  //which will be the MultiPSF object containing the PSF objects
  AnalysisList* pAnal    = merit->getAnalysisList();
  int           listSize = pAnal->getListSize();

  //Get the last object in the list and make sure it is
  //what we want
  Analyze*  pLast = pAnal->getListItem(listSize-1);

  if (strcmp(pLast->getName(),"PSF analysis"))
  {
    MultiPSF* psf   = (MultiPSF*)pLast;
    int       nObjs = psf->getListSize();

    //Right now the one we want has 24 objects in the list
    if (nObjs == 24)
    {
      int cosBin = 4;

      TPad* pPad = pads[0];

      pPad->cd();
      pPad->Clear();

      TLegend* pLegend = new TLegend(0.50,0.68,0.9,0.88,"PSF 95% to 65% Containment vs Energy","NDC");

      while(cosBin--)
      {
        int   nEneBins  = 6;
        int   cosOffSet = 6*cosBin;

        float eneMin[6];
        float eneMax[6];
        float eMean[6];
        float psf68[6];
        float psf95[6];

        while(nEneBins--)
        {
          PSFanalysis* pPsf = psf->getListItem(cosOffSet+nEneBins);

          eneMin[nEneBins] = pPsf->minE();
          eneMax[nEneBins] = pPsf->maxE();
          eMean[nEneBins]  = pPsf->meanE();
          psf68[nEneBins]  = pPsf->percentile(68.) * 180./3.14159;
          psf95[nEneBins]  = pPsf->percentile(95.) * 180./3.14159;
        }

        //This will define the drawing area used by TGraph
        if (cosBin == 3)
        {
            float minE = eneMin[0];
            float maxE = eneMax[5];

            TH2F* pArea = new TH2F("drea","PSF vs Energy",2,minE,maxE,2,0.1,1000.);
            pArea->SetXTitle("MeV");
            pArea->SetYTitle("degrees");
            pArea->SetStats(0);
            pArea->DrawCopy();
            pPad->SetGrid(1,1);
            pPad->SetLogx(1);
            pPad->SetLogy(1);
            pCan->GetFrame()->SetFillColor(21);
            pCan->GetFrame()->SetBorderSize(12);
            delete pArea;
        }

        TGraph* e68Hist = new TGraph(6,eMean,psf68);
        e68Hist->SetName("psf68");
        e68Hist->SetLineColor(4-cosBin);
        e68Hist->SetLineStyle(4-cosBin);
        e68Hist->SetMarkerStyle(20+cosBin);
        e68Hist->SetMarkerColor(4-cosBin);

        TGraph* e95Hist = new TGraph(6,eMean,psf95);
        e95Hist->SetName("psf95");
        e95Hist->SetLineColor(4-cosBin);
        e95Hist->SetMarkerStyle(20+cosBin);
        e95Hist->SetMarkerColor(4-cosBin);
        e95Hist->SetLineStyle(4-cosBin);

        e95Hist->SetBit(1);
        e95Hist->Draw("lp");
        e68Hist->SetBit(1);
        e68Hist->Draw("lp");

        char legendTitle[] = "Contours for Angle Bin #i";
        sprintf(legendTitle,"Contours for Angle Bin #%i",cosBin);
        pLegend->AddEntry(e95Hist, legendTitle, "LP");
      }

      pLegend->SetTextColor(4);
      pLegend->SetFillColor(38);
      pLegend->SetBit(1);
      pLegend->Draw();
      pPad->Update();
    }
  }

  return;
}

//This will draw the ratio of the 95% to 68% constainment as
//a function of energy. 

void MeritPlots::plotPSFrat(TPad* pPad)
{
  //Do we need a new Canvas?
  if (pPad == 0)
  {
    if (numPadsX != 1 || numPadsY != 1) newCanvas(1,1);
    pPad = pads[0];
  }

  //Ok, retrieve pointer to the last object in the AnalysisList
  //which will be the MultiPSF object containing the PSF objects
  AnalysisList* pAnal    = merit->getAnalysisList();
  int           listSize = pAnal->getListSize();

  //Get the last object in the list and make sure it is
  //what we want
  Analyze*  pLast = pAnal->getListItem(listSize-1);

  if (strcmp(pLast->getName(),"PSF analysis"))
  {
    MultiPSF* psf   = (MultiPSF*)pLast;
    int       nObjs = psf->getListSize();

    //Right now the one we want has 24 objects in the list
    if (nObjs == 24)
    {
      int       cosBin = 4;

      pPad->cd();
      pPad->Clear();

      TLegend* pLegend = new TLegend(0.50,0.68,0.88,0.88,"PSF 95% to 65% Ratios vs Energy","NDC");

      while(cosBin--)
      {
        int   nEneBins  = 6;
        int   cosOffSet = 6*cosBin;

        float eneMin[6];
        float eneMax[6];
        float eMean[6];
        float psf68[6];
        float psf95[6];
        float psfRat[6];

        while(nEneBins--)
        {
          PSFanalysis* pPsf = psf->getListItem(cosOffSet+nEneBins);

          eneMin[nEneBins] = 1000.*pPsf->minE();
          eneMax[nEneBins] = 1000.*pPsf->maxE();
          eMean[nEneBins]  = 1000.*pPsf->meanE();
          psf68[nEneBins]  = pPsf->percentile(68.) * 180./3.14159;
          psf95[nEneBins]  = pPsf->percentile(95.) * 180./3.14159;
          psfRat[nEneBins] = psf95[nEneBins]/psf68[nEneBins];
        }

        if (cosBin == 3)
        {
            float minE = eneMin[0];
            float maxE = eneMax[5];

            //Ok, this is to define the drawing area...
            TH2F* pArea = new TH2F("drea","PSF Ratio vs Energy",2,minE,maxE,2,0.,15.);
            pArea->SetXTitle("log(MeV)");
            pArea->SetYTitle("log(degrees)");
            pArea->SetStats(0);
            pArea->DrawCopy();
            pPad->SetGrid(1,1);
            pPad->SetLogx(1);
            pPad->SetLogy(0);
            pCan->GetFrame()->SetFillColor(21);
            pCan->GetFrame()->SetBorderSize(12);
            delete pArea;
        }

        TGraph* pPlot = new TGraph(6, eMean, psfRat);
        pPlot->SetLineColor(4-cosBin);
        pPlot->SetLineStyle(4-cosBin);
        pPlot->SetMarkerStyle(20+cosBin);
        pPlot->SetMarkerColor(4-cosBin);
        pPlot->SetBit(1);
        pPlot->Draw("lp");

        char legendTitle[] = "Ratio for Angle Bin #i";
        sprintf(legendTitle,"Ratio for Angle Bin #%i",cosBin);
        pLegend->AddEntry(pPlot, legendTitle, "LP");
      }

      pLegend->SetTextColor(4);
      pLegend->SetFillColor(38);
      pLegend->SetBit(1);
      pLegend->Draw();
      pPad->Update();
    }
  }

  return;
}


void MeritPlots::surfPSFvsE()
{
  //Do we need a new Canvas?
  if (numPadsX != 1 || numPadsY != 2) newCanvas(1,2);
  pCan->cd();

  //Ok, retrieve pointer to the last object in the AnalysisList
  //which will be the MultiPSF object containing the PSF objects
  AnalysisList* pAnal    = merit->getAnalysisList();
  int           listSize = pAnal->getListSize();

  //Get the last object in the list and make sure it is
  //what we want
  Analyze*  pLast = pAnal->getListItem(listSize-1);

  if (strcmp(pLast->getName(),"PSF analysis"))
  {
    MultiPSF* psf   = (MultiPSF*)pLast;
    int       nObjs = psf->getListSize();

    //Right now the one we want has 24 objects in the list
    if (nObjs == 24)
    {
      PSFanalysis* pPsf = psf->getListItem(0);
      float        minE  = log10(pPsf->minE());

      pPsf = psf->getListItem(23);

      float        maxE  = log10(pPsf->maxE());
  
      //Define the two histograms
      TH2F* e68Hist = new TH2F("e68h","PSF 68% vs Energy and angle",6, minE, maxE,4,0.,4.);
      TH2F* e95Hist = new TH2F("e95h","PSF 95% vs Energy and angle",6, minE, maxE,4,0.,4.);
      e68Hist->SetStats(0);
      e68Hist->SetXTitle("log(GeV)");
      e68Hist->SetYTitle("Cos Bin");
      e68Hist->SetZTitle("PSF (degrees)");
      e95Hist->SetStats(0);
      e95Hist->SetXTitle("log(GeV)");
      e95Hist->SetYTitle("Cos Bin");
      e95Hist->SetZTitle("PSF (degrees)");

      int cosBins = 4;

      while(cosBins--)
      {
        int   nEneBins  = 6;
        int   cosOffSet = 6*cosBins;
        float cosBin    = (float)cosBins;

        while(nEneBins--)
        {
          PSFanalysis* pPsf = psf->getListItem(cosOffSet+nEneBins);

          float meanE  = log10(pPsf->meanE());
          float psfVal = pPsf->percentile(68.) * 180./3.14159;

          e68Hist->Fill(meanE, cosBin, psfVal);

          psfVal = pPsf->percentile(95.) * 180./3.14159;
          e95Hist->Fill(meanE, cosBin, psfVal);
        }

        TPad* pPad = pads[0];

        pPad->cd();
        pPad->Clear();
        //pPad->SetLogx(1);
        //pPad->SetLogy(1);
        e95Hist->Draw("SURF1");
        pPad->Update();
        pPad = pads[1];
        pPad->cd();
        pPad->Clear();
        e68Hist->Draw("SURF1");
        pPad->Update();
        //pPad = pads[2];
        //pPad->cd();
        //pPad->Clear();
        //TAxis* pAxis = e95Hist->GetZaxis();
        //float max95z = pAxis->GetXmax();
        //pAxis = e68Hist->GetZaxis();
        //pAxis->SetLimits(0,max95z);
        //e95Hist->Draw("SURF1");
        //e68Hist->Draw("SURF1,same");
        //pPad->Update();

        //delete e68Hist;
        //delete e95Hist;
      }
    }
  }

  return;
}

