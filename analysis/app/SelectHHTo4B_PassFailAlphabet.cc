//================================================================================================
//
// Simple Example
//
//root -l macros/SelectTTBarDileptonControlSample.C+
//
//________________________________________________________________________________________________


#if !defined(__CINT__) || defined(__MAKECINT__)
#include <TROOT.h>                  // access to gROOT, entry point to ROOT system
#include <TSystem.h>                // interface to OS
#include <TFile.h>                  // file handle class
#include <TTree.h>                  // class to access ntuples
#include <TClonesArray.h>           // ROOT array class
#include <vector>                   // STL std::vector class
#include <iostream>                 // standard I/O
#include <iomanip>                  // functions to format standard I/O
#include <fstream>                  // functions for file I/O
#include <string>                   // C++ std::string class
#include <sstream>                  // class for parsing std::strings
#include <TH1D.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <THStack.h>

#include "tdrstyle.C"
#include "CMS_lumi.C"

#endif

double getTriggerEff( TH2F *trigEffHist , double pt, double mass ) {
  double result = 0.0;
  double tmpMass = 0;
  double tmpPt = 0;

  if (trigEffHist) {
      // constrain to histogram bounds
      if( mass > trigEffHist->GetXaxis()->GetXmax() * 0.999 ) {
	tmpMass = trigEffHist->GetXaxis()->GetXmax() * 0.999;
      } else if ( mass < 0 ) {
	tmpMass = 0.001;
	//std::cout << "Warning: mass=" << mass << " is negative and unphysical\n";
      } else {
	tmpMass = mass;
      }

      if( pt > trigEffHist->GetYaxis()->GetXmax() * 0.999 ) {
	tmpPt = trigEffHist->GetYaxis()->GetXmax() * 0.999;
      } else if (pt < 0) {
	tmpPt = 0.001;
	std::cout << "Warning: pt=" << pt << " is negative and unphysical\n";
      } else {
	tmpPt = pt;
      }

      result = trigEffHist->GetBinContent(
				 trigEffHist->GetXaxis()->FindFixBin( tmpMass ),
				 trigEffHist->GetYaxis()->FindFixBin( tmpPt )
				 );

  } else {
    std::cout << "Error: expected a histogram, got a null pointer" << std::endl;
    return 0;
  }

  //std::cout << "mass = " << mass << " , pt = " << pt << " : trigEff = " << result << "\n";

  return result;
}

void PlotDataAndStackedBkg( std::vector<TH1D*> hist , std::vector<std::string> processLabels, std::vector<int> color,  bool hasData, std::string varName, double lumi, std::string label ) {

  TCanvas *cv =0;
  TLegend *legend = 0;

  cv = new TCanvas("cv","cv", 800,700);
  cv->SetHighLightColor(2);
  cv->SetFillColor(0);
  cv->SetBorderMode(0);
  cv->SetBorderSize(2);
  cv->SetLeftMargin(0.16);
  cv->SetRightMargin(0.3);
  cv->SetTopMargin(0.07);
  cv->SetBottomMargin(0.12);
  cv->SetFrameBorderMode(0);

  TPad *pad1 = new TPad("pad1","pad1", 0,0.25,1,1);
  pad1->SetBottomMargin(0.0);
  pad1->SetRightMargin(0.04);
  pad1->Draw();
  pad1->cd();

  legend = new TLegend(0.60,0.50,0.90,0.84);
  legend->SetTextSize(0.04);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);

  THStack *stack = new THStack();
  TH1D *histDataOverMC = (TH1D*)hist[0]->Clone("histDataOverMC");

  if (hasData) {
    for (int i = hist.size()-1; i >= 1; --i) {
      hist[i]->SetFillColor(color[i]);
      hist[i]->SetFillStyle(1001);

      if ( hist[i]->Integral() > 0) {
  	stack->Add(hist[i]);
      }
    }
  } else {
    for (int i = hist.size()-1; i >= 0; --i) {
      hist[i]->SetFillColor(color[i]);
      hist[i]->SetFillStyle(1001);

      if ( hist[i]->Integral() > 0) {
  	stack->Add(hist[i]);
      }
    }
  }

  for (uint i = 0 ; i < hist.size(); ++i) {
    if (hasData && i==0) {
      legend->AddEntry(hist[i],(processLabels[i]).c_str(), "LP");
    } else {
      legend->AddEntry(hist[i],(processLabels[i]).c_str(), "F");
    }
  }

  if (stack->GetHists()->GetEntries() > 0) {
    stack->Draw("hist");
    stack->GetHistogram()->GetXaxis()->SetTitle(((TH1D*)(stack->GetHists()->At(0)))->GetXaxis()->GetTitle());
    stack->GetHistogram()->GetYaxis()->SetTitle(((TH1D*)(stack->GetHists()->At(0)))->GetYaxis()->GetTitle());
    stack->GetHistogram()->GetYaxis()->SetTitleOffset(1.0);
    stack->GetHistogram()->GetYaxis()->SetTitleSize(0.05);
    stack->GetHistogram()->GetXaxis()->SetTitleSize(0.15);
    stack->SetMaximum( 1.2* fmax( stack->GetMaximum(), hist[0]->GetMaximum()) );
    stack->SetMinimum( 0.1 );

    if (hasData) {
     hist[0]->SetMarkerStyle(20);
      hist[0]->SetMarkerSize(1);
      hist[0]->SetLineWidth(1);
      hist[0]->SetLineColor(color[0]);
      hist[0]->Draw("pesame");
    }
    legend->Draw();
 }

  //****************************
  //Add CMS and Lumi Labels
  //****************************
  // lumi_13TeV = "42 pb^{-1}";
  lumi_13TeV = Form("%.1f fb^{-1}", lumi/1000.0);
  writeExtraText = true;
  relPosX = 0.13;
  CMS_lumi(pad1,4,0);

  cv->cd();
  cv->Update();


  TPad *pad2 = new TPad("pad2","pad2", 0,0,1,0.25);
  pad2->SetTopMargin(0.01);
  pad2->SetBottomMargin(0.37);
  pad2->SetRightMargin(0.04);
  pad2->SetGridy();
  pad2->Draw();
  pad2->cd();

  for (int b=0; b<histDataOverMC->GetXaxis()->GetNbins()+2; ++b) {
    double data = 0;
    if (hasData) {
      data = hist[0]->GetBinContent(b);
    }
    double MC = 0;
    double MCErrSqr = 0;
    if (hasData) {
      for (uint i = 1 ; i < hist.size(); ++i) {
	MC += hist[i]->GetBinContent(b);
	MCErrSqr += pow(hist[i]->GetBinError(b),2);
      }
    } else {
      MC = 1;
    }

    if (MC > 0) {
      histDataOverMC->SetBinContent(b, data / MC);
      histDataOverMC->SetBinError(b, (data / MC)*sqrt(1/data + MCErrSqr/pow(MC,2) ));
    } else {
      histDataOverMC->SetBinContent(b, 0);
      histDataOverMC->SetBinError(b, 0);
    }
    //std::cout << "bin " << b << " : " << histDataOverMC->GetBinContent(b) << " " << histDataOverMC->GetBinError(b) << "\n";
  }

  histDataOverMC->GetYaxis()->SetTitle("Data/MC");
  histDataOverMC->GetYaxis()->SetNdivisions(306);
  histDataOverMC->GetYaxis()->SetTitleSize(0.10);
  histDataOverMC->GetYaxis()->SetTitleOffset(0.3);
  histDataOverMC->GetYaxis()->SetRangeUser(0.5,1.5);
  histDataOverMC->GetYaxis()->SetLabelSize(0.10);
  histDataOverMC->GetXaxis()->SetLabelSize(0.125);
  histDataOverMC->GetXaxis()->SetTitleSize(0.15);
  histDataOverMC->GetXaxis()->SetTitleOffset(1.0);
  histDataOverMC->SetLineColor(kBlack);
  histDataOverMC->SetMarkerStyle(20);
  histDataOverMC->SetMarkerSize(1);
  histDataOverMC->SetStats(false);
  histDataOverMC->Draw("pe");

  pad1->SetLogy(false);
  cv->SaveAs(Form("HHTo4B_SR_%s%s.png",varName.c_str(), label.c_str()));
  cv->SaveAs(Form("HHTo4B_SR_%s%s.pdf",varName.c_str(), label.c_str()));

  pad1->SetLogy(true);
  cv->SaveAs(Form("HHTo4B_SR_%s%s_Logy.png",varName.c_str(),label.c_str()));
  cv->SaveAs(Form("HHTo4B_SR_%s%s_Logy.pdf",varName.c_str(),label.c_str()));




}



//=== MAIN MACRO =================================================================================================


void RunSelectHHTo4B(  std::vector<std::pair<std::vector<std::string>,std::string > > datafiles, std::vector<std::vector<std::pair<std::vector<std::string>,std::string > > > bkgfiles, std::vector<std::string> bkgLabels, std::vector<int> bkgColors, std::vector<float> bkgScaleFactors, int channelOption = 1, double totalLumi = 0, std::string label = "") {

  std::string Label = "";
  if (label != "") Label = "_" + label;

  //--------------------------------------------------------------------------------------------------------------
  // Settings
  //==============================================================================================================
  // TFile *triggerEff2016File = new TFile("/afs/cern.ch/work/s/sixie/public/releases/run2/analysis/CMSSW_10_6_8/src/HHBoostedAnalyzer/data/JetHTTriggerEfficiency_2016.root","READ");
  // TFile *triggerEff2017File = new TFile("/afs/cern.ch/work/s/sixie/public/releases/run2/analysis/CMSSW_10_6_8/src/HHBoostedAnalyzer/data/JetHTTriggerEfficiency_2017.root","READ");
  // TFile *triggerEff2018File = new TFile("/afs/cern.ch/work/s/sixie/public/releases/run2/analysis/CMSSW_10_6_8/src/HHBoostedAnalyzer/data/JetHTTriggerEfficiency_2018.root","READ");
  // TH2F *triggerEff2016Hist = (TH2F*)triggerEff2016File->Get("efficiency_ptmass");
  // TH2F *triggerEff2017Hist = (TH2F*)triggerEff2017File->Get("efficiency_ptmass");
  // TH2F *triggerEff2018Hist = (TH2F*)triggerEff2018File->Get("efficiency_ptmass");

  //*****************************************************************************************
  //Set up for input files
  //*****************************************************************************************
  std::vector<std::vector<std::pair<std::vector<std::string>,std::string > > > inputfiles;
  std::vector<std::string> processLabels;
  std::vector<int> color;
  std::vector<float> scaleFactors;

  //*****************************************************************************************
  //Add data files
  //*****************************************************************************************
  inputfiles.push_back(datafiles);
  processLabels.push_back("Data");
  color.push_back(kBlack);
  scaleFactors.push_back(1.0);


  //*****************************************************************************************
  //Add bkg files
  //*****************************************************************************************
  //check all std::vector sizes are correct
  if (!(
	bkgfiles.size() == bkgLabels.size()
	&& bkgfiles.size() == bkgColors.size()
	&& bkgfiles.size() == bkgScaleFactors.size()
	)) {
    std::cout << "Error: std::vector size mismatch. bkgfiles.size() = " << bkgfiles.size() << " , bkgLabels.size() = " << bkgLabels.size() << " , "
	 << "bkgColors.size() = " << bkgColors.size() << " , "
	 << "bkgScaleFactors.size() = " << bkgScaleFactors.size() << " , "
	 << "\n";
    return;
  }
  for (int i=0; i < int(bkgfiles.size()); ++i) {
    inputfiles.push_back(bkgfiles[i]);
    processLabels.push_back(bkgLabels[i]);
    color.push_back(bkgColors[i]);
    scaleFactors.push_back(bkgScaleFactors[i]);
  }

  for (int i=0; i < int(inputfiles.size()); ++i) {
    std::cout << processLabels[i] << " : " << scaleFactors[i] << "\n";
  }

  //*****************************************************************************************
  //Make some histograms
  //*****************************************************************************************
  //------------------------
  //pass histograms
  //------------------------
  std::vector<TH1D*> histMET;
  std::vector<TH1D*> histNLeptons;
  std::vector<TH1D*> histNJetsHaveLeptons;
  std::vector<TH1D*> histJet1Mass;
  std::vector<TH1D*> histJet1Pt;
  std::vector<TH1D*> histJet1DDB;
  std::vector<TH1D*> histJet1PNetXbb;
  std::vector<TH1D*> histJet1Tau3OverTau2;
  std::vector<TH1D*> histJet2Mass;
  std::vector<TH1D*> histJet2Pt;
  std::vector<TH1D*> histJet2DDB;
  std::vector<TH1D*> histJet2PNetXbb;
  std::vector<TH1D*> histJet2Tau3OverTau2;
  std::vector<TH1D*> histHHPt;
  std::vector<TH1D*> histHHMass;
  //-------------------
  //fail histograms
  //-------------------
  std::vector<TH1D*> histMET_fail;
  std::vector<TH1D*> histNLeptons_fail;
  std::vector<TH1D*> histNJetsHaveLeptons_fail;
  std::vector<TH1D*> histJet1Mass_fail;
  std::vector<TH1D*> histJet1Pt_fail;
  std::vector<TH1D*> histJet1DDB_fail;
  std::vector<TH1D*> histJet1PNetXbb_fail;
  std::vector<TH1D*> histJet1Tau3OverTau2_fail;
  std::vector<TH1D*> histJet2Mass_fail;
  std::vector<TH1D*> histJet2Pt_fail;
  std::vector<TH1D*> histJet2DDB_fail;
  std::vector<TH1D*> histJet2PNetXbb_fail;
  std::vector<TH1D*> histJet2Tau3OverTau2_fail;
  std::vector<TH1D*> histHHPt_fail;
  std::vector<TH1D*> histHHMass_fail;

  assert (inputfiles.size() == processLabels.size());
  for (uint i=0; i < inputfiles.size(); ++i) {
    histMET.push_back(new TH1D(Form("histMET_%s",processLabels[i].c_str()), "; MET [GeV] ; Number of Events", 25, 0, 200));
    histNLeptons.push_back(new TH1D(Form("histNLeptons_%s",processLabels[i].c_str()), "; NLeptons ; Number of Events", 10, -0.5, 9.5));
    histNJetsHaveLeptons.push_back(new TH1D(Form("histNJetsHaveLeptons_%s",processLabels[i].c_str()), "; NJetsHaveLeptons ; Number of Events", 3, -0.5, 2.5));
    histJet1Mass.push_back(new TH1D(Form("histJet1Mass_%s",processLabels[i].c_str()), "; Jet1 Mass [GeV] ; Number of Events", 25, 0, 500));
    histJet1Pt.push_back(new TH1D(Form("histJet1Pt_%s",processLabels[i].c_str()), "; Jet1 p_{T} [GeV] ; Number of Events", 25, 0, 2000));
    histJet1DDB.push_back(new TH1D(Form("histJet1DDB_%s",processLabels[i].c_str()), "; Jet1 DDB ; Number of Events", 25, 0, 1.0));
    histJet1PNetXbb.push_back(new TH1D(Form("histJet1PNetXbb_%s",processLabels[i].c_str()), "; Jet1 PNetXbb ; Number of Events", 25, 0, 1.0));
    histJet1Tau3OverTau2.push_back(new TH1D(Form("histJet1Tau3OverTau2_%s",processLabels[i].c_str()), "; Jet1 Tau3OverTau2 ; Number of Events", 25, 0, 1.0));
    histJet2Mass.push_back(new TH1D(Form("histJet2Mass_%s",processLabels[i].c_str()), "; Jet2 Mass [GeV] ; Number of Events", 25, 15, 515));
    histJet2Pt.push_back(new TH1D(Form("histJet2Pt_%s",processLabels[i].c_str()), "; Jet2 p_{T} [GeV] ; Number of Events", 25, 0, 1000));
    histJet2DDB.push_back(new TH1D(Form("histJet1DDB_%s",processLabels[i].c_str()), "; Jet2 DDB ; Number of Events", 25, 0, 1.0));
    histJet2PNetXbb.push_back(new TH1D(Form("histJet2PNetXbb_%s",processLabels[i].c_str()), "; Jet2 PNetXbb ; Number of Events", 25, 0, 1.0));
    histJet2Tau3OverTau2.push_back(new TH1D(Form("histJet1Tau3OverTau2_%s",processLabels[i].c_str()), "; Jet2 Tau3OverTau2 ; Number of Events", 25, 0, 1.0));
    histHHPt.push_back(new TH1D(Form("histHHPt_%s",processLabels[i].c_str()), "; HH p_{T} [GeV] ; Number of Events", 25, 0, 1000));
    histHHMass.push_back(new TH1D(Form("histHHMass_%s",processLabels[i].c_str()), "; m_{HH} [GeV] ; Number of Events", 25, 0, 2000));


    histMET[i]->Sumw2();
    histNLeptons[i]->Sumw2();
    histNJetsHaveLeptons[i]->Sumw2();
    histJet1Mass[i]->Sumw2();
    histJet1Pt[i]->Sumw2();
    histJet1DDB[i]->Sumw2();
    histJet1PNetXbb[i]->Sumw2();
    histJet1Tau3OverTau2[i]->Sumw2();
    histJet2Mass[i]->Sumw2();
    histJet2Pt[i]->Sumw2();
    histJet2DDB[i]->Sumw2();
    histJet2PNetXbb[i]->Sumw2();
    histJet2Tau3OverTau2[i]->Sumw2();
    histHHPt[i]->Sumw2();
    histHHMass[i]->Sumw2();

    //---------------
    //fail histograms
    //---------------
    histMET_fail.push_back(new TH1D(Form("histMET_fail_%s",processLabels[i].c_str()), "; MET [GeV] ; Number of Events", 25, 0, 200));
    histNLeptons_fail.push_back(new TH1D(Form("histNLeptons_fail_%s",processLabels[i].c_str()), "; NLeptons ; Number of Events", 10, -0.5, 9.5));
    histNJetsHaveLeptons_fail.push_back(new TH1D(Form("histNJetsHaveLeptons_fail_%s",processLabels[i].c_str()), "; NJetsHaveLeptons ; Number of Events", 3, -0.5, 2.5));
    histJet1Mass_fail.push_back(new TH1D(Form("histJet1Mass_fail_%s",processLabels[i].c_str()), "; Jet1 Mass [GeV] ; Number of Events", 25, 0, 500));
    histJet1Pt_fail.push_back(new TH1D(Form("histJet1Pt_fail_%s",processLabels[i].c_str()), "; Jet1 p_{T} [GeV] ; Number of Events", 25, 0, 2000));
    histJet1DDB_fail.push_back(new TH1D(Form("histJet1DDB_fail_%s",processLabels[i].c_str()), "; Jet1 DDB ; Number of Events", 25, 0, 1.0));
    histJet1PNetXbb_fail.push_back(new TH1D(Form("histJet1PNetXbb_fail_%s",processLabels[i].c_str()), "; Jet1 PNetXbb ; Number of Events", 25, 0, 1.0));
    histJet1Tau3OverTau2_fail.push_back(new TH1D(Form("histJet1Tau3OverTau2_fail_%s",processLabels[i].c_str()), "; Jet1 Tau3OverTau2 ; Number of Events", 25, 0, 1.0));
    histJet2Mass_fail.push_back(new TH1D(Form("histJet2Mass_fail_%s",processLabels[i].c_str()), "; Jet2 Mass [GeV] ; Number of Events", 25, 15, 515));
    histJet2Pt_fail.push_back(new TH1D(Form("histJet2Pt_fail_%s",processLabels[i].c_str()), "; Jet2 p_{T} [GeV] ; Number of Events", 25, 0, 1000));
    histJet2DDB_fail.push_back(new TH1D(Form("histJet1DDB_fail_%s",processLabels[i].c_str()), "; Jet2 DDB ; Number of Events", 25, 0, 1.0));
    histJet2PNetXbb_fail.push_back(new TH1D(Form("histJet2PNetXbb_fail_%s",processLabels[i].c_str()), "; Jet2 PNetXbb ; Number of Events", 25, 0, 1.0));
    histJet2Tau3OverTau2_fail.push_back(new TH1D(Form("histJet1Tau3OverTau2_fail_%s",processLabels[i].c_str()), "; Jet2 Tau3OverTau2 ; Number of Events", 25, 0, 1.0));
    histHHPt_fail.push_back(new TH1D(Form("histHHPt_fail_%s",processLabels[i].c_str()), "; HH p_{T} [GeV] ; Number of Events", 25, 0, 1000));
    histHHMass_fail.push_back(new TH1D(Form("histHHMass_fail_%s",processLabels[i].c_str()), "; m_{HH} [GeV] ; Number of Events", 25, 0, 2000));


    histMET_fail[i]->Sumw2();
    histNLeptons_fail[i]->Sumw2();
    histNJetsHaveLeptons_fail[i]->Sumw2();
    histJet1Mass_fail[i]->Sumw2();
    histJet1Pt_fail[i]->Sumw2();
    histJet1DDB_fail[i]->Sumw2();
    histJet1PNetXbb_fail[i]->Sumw2();
    histJet1Tau3OverTau2_fail[i]->Sumw2();
    histJet2Mass_fail[i]->Sumw2();
    histJet2Pt_fail[i]->Sumw2();
    histJet2DDB_fail[i]->Sumw2();
    histJet2PNetXbb_fail[i]->Sumw2();
    histJet2Tau3OverTau2_fail[i]->Sumw2();
    histHHPt_fail[i]->Sumw2();
    histHHMass_fail[i]->Sumw2();
  }

  double dataYield        = 0;
  double BkgYield         = 0;
  double SignalYield      = 0;
  double SignalYield_fail = 0;

  //*******************************************************************************************
  //Loop over all input processes
  //*******************************************************************************************
  for (uint i=0; i < inputfiles.size(); ++i) {

    //for duplicate event checking
    std::map<std::pair<uint,uint>, bool > processedRunEvents;

    //loop over dataset years
    for (uint y=0; y < inputfiles[i].size(); ++y) {

      //inputfiles[i][y] is a pair < std::vector<std::string> , std::string >
      std::string year = inputfiles[i][y].second;
      double lumi = 0;
      if (year == "2016") {
	lumi = 35922;
      }
      if (year == "2017") {
	lumi = 41480;
      }
      if (year == "2018") {
	lumi = 59741;
      }

      //loop over files for the process and dataset-year
      for (uint j=0; j < (inputfiles[i][y].first).size(); ++j) {

	TFile *file = new TFile((inputfiles[i][y].first)[j].c_str(),"READ");
	TTree *tree = 0;
	if (!file) {
	  std::cout << "Input file " << (inputfiles[i][y].first)[j] << " could not be opened\n";
	  continue;
	} else {
	  tree = (TTree*)file->Get("tree");
	}
	if (!tree) {
	  std::cout << "Tree could not be found in input file " << (inputfiles[i][y].first)[j] << "\n";
	  continue;
	}

	float weight = 0;
	float triggerEffWeight = 0;
	float pileupWeight = 0;
	float MET = 0;
	float fatJet1Pt = 0;
	float fatJet1MassSD = 0;
	float fatJet1DDBTagger = 0;
	float fatJet1PNetXbb = -99;
	float fatJet1PNetQCDb = -99;
	float fatJet1PNetQCDbb = -99;
	float fatJet1PNetQCDothers = -99;
	float fatJet1Tau3OverTau2 = -99;
	bool fatJet1HasMuon = 0;
	bool fatJet1HasElectron = 0;
	bool fatJet1HasBJetCSVLoose = 0;
	bool fatJet1HasBJetCSVMedium = 0;
	bool fatJet1HasBJetCSVTight = 0;
	float fatJet2Pt = 0;
	float fatJet2MassSD = 0;
	float fatJet2DDBTagger = 0;
	float fatJet2PNetXbb = -99;
	float fatJet2PNetQCDb = -99;
	float fatJet2PNetQCDbb = -99;
	float fatJet2PNetQCDothers = -99;
	float fatJet2Tau3OverTau2 = -99;
	bool fatJet2HasMuon = 0;
	bool fatJet2HasElectron = 0;
	bool fatJet2HasBJetCSVLoose = 0;
	bool fatJet2HasBJetCSVMedium = 0;
	bool fatJet2HasBJetCSVTight = 0;
	float hh_pt = -99;
	float hh_eta = -99;
	float hh_phi = -99;
	float hh_mass = -99;
	float fatJet1PtOverMHH = -99;
	float fatJet1PtOverMSD = -99;
	float fatJet2PtOverMHH = -99;
	float fatJet2PtOverMSD = -99;
	float deltaEta_j1j2 = -99;
	float deltaPhi_j1j2 = -99;
	float deltaR_j1j2 = -99;
	float ptj2_over_ptj1 = -99;
	float mj2_over_mj1 = -99;
	float disc_qcd_2017_basic0  = -99;
	float disc_qcd_2017_basic1  = -99;
	float disc_qcd_2017_basic2  = -99;
	float disc_qcd_2017_enhanced  = -99;
	float disc_qcd_2017_enhanced_v2  = -99;
	float disc_ttbar_2017_basic0  = -99;
	float disc_ttbar_2017_basic1  = -99;
	float disc_ttbar_2017_basic2  = -99;
	float disc_ttbar_2017_enhanced = -99;
	float disc_ttbar_2017_enhanced_v2 = -99;
	float disc_qcd_and_ttbar_2017_basic0  = -99;
	float disc_qcd_and_ttbar_2017_basic1  = -99;
	float disc_qcd_and_ttbar_2017_basic2  = -99;
	float disc_qcd_and_ttbar_2017_enhanced  = -99;
	float disc_qcd_and_ttbar_2017_enhanced_v2  = -99;
	float disc_qcd_and_ttbar_2017_enhanced_v5  = -99;
	float disc_qcd_and_ttbar_2017_enhanced_v6  = -99;
	float disc_qcd_and_ttbar_2017_enhanced_v7  = -99;
	float disc_qcd_and_ttbar_2017_enhanced_v8  = -99;
	float disc_qcd_and_ttbar_2017_enhanced_v10  = -99;
	bool HLT_PFHT1050 = false;
	bool HLT_AK8PFJet360_TrimMass30 = false;
	bool HLT_AK8PFJet380_TrimMass30 = false;
	bool HLT_AK8PFJet400_TrimMass30 = false;
	bool HLT_AK8PFJet420_TrimMass30 = false;
	bool HLT_AK8PFHT750_TrimMass50 = false;
	bool HLT_AK8PFHT800_TrimMass50 = false;
	bool HLT_AK8PFHT850_TrimMass50 = false;
	bool HLT_AK8PFHT900_TrimMass50 = false;
	bool HLT_PFJet450 = false;
	bool HLT_PFJet500 = false;
	bool HLT_PFJet550 = false;
	bool HLT_AK8PFJet450 = false;
	bool HLT_AK8PFJet500 = false;
	bool HLT_AK8PFJet550 = false;
	bool HLT_AK8PFJet330_TrimMass30_PFAK8BTagDeepCSV_p17 = false;
	bool HLT_AK8PFJet330_TrimMass30_PFAK8BTagDeepCSV_p1 = false;
	bool HLT_AK8PFJet330_PFAK8BTagCSV_p1 = false;
	bool HLT_AK8PFJet330_PFAK8BTagCSV_p17 = false;
	bool HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_p02 = false;
	bool HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np2 = false;
	bool HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np4 = false;
	bool HLT_AK8DiPFJet300_200_TrimMass30_BTagCSV_p20 = false;
	bool HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p087 = false;
	bool HLT_AK8DiPFJet300_200_TrimMass30_BTagCSV_p087 = false;
	bool HLT_AK8PFHT600_TrimR0p1PT0p03Mass50_BTagCSV_p20 = false;
	bool HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p20 = false;
	bool HLT_AK8DiPFJet250_200_TrimMass30_BTagCSV_p20	 = false;

	tree->SetBranchAddress("weight",&weight);
	tree->SetBranchAddress("triggerEffWeight",&triggerEffWeight);
	tree->SetBranchAddress("pileupWeight",&pileupWeight);
	tree->SetBranchAddress("MET",&MET);
	tree->SetBranchAddress("fatJet1Pt",&fatJet1Pt);
	tree->SetBranchAddress("fatJet1MassSD",&fatJet1MassSD);
	tree->SetBranchAddress("fatJet1DDBTagger",&fatJet1DDBTagger);
	tree->SetBranchAddress("fatJet1PNetXbb",&fatJet1PNetXbb);
	tree->SetBranchAddress("fatJet1PNetQCDb",&fatJet1PNetQCDb);
	tree->SetBranchAddress("fatJet1PNetQCDbb",&fatJet1PNetQCDbb);
	tree->SetBranchAddress("fatJet1PNetQCDothers",&fatJet1PNetQCDothers);
	tree->SetBranchAddress("fatJet1Tau3OverTau2",&fatJet1Tau3OverTau2);
	tree->SetBranchAddress("fatJet1HasMuon",&fatJet1HasMuon);
	tree->SetBranchAddress("fatJet1HasElectron",&fatJet1HasElectron);
	tree->SetBranchAddress("fatJet1HasBJetCSVLoose",&fatJet1HasBJetCSVLoose);
	tree->SetBranchAddress("fatJet1HasBJetCSVMedium",&fatJet1HasBJetCSVMedium);
	tree->SetBranchAddress("fatJet1HasBJetCSVTight",&fatJet1HasBJetCSVTight);
	tree->SetBranchAddress("fatJet2Pt",&fatJet2Pt);
	tree->SetBranchAddress("fatJet2MassSD",&fatJet2MassSD);
	tree->SetBranchAddress("fatJet2DDBTagger",&fatJet2DDBTagger);
	tree->SetBranchAddress("fatJet2PNetXbb",&fatJet2PNetXbb);
	tree->SetBranchAddress("fatJet2PNetQCDb",&fatJet2PNetQCDb);
	tree->SetBranchAddress("fatJet2PNetQCDbb",&fatJet2PNetQCDbb);
	tree->SetBranchAddress("fatJet2PNetQCDothers",&fatJet2PNetQCDothers);
	tree->SetBranchAddress("fatJet2Tau3OverTau2",&fatJet2Tau3OverTau2);
	tree->SetBranchAddress("fatJet2HasMuon",&fatJet2HasMuon);
	tree->SetBranchAddress("fatJet2HasElectron",&fatJet2HasElectron);
	tree->SetBranchAddress("fatJet2HasBJetCSVLoose",&fatJet2HasBJetCSVLoose);
	tree->SetBranchAddress("fatJet2HasBJetCSVMedium",&fatJet2HasBJetCSVMedium);
	tree->SetBranchAddress("fatJet2HasBJetCSVTight",&fatJet2HasBJetCSVTight);
	tree->SetBranchAddress("hh_pt", &hh_pt);
	tree->SetBranchAddress("hh_eta", &hh_eta);
	tree->SetBranchAddress("hh_phi", &hh_phi);
	tree->SetBranchAddress("hh_mass", &hh_mass);
	tree->SetBranchAddress("disc_qcd_2017_basic0", &disc_qcd_2017_basic0);
	tree->SetBranchAddress("disc_qcd_2017_basic1", &disc_qcd_2017_basic1);
	tree->SetBranchAddress("disc_qcd_2017_basic2", &disc_qcd_2017_basic2);
	tree->SetBranchAddress("disc_qcd_2017_enhanced", &disc_qcd_2017_enhanced);
	tree->SetBranchAddress("disc_qcd_2017_enhanced_v2", &disc_qcd_2017_enhanced_v2);
	tree->SetBranchAddress("disc_ttbar_2017_basic0", &disc_ttbar_2017_basic0);
	tree->SetBranchAddress("disc_ttbar_2017_basic1", &disc_ttbar_2017_basic1);
	tree->SetBranchAddress("disc_ttbar_2017_basic2", &disc_ttbar_2017_basic2);
	tree->SetBranchAddress("disc_ttbar_2017_enhanced", &disc_ttbar_2017_enhanced);
	tree->SetBranchAddress("disc_ttbar_2017_enhanced_v2", &disc_ttbar_2017_enhanced_v2);
	tree->SetBranchAddress("disc_qcd_and_ttbar_2017_basic0", &disc_qcd_and_ttbar_2017_basic0);
	tree->SetBranchAddress("disc_qcd_and_ttbar_2017_basic1", &disc_qcd_and_ttbar_2017_basic1);
	tree->SetBranchAddress("disc_qcd_and_ttbar_2017_basic2", &disc_qcd_and_ttbar_2017_basic2);
	tree->SetBranchAddress("disc_qcd_and_ttbar_2017_enhanced", &disc_qcd_and_ttbar_2017_enhanced);
	tree->SetBranchAddress("disc_qcd_and_ttbar_2017_enhanced_v2", &disc_qcd_and_ttbar_2017_enhanced_v2);
	tree->SetBranchAddress("disc_qcd_and_ttbar_Run2_enhanced_v5p2", &disc_qcd_and_ttbar_2017_enhanced_v5);
	tree->SetBranchAddress("disc_qcd_and_ttbar_2017_enhanced_v6", &disc_qcd_and_ttbar_2017_enhanced_v6);
	tree->SetBranchAddress("disc_qcd_and_ttbar_2017_enhanced_v7", &disc_qcd_and_ttbar_2017_enhanced_v7);
	tree->SetBranchAddress("disc_qcd_and_ttbar_Run2_enhanced_v8p2", &disc_qcd_and_ttbar_2017_enhanced_v8);
	tree->SetBranchAddress("disc_qcd_and_ttbar_Run2_enhanced_v10p2", &disc_qcd_and_ttbar_2017_enhanced_v10);
	tree->SetBranchAddress("fatJet1PtOverMHH", &fatJet1PtOverMHH);
	tree->SetBranchAddress("fatJet1PtOverMSD", &fatJet1PtOverMSD);
	tree->SetBranchAddress("fatJet2PtOverMHH", &fatJet2PtOverMHH);
	tree->SetBranchAddress("fatJet2PtOverMSD", &fatJet2PtOverMSD);
	tree->SetBranchAddress("deltaEta_j1j2", &deltaEta_j1j2);
	tree->SetBranchAddress("deltaPhi_j1j2", &deltaPhi_j1j2);
	tree->SetBranchAddress("deltaR_j1j2", &deltaR_j1j2);
	tree->SetBranchAddress("ptj2_over_ptj1", &ptj2_over_ptj1);
	tree->SetBranchAddress("mj2_over_mj1", &mj2_over_mj1);
	tree->SetBranchAddress("HLT_PFHT1050",                                        &HLT_PFHT1050);
	tree->SetBranchAddress("HLT_AK8PFJet360_TrimMass30",                          &HLT_AK8PFJet360_TrimMass30);
	tree->SetBranchAddress("HLT_AK8PFJet380_TrimMass30",                          &HLT_AK8PFJet380_TrimMass30);
	tree->SetBranchAddress("HLT_AK8PFJet400_TrimMass30",                          &HLT_AK8PFJet400_TrimMass30);
	tree->SetBranchAddress("HLT_AK8PFJet420_TrimMass30",                          &HLT_AK8PFJet420_TrimMass30);
	tree->SetBranchAddress("HLT_AK8PFHT750_TrimMass50",                           &HLT_AK8PFHT750_TrimMass50);
	tree->SetBranchAddress("HLT_AK8PFHT800_TrimMass50",                           &HLT_AK8PFHT800_TrimMass50);
	tree->SetBranchAddress("HLT_AK8PFHT850_TrimMass50",                           &HLT_AK8PFHT850_TrimMass50);
	tree->SetBranchAddress("HLT_AK8PFHT900_TrimMass50",                           &HLT_AK8PFHT900_TrimMass50);
	tree->SetBranchAddress("HLT_PFJet450",                                        &HLT_PFJet450);
	tree->SetBranchAddress("HLT_PFJet500",                                        &HLT_PFJet500);
	tree->SetBranchAddress("HLT_PFJet550",                                        &HLT_PFJet550);
	tree->SetBranchAddress("HLT_AK8PFJet450",                                     &HLT_AK8PFJet450);
	tree->SetBranchAddress("HLT_AK8PFJet500",                                     &HLT_AK8PFJet500);
	tree->SetBranchAddress("HLT_AK8PFJet550",                                     &HLT_AK8PFJet550);
	tree->SetBranchAddress("HLT_AK8PFJet330_TrimMass30_PFAK8BTagDeepCSV_p17",     &HLT_AK8PFJet330_TrimMass30_PFAK8BTagDeepCSV_p17);
	tree->SetBranchAddress("HLT_AK8PFJet330_TrimMass30_PFAK8BTagDeepCSV_p1",      &HLT_AK8PFJet330_TrimMass30_PFAK8BTagDeepCSV_p1);
	tree->SetBranchAddress("HLT_AK8PFJet330_PFAK8BTagCSV_p1",                     &HLT_AK8PFJet330_PFAK8BTagCSV_p1);
	tree->SetBranchAddress("HLT_AK8PFJet330_PFAK8BTagCSV_p17",                    &HLT_AK8PFJet330_PFAK8BTagCSV_p17);
	tree->SetBranchAddress("HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_p02",  &HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_p02);
	tree->SetBranchAddress("HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np2",  &HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np2);
	tree->SetBranchAddress("HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np4",  &HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np4);
	tree->SetBranchAddress("HLT_AK8DiPFJet300_200_TrimMass30_BTagCSV_p20",        &HLT_AK8DiPFJet300_200_TrimMass30_BTagCSV_p20);
	tree->SetBranchAddress("HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p087",       &HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p087);
	tree->SetBranchAddress("HLT_AK8DiPFJet300_200_TrimMass30_BTagCSV_p087",       &HLT_AK8DiPFJet300_200_TrimMass30_BTagCSV_p087);
	tree->SetBranchAddress("HLT_AK8PFHT600_TrimR0p1PT0p03Mass50_BTagCSV_p20",     &HLT_AK8PFHT600_TrimR0p1PT0p03Mass50_BTagCSV_p20);
	tree->SetBranchAddress("HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p20",        &HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p20);
	tree->SetBranchAddress("HLT_AK8DiPFJet250_200_TrimMass30_BTagCSV_p20",        &HLT_AK8DiPFJet250_200_TrimMass30_BTagCSV_p20);


	bool isData = false;
	if ( processLabels[i] == "Data") isData = true;

	std::cout << "process: " << processLabels[i] << " is Data = " << isData << " | file " << (inputfiles[i][y].first)[j] << " | Total Entries: " << tree->GetEntries() << "\n";

	for(UInt_t ientry=0; ientry < tree->GetEntries(); ientry++) {
	  tree->GetEntry(ientry);


	  if (ientry % 100000 == 0) std::cout << "Event " << ientry << std::endl;

	  double puWeight = 1;
	  double myWeight = 1;
	  if (!isData) {
	    myWeight = lumi * weight * triggerEffWeight * pileupWeight * scaleFactors[i];
	    //myWeight = lumi * weight * triggerEffWeight * scaleFactors[i];
	    // myWeight = lumi * weight * scaleFactors[i];
	    //std::cout << "scaleFactors: " << scaleFactors[i] << "\n";
	  }

	  //******************************
	  //Trigger Selection
	  //******************************
	  bool passTrigger = false;

	  if (year == "2016") {
	    passTrigger =
	      (0 == 1)
	      || HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p20
	      || HLT_AK8PFHT600_TrimR0p1PT0p03Mass50_BTagCSV_p20
	      || HLT_AK8DiPFJet250_200_TrimMass30_BTagCSV_p20
	      ;

	    // apply trigger efficiency correction for some triggers that were not enabled for full run
	    if (!isData) {
	      // double triggerSF = 1.0;
	      // if (HLT_AK8DiPFJet280_200_TrimMass30_BTagCSV_p20)                         triggerSF = 1.0;
	      // else if (HLT_AK8PFHT600_TrimR0p1PT0p03Mass50_BTagCSV_p20
	      // 	     || HLT_AK8DiPFJet250_200_TrimMass30_BTagCSV_p20)                 triggerSF = 19.9 / 35.9;
	      // else                                                                      triggerSF = 0;
	      // myWeight = myWeight * triggerSF;

	      passTrigger = true;
	      // double triggerEff = 1.0 -
	      //   (1 - getTriggerEff( triggerEff2016Hist , fatJet1Pt, fatJet1MassSD )) *
	      //   (1 - getTriggerEff( triggerEff2016Hist , fatJet2Pt, fatJet2MassSD ))
	      //   ;
	      // myWeight = myWeight * triggerEff;
	    }

	  }
	  if (year == "2017") {
	    passTrigger =
	      (0 == 1)
	      || HLT_PFJet500
	      || HLT_AK8PFJet500
	      || HLT_AK8PFJet360_TrimMass30
	      || HLT_AK8PFJet380_TrimMass30
	      || HLT_AK8PFJet400_TrimMass30
	      || HLT_AK8PFHT800_TrimMass50
	      || HLT_AK8PFJet330_PFAK8BTagCSV_p17
	      ;

	    // apply trigger efficiency correction for some triggers that were not enabled for full run
	    if (!isData) {
	      // double triggerSF = 1.0;
	      // if (HLT_PFJet500 || HLT_AK8PFJet500)                                    triggerSF = 1.0;
	      // else {
	      //   //std::cout << "fail\n";
	      //   if (HLT_AK8PFJet400_TrimMass30 || HLT_AK8PFHT800_TrimMass50)          triggerSF = 36.42 / 41.48;
	      //   else if (HLT_AK8PFJet380_TrimMass30)                                    triggerSF = 31.15 / 41.48;
	      //   else if (HLT_AK8PFJet360_TrimMass30)                                    triggerSF = 28.23 / 41.48;
	      //   else if (HLT_AK8PFJet330_PFAK8BTagCSV_p17)                              triggerSF = 7.73 / 41.48;
	      //   else                                                                    triggerSF = 0;
	      //   //std::cout << "triggerSF = " << triggerSF << "\n";
	      // }
	      // myWeight = myWeight * triggerSF;

	      passTrigger = true;
	      // double triggerEff = 1.0 -
	      //   (1 - getTriggerEff( triggerEff2017Hist , fatJet1Pt, fatJet1MassSD )) *
	      //   (1 - getTriggerEff( triggerEff2017Hist , fatJet2Pt, fatJet2MassSD ))
	      //   ;
	      // myWeight = myWeight * triggerEff;
	    }


	  }


	  if (year == "2018") {
	    passTrigger =
	      (0 == 1)
	      || HLT_AK8PFJet400_TrimMass30
	      || HLT_AK8PFHT800_TrimMass50
	      || HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np4
	      ;


	    // apply trigger efficiency correction for some triggers that were not enabled for full run
	    if (!isData) {
	      // double triggerSF = 1.0;
	      // if (HLT_AK8PFJet400_TrimMass30 || HLT_AK8PFHT800_TrimMass50)              triggerSF = 1.0;
	      // else if (HLT_AK8PFJet330_TrimMass30_PFAK8BoostedDoubleB_np4)              triggerSF = 54.5 / 59.7;
	      // else                                                                      triggerSF = 0;

	      // myWeight = myWeight * triggerSF;

	      passTrigger = true;
	      // double triggerEff = 1.0 -
	      //   (1 - getTriggerEff( triggerEff2018Hist , fatJet1Pt, fatJet1MassSD )) *
	      //   (1 - getTriggerEff( triggerEff2018Hist , fatJet2Pt, fatJet2MassSD ))
	      //   ;
	      // myWeight = myWeight * triggerEff;
	    }
	  }


	  // if (isData) {
	  //   std::cout << "year = " << year << " : " << passTrigger << "\n";
	  // }

	  if (!passTrigger) continue;


	  //if (isData) continue;

	  //******************************
	  //Selection Cuts
	  //******************************
	  if ( !(fatJet1Pt > 250 )) continue;
	  if ( !(fatJet2Pt > 250 )) continue;
	  if ( !(fatJet1MassSD > 50)) continue;
	  if ( !(fatJet2MassSD > 50)) continue;


	  //SR pre-selection
	  if (channelOption == 0) {
	    if ( !(fatJet1Pt > 350 || fatJet2Pt > 350)) continue;
	    if ( !(fatJet1Pt > 300 && fatJet2Pt > 300 )) continue;
	    if ( !(fatJet1PNetXbb > 0.8)) continue;
	    if ( !(fatJet2PNetXbb > 0.8)) continue;
	  }

	  //SR selection
	  if (channelOption == 1) {



	    //Best 3-bin analysis - Bin2
	    // if ( !(disc_qcd_and_ttbar_2017_enhanced_v8 > 0.023) ) continue;
	    // if ( !(disc_qcd_and_ttbar_2017_enhanced_v8 <= 0.23) ) continue;
	    // if ( !(fatJet2PNetXbb > 0.985)) continue;

 	    // //Best 3-bin analysis - Bin3
	    // if ( !(disc_qcd_and_ttbar_2017_enhanced_v8 > 0.408) ) continue;
	    // if ( !(fatJet2PNetXbb <= 0.985)) continue;
	    // if ( !(fatJet2PNetXbb > 0.945)) continue;



	    //cut-based
	    // if ( !(fatJet1Pt > 350 || fatJet2Pt > 350)) continue;
	    // if ( !(fatJet1Pt > 310 && fatJet2Pt > 310 )) continue;
	    // if ( !(fatJet1PNetXbb > 0.985)) continue;
	    // if ( !(fatJet2PNetXbb > 0.985)) continue;
	    // if ( !(fatJet1MassSD > 105 && fatJet1MassSD < 135)) continue;

	    // baseline cut-based (from Zhicai)
	    // if ( !(fatJet1Pt > 350 || fatJet2Pt > 350)) continue;
	    // if ( !(fatJet1Pt > 300 && fatJet2Pt > 300 )) continue;
	    // if ( !(fatJet1PNetXbb > 0.975)) continue;
	    // if ( !(fatJet2PNetXbb > 0.975)) continue;
	    // if ( !(fatJet1MassSD > 100 && fatJet1MassSD < 140)) continue;

	    //blind the data
      //if data process will just skipp filling the histograms

      // //Best 3-bin analysis - Bin1
	    //if ( !(disc_qcd_and_ttbar_2017_enhanced_v8 > 0.23) ) continue;
	    //if ( !(fatJet2PNetXbb > 0.985)) continue;

      //-------------------------
      //pass requirements
      //-------------------------
      if ( disc_qcd_and_ttbar_2017_enhanced_v8 > 0.23 && fatJet2PNetXbb > 0.985 )
      {
        if (isData)
        {
          if (fatJet2MassSD > 95 && fatJet2MassSD < 135)
          {
            histMET[i]->Fill(MET);
      	    histNLeptons[i]->Fill(0.0);
      	    //histNJetsHaveLeptons[i]->Fill(NJetsHaveLeptons);
      	    histJet1Mass[i]->Fill(fatJet1MassSD);
      	    histJet1Pt[i]->Fill(fatJet1Pt);
      	    histJet1DDB[i]->Fill(fatJet1DDBTagger);
      	    histJet1PNetXbb[i]->Fill(fatJet1PNetXbb);
      	    histJet1Tau3OverTau2[i]->Fill(fatJet1Tau3OverTau2);
      	    //histJet2Mass[i]->Fill(fatJet2MassSD);
      	    histJet2Pt[i]->Fill(fatJet2Pt);
      	    histJet2DDB[i]->Fill(fatJet2DDBTagger);
      	    histJet2PNetXbb[i]->Fill(fatJet2PNetXbb);
      	    histJet2Tau3OverTau2[i]->Fill(fatJet2Tau3OverTau2);
      	    histHHPt[i]->Fill(hh_pt);
      	    histHHMass[i]->Fill(hh_mass);
          }
          else
          {
            histMET[i]->Fill(MET);
      	    histNLeptons[i]->Fill(0.0);
      	    //histNJetsHaveLeptons[i]->Fill(NJetsHaveLeptons);
      	    histJet1Mass[i]->Fill(fatJet1MassSD);
      	    histJet1Pt[i]->Fill(fatJet1Pt);
      	    histJet1DDB[i]->Fill(fatJet1DDBTagger);
      	    histJet1PNetXbb[i]->Fill(fatJet1PNetXbb);
      	    histJet1Tau3OverTau2[i]->Fill(fatJet1Tau3OverTau2);
      	    histJet2Mass[i]->Fill(fatJet2MassSD);
      	    histJet2Pt[i]->Fill(fatJet2Pt);
      	    histJet2DDB[i]->Fill(fatJet2DDBTagger);
      	    histJet2PNetXbb[i]->Fill(fatJet2PNetXbb);
      	    histJet2Tau3OverTau2[i]->Fill(fatJet2Tau3OverTau2);
      	    histHHPt[i]->Fill(hh_pt);
      	    histHHMass[i]->Fill(hh_mass);
          }
        }
        else
        {
          if (processLabels[i] == "HH")
          {
            if ( (fatJet2MassSD > 95 && fatJet2MassSD < 135))
            {
              SignalYield += myWeight;
            }
    	    }
          else
          {
    	      //MCYield += myWeight;//nothing done here
    	    }
    	    histMET[i]->Fill(MET, myWeight);
    	    histNLeptons[i]->Fill(0.0, myWeight);
    	    //histNJetsHaveLeptons[i]->Fill(NJetsHaveLeptons, myWeight);
    	    histJet1Mass[i]->Fill(fatJet1MassSD, myWeight);
    	    histJet1Pt[i]->Fill(fatJet1Pt, myWeight);
    	    histJet1DDB[i]->Fill(fatJet1DDBTagger, myWeight);
    	    histJet1PNetXbb[i]->Fill(fatJet1PNetXbb, myWeight);
    	    histJet1Tau3OverTau2[i]->Fill(fatJet1Tau3OverTau2, myWeight);
    	    histJet2Mass[i]->Fill(fatJet2MassSD, myWeight);
    	    histJet2Pt[i]->Fill(fatJet2Pt, myWeight);
    	    histJet2DDB[i]->Fill(fatJet2DDBTagger, myWeight);
    	    histJet2PNetXbb[i]->Fill(fatJet2PNetXbb, myWeight);
    	    histJet2Tau3OverTau2[i]->Fill(fatJet2Tau3OverTau2, myWeight);
    	    histHHPt[i]->Fill(hh_pt, myWeight);
    	    histHHMass[i]->Fill(hh_mass, myWeight);
        }
      }//end pass section
      //------------------
      //begin fail section
      //------------------
      else if( disc_qcd_and_ttbar_2017_enhanced_v8 > 0.23 && fatJet2PNetXbb <= 0.985 )
      {
        if (isData)
        {
          if (fatJet2MassSD > 95 && fatJet2MassSD < 135)
          {
            histMET_fail[i]->Fill(MET);
      	    histNLeptons_fail[i]->Fill(0.0);
      	    //histNJetsHaveLeptons_fail[i]->Fill(NJetsHaveLeptons);
      	    histJet1Mass_fail[i]->Fill(fatJet1MassSD);
      	    histJet1Pt_fail[i]->Fill(fatJet1Pt);
      	    histJet1DDB_fail[i]->Fill(fatJet1DDBTagger);
      	    histJet1PNetXbb_fail[i]->Fill(fatJet1PNetXbb);
      	    histJet1Tau3OverTau2_fail[i]->Fill(fatJet1Tau3OverTau2);
      	    //histJet2Mass_fail[i]->Fill(fatJet2MassSD);
      	    histJet2Pt_fail[i]->Fill(fatJet2Pt);
      	    histJet2DDB_fail[i]->Fill(fatJet2DDBTagger);
      	    histJet2PNetXbb_fail[i]->Fill(fatJet2PNetXbb);
      	    histJet2Tau3OverTau2_fail[i]->Fill(fatJet2Tau3OverTau2);
      	    histHHPt_fail[i]->Fill(hh_pt);
      	    histHHMass_fail[i]->Fill(hh_mass);
          }
          else
          {
            histMET_fail[i]->Fill(MET);
      	    histNLeptons_fail[i]->Fill(0.0);
      	    //histNJetsHaveLeptons_fail[i]->Fill(NJetsHaveLeptons);
      	    histJet1Mass_fail[i]->Fill(fatJet1MassSD);
      	    histJet1Pt_fail[i]->Fill(fatJet1Pt);
      	    histJet1DDB_fail[i]->Fill(fatJet1DDBTagger);
      	    histJet1PNetXbb_fail[i]->Fill(fatJet1PNetXbb);
      	    histJet1Tau3OverTau2_fail[i]->Fill(fatJet1Tau3OverTau2);
      	    histJet2Mass_fail[i]->Fill(fatJet2MassSD);
      	    histJet2Pt_fail[i]->Fill(fatJet2Pt);
      	    histJet2DDB_fail[i]->Fill(fatJet2DDBTagger);
      	    histJet2PNetXbb_fail[i]->Fill(fatJet2PNetXbb);
      	    histJet2Tau3OverTau2_fail[i]->Fill(fatJet2Tau3OverTau2);
      	    histHHPt_fail[i]->Fill(hh_pt);
      	    histHHMass_fail[i]->Fill(hh_mass);
          }
        }
        else
        {
          if (processLabels[i] == "HH")
          {
            if ( (fatJet2MassSD > 95 && fatJet2MassSD < 135))
            {
              SignalYield_fail += myWeight;
            }
    	    }
          else
          {
    	      //MCYield += myWeight;//nothing done here
    	    }
    	    histMET_fail[i]->Fill(MET, myWeight);
    	    histNLeptons_fail[i]->Fill(0.0, myWeight);
    	    //histNJetsHaveLeptons_fail[i]->Fill(NJetsHaveLeptons, myWeight);
    	    histJet1Mass_fail[i]->Fill(fatJet1MassSD, myWeight);
    	    histJet1Pt_fail[i]->Fill(fatJet1Pt, myWeight);
    	    histJet1DDB_fail[i]->Fill(fatJet1DDBTagger, myWeight);
    	    histJet1PNetXbb_fail[i]->Fill(fatJet1PNetXbb, myWeight);
    	    histJet1Tau3OverTau2_fail[i]->Fill(fatJet1Tau3OverTau2, myWeight);
    	    histJet2Mass_fail[i]->Fill(fatJet2MassSD, myWeight);
    	    histJet2Pt_fail[i]->Fill(fatJet2Pt, myWeight);
    	    histJet2DDB_fail[i]->Fill(fatJet2DDBTagger, myWeight);
    	    histJet2PNetXbb_fail[i]->Fill(fatJet2PNetXbb, myWeight);
    	    histJet2Tau3OverTau2_fail[i]->Fill(fatJet2Tau3OverTau2, myWeight);
    	    histHHPt_fail[i]->Fill(hh_pt, myWeight);
    	    histHHMass_fail[i]->Fill(hh_mass, myWeight);
        }
      }
	  }//pass-fail section
	} //loop over events
      } //loop over input files
    } //loop over dataset year
  } //loop over input process type

  //--------------------------------------------------------------------------------------------------------------
  // Make Plots
  //==============================================================================================================


  //--------------------------------------------------------------------------------------------------------------
  // Draw
  //==============================================================================================================
  TCanvas *cv =0;
  TLegend *legend = 0;

  //*******************************************************************************************
  //MR
  //*******************************************************************************************
  PlotDataAndStackedBkg( histMET, processLabels, color, true, "MET", totalLumi, Label);
  PlotDataAndStackedBkg( histNLeptons, processLabels, color, true, "NLeptons", totalLumi, Label);
  //PlotDataAndStackedBkg( histNJetsHaveLeptons, processLabels, color, true, "NJetsHaveLeptons", totalLumi, Label);
  PlotDataAndStackedBkg( histJet1Mass, processLabels, color, true, "Jet1_Mass", totalLumi, Label);
  PlotDataAndStackedBkg( histJet1Pt, processLabels, color, true, "Jet1_Pt", totalLumi, Label);
  PlotDataAndStackedBkg( histJet1DDB, processLabels, color, true, "Jet1_DDB", totalLumi, Label);
  PlotDataAndStackedBkg( histJet1PNetXbb, processLabels, color, true, "Jet1_PNetXbb", totalLumi, Label);
  PlotDataAndStackedBkg( histJet1Tau3OverTau2, processLabels, color, true, "Jet1_Tau3OverTau2", totalLumi, Label);
  PlotDataAndStackedBkg( histJet2Mass, processLabels, color, true, "Jet2_Mass", totalLumi, Label);
  PlotDataAndStackedBkg( histJet2Pt, processLabels, color, true, "Jet2_Pt", totalLumi, Label);
  PlotDataAndStackedBkg( histJet2DDB, processLabels, color, true, "Jet2_DDB", totalLumi, Label);
  PlotDataAndStackedBkg( histJet2PNetXbb, processLabels, color, true, "Jet2_PNetXbb", totalLumi, Label);
  PlotDataAndStackedBkg( histJet2Tau3OverTau2, processLabels, color, true, "Jet2_Tau3OverTau2", totalLumi, Label);
  PlotDataAndStackedBkg( histHHPt, processLabels, color, true, "HHPt", totalLumi, Label);
  PlotDataAndStackedBkg( histHHMass, processLabels, color, true, "HHMass", totalLumi, Label);


  //--------------------------------------------------------------------------------------------------------------
  // Tables
  //==============================================================================================================
  std::cout << "For Luminosity = " << totalLumi << " pb^-1\n";

  std::cout << "Selected Event Yield \n";
  std::cout << "Total Sideband Data: " << dataYield << "\n";
  std::cout << "Bkg Prediction From Sideband: " << BkgYield << "\n";
  std::cout << "Signal: " << SignalYield << "\n";

  // //--------------------------------------------------------------------------------------------------------------
  // // Output
  // //==============================================================================================================
  TFile *file = TFile::Open(("HHTo4BPlots"+Label+".root").c_str(), "UPDATE");
  file->cd();

  for(int i=0; i<int(inputfiles.size()); i++) {
    file->WriteTObject(histJet1Mass[i], Form("histJet1Mass_%s",processLabels[i].c_str()), "WriteDelete");
    file->WriteTObject(histJet1Pt[i], Form("histJet1Pt_%s",processLabels[i].c_str()), "WriteDelete");
    file->WriteTObject(histJet2Mass[i], Form("histJet2Mass_%s",processLabels[i].c_str()), "WriteDelete");
    file->WriteTObject(histJet2Pt[i], Form("histJet2Pt_%s",processLabels[i].c_str()), "WriteDelete");
    file->WriteTObject(histNJetsHaveLeptons[i], Form("histNJetsHaveLeptons_%s",processLabels[i].c_str()), "WriteDelete");
    file->WriteTObject(histMET[i], Form("histMET_%s",processLabels[i].c_str()), "WriteDelete");
    //fail histograms
    file->WriteTObject(histJet2Mass_fail[i], Form("histJet2Mass_%s_fail",processLabels[i].c_str()), "WriteDelete");
  }

  file->Close();
  delete file;

}







int main( int argc, char** argv) {

  int option = -1;
  std::vector<std::pair<std::vector<std::string>,std::string > > datafiles;
  std::vector<std::vector<std::pair<std::vector<std::string>,std::string > > > bkgfiles;
  std::vector<std::string> processLabels;
  std::vector<int> colors;
  std::vector<float> scaleFactors;

  std::vector<std::string> datafiles_2016;
  std::vector<std::string> datafiles_2017;
  std::vector<std::string> datafiles_2018;

  std::vector<std::string> bkgfiles_ttbar_2016;
  std::vector<std::string> bkgfiles_H_2016;
  std::vector<std::string> bkgfiles_VH_2016;
  std::vector<std::string> bkgfiles_ttH_2016;
  std::vector<std::string> bkgfiles_qcd_2016;
  std::vector<std::string> bkgfiles_HH_2016;
  std::vector<std::string> bkgfiles_ttbar_2017;
  std::vector<std::string> bkgfiles_H_2017;
  std::vector<std::string> bkgfiles_VH_2017;
  std::vector<std::string> bkgfiles_ttH_2017;
  std::vector<std::string> bkgfiles_qcd_2017;
  std::vector<std::string> bkgfiles_HH_2017;
  std::vector<std::string> bkgfiles_ttbar_2018;
  std::vector<std::string> bkgfiles_H_2018;
  std::vector<std::string> bkgfiles_VH_2018;
  std::vector<std::string> bkgfiles_ttH_2018;
  std::vector<std::string> bkgfiles_qcd_2018;
  std::vector<std::string> bkgfiles_HH_2018;

  //***********************************
  //2016 Data and MC
  //***********************************
  std::string file_dir_2016 = "/Users/cmorgoth/Work/data/HH/v7/2016/";
  datafiles_2016.push_back(file_dir_2016+"JetHT_2016_Mass30Skim_GoodLumi_BDTs.root");

  bkgfiles_ttbar_2016.push_back(file_dir_2016+"TTToHadronic_TuneCP5_PSweights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_ttbar_2016.push_back(file_dir_2016+"TTToSemiLeptonic_TuneCP5_PSweights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_H_2016.push_back(file_dir_2016+"GluGluHToBB_M-125_13TeV_powheg_MINLO_NNLOPS_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_H_2016.push_back(file_dir_2016+"VBFHToBB_M-125_13TeV_powheg_pythia8_weightfix-combined_1pb_weighted_Mass30Skim_BDTs.root");
  //bkgfiles_VH_2016.push_back(file_dir_2016+"WminusH_HToBB_WToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  //bkgfiles_VH_2016.push_back(file_dir_2016+"WplusH_HToBB_WToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_VH_2016.push_back(file_dir_2016+"ZH_HToBB_ZToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  //  bkgfiles_VH_2016.push_back(file_dir_2016+"ggZH_HToBB_ZToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_ttH_2016.push_back(file_dir_2016+"ttHTobb_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");

  bkgfiles_qcd_2016.push_back(file_dir_2016+"QCD_HT300to500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8-combined_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2016.push_back(file_dir_2016+"QCD_HT500to700_TuneCUETP8M1_13TeV-madgraphMLM-pythia8-combined_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2016.push_back(file_dir_2016+"QCD_HT700to1000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8-combined_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2016.push_back(file_dir_2016+"QCD_HT1000to1500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8-combined_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2016.push_back(file_dir_2016+"QCD_HT1500to2000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8-combined_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2016.push_back(file_dir_2016+"QCD_HT2000toInf_TuneCUETP8M1_13TeV-madgraphMLM-pythia8-combined_1pb_weighted_Mass30Skim_Testing_BDTs.root");

  bkgfiles_HH_2016.push_back(file_dir_2016+"GluGluToHHTo4B_node_cHHH1_TuneCUETP8M1_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  //bkgfiles_HH_2016.push_back(file_dir_2016+"GluGluToHHTo4B_node_cHHH0_TuneCUETP8M1_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  //bkgfiles_HH_2016.push_back(file_dir_2016+"GluGluToHHTo4B_node_cHHH2p45_TuneCUETP8M1_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  //bkgfiles_HH_2016.push_back(file_dir_2016+"GluGluToHHTo4B_node_cHHH5_TuneCUETP8M1_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");



  //***********************************
  //2017 Data and MC
  //***********************************
  std::string file_dir_2017 = "/Users/cmorgoth/Work/data/HH/v7/2017/";
  datafiles_2017.push_back(file_dir_2017+"JetHT_2017_Mass30Skim_GoodLumi_BDTs.root");

  bkgfiles_ttbar_2017.push_back(file_dir_2017+"TTToHadronic_TuneCP5_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_ttbar_2017.push_back(file_dir_2017+"TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_H_2017.push_back(file_dir_2017+"GluGluHToBB_M-125_13TeV_powheg_MINLO_NNLOPS_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_H_2017.push_back(file_dir_2017+"VBFHToBB_M-125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_VH_2017.push_back(file_dir_2017+"WminusH_HToBB_WToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_VH_2017.push_back(file_dir_2017+"WplusH_HToBB_WToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_VH_2017.push_back(file_dir_2017+"ZH_HToBB_ZToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  //  bkgfiles_VH_2017.push_back(file_dir_2017+"ggZH_HToBB_ZToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_ttH_2017.push_back(file_dir_2017+"ttHTobb_M125_TuneCP5_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_BDTs.root");

  bkgfiles_qcd_2017.push_back(file_dir_2017+"QCD_HT300to500_TuneCP5_13TeV-madgraph-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2017.push_back(file_dir_2017+"QCD_HT500to700_TuneCP5_13TeV-madgraph-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2017.push_back(file_dir_2017+"QCD_HT700to1000_TuneCP5_13TeV-madgraph-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2017.push_back(file_dir_2017+"QCD_HT1000to1500_TuneCP5_13TeV-madgraph-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2017.push_back(file_dir_2017+"QCD_HT1500to2000_TuneCP5_13TeV-madgraph-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2017.push_back(file_dir_2017+"QCD_HT2000toInf_TuneCP5_13TeV-madgraph-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");

  bkgfiles_HH_2017.push_back(file_dir_2017+"GluGluToHHTo4B_node_cHHH1_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  // bkgfiles_HH_2017.push_back(file_dir_2017+"GluGluToHHTo4B_node_cHHH0_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  // bkgfiles_HH_2017.push_back(file_dir_2017+"GluGluToHHTo4B_node_cHHH2p45_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  // bkgfiles_HH_2017.push_back(file_dir_2017+"GluGluToHHTo4B_node_cHHH5_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");

  //***********************************
  //2018 Data and MC
  //***********************************
  std::string file_dir_2018 = "/Users/cmorgoth/Work/data/HH/v7/2018/";
  datafiles_2018.push_back(file_dir_2018+"JetHT_2018_Mass30Skim_GoodLumi_BDTs.root");

  bkgfiles_ttbar_2018.push_back(file_dir_2018+"TTToHadronic_TuneCP5_13TeV-powheg-pythia8-combined_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_ttbar_2018.push_back(file_dir_2018+"TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8-combined_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_H_2018.push_back(file_dir_2018+"GluGluHToBB_M-125_13TeV_powheg_MINLO_NNLOPS_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_H_2018.push_back(file_dir_2018+"VBFHToBB_M-125_13TeV_powheg_pythia8_weightfix_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_VH_2018.push_back(file_dir_2018+"WminusH_HToBB_WToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_VH_2018.push_back(file_dir_2018+"WplusH_HToBB_WToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_VH_2018.push_back(file_dir_2018+"ZH_HToBB_ZToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  //  bkgfiles_VH_2018.push_back(file_dir_2018+"ggZH_HToBB_ZToQQ_M125_13TeV_powheg_pythia8_1pb_weighted_Mass30Skim_BDTs.root");
  bkgfiles_ttH_2018.push_back(file_dir_2018+"ttHTobb_M125_TuneCP5_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_BDTs.root");

  bkgfiles_qcd_2018.push_back(file_dir_2018+"QCD_HT300to500_TuneCP5_13TeV-madgraphMLM-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2018.push_back(file_dir_2018+"QCD_HT500to700_TuneCP5_13TeV-madgraphMLM-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2018.push_back(file_dir_2018+"QCD_HT700to1000_TuneCP5_13TeV-madgraphMLM-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2018.push_back(file_dir_2018+"QCD_HT1000to1500_TuneCP5_13TeV-madgraphMLM-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2018.push_back(file_dir_2018+"QCD_HT1500to2000_TuneCP5_13TeV-madgraphMLM-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  bkgfiles_qcd_2018.push_back(file_dir_2018+"QCD_HT2000toInf_TuneCP5_13TeV-madgraphMLM-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");

  bkgfiles_HH_2018.push_back(file_dir_2018+"GluGluToHHTo4B_node_cHHH1_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  // bkgfiles_HH_2018.push_back(file_dir_2018+"GluGluToHHTo4B_node_cHHH0_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  // bkgfiles_HH_2018.push_back(file_dir_2018+"GluGluToHHTo4B_node_cHHH2p45_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");
  // bkgfiles_HH_2018.push_back(file_dir_2018+"GluGluToHHTo4B_node_cHHH5_TuneCP5_PSWeights_13TeV-powheg-pythia8_1pb_weighted_Mass30Skim_Testing_BDTs.root");


  //another std::vector to contain the different dataset years
  std::vector<std::pair<std::vector<std::string>,std::string > > bkgfiles_ttbar;
  std::vector<std::pair<std::vector<std::string>,std::string > > bkgfiles_H;
  std::vector<std::pair<std::vector<std::string>,std::string > > bkgfiles_VH;
  std::vector<std::pair<std::vector<std::string>,std::string > > bkgfiles_ttH;
  std::vector<std::pair<std::vector<std::string>,std::string > > bkgfiles_qcd;
  std::vector<std::pair<std::vector<std::string>,std::string > > bkgfiles_HH;

  if (option == -1) {
    datafiles.push_back(std::pair<std::vector<std::string> , std::string> ( datafiles_2016, "2016"));
    datafiles.push_back(std::pair<std::vector<std::string> , std::string> ( datafiles_2017, "2017"));
    datafiles.push_back(std::pair<std::vector<std::string> , std::string> ( datafiles_2018, "2018"));

    bkgfiles_qcd.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_qcd_2016 , "2016"));
    bkgfiles_ttbar.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttbar_2016 , "2016"));
    bkgfiles_VH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_VH_2016 , "2016"));
    bkgfiles_H.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_H_2016 , "2016"));
    bkgfiles_ttH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttH_2016 , "2016"));
    bkgfiles_HH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_HH_2016 , "2016"));

    bkgfiles_qcd.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_qcd_2017 , "2017"));
    bkgfiles_ttbar.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttbar_2017 , "2017"));
    bkgfiles_VH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_VH_2017 , "2017"));
    bkgfiles_H.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_H_2017 , "2017"));
    bkgfiles_ttH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttH_2017 , "2017"));
    bkgfiles_HH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_HH_2017 , "2017"));

    bkgfiles_qcd.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_qcd_2018 , "2018"));
    bkgfiles_ttbar.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttbar_2018 , "2018"));
    bkgfiles_VH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_VH_2018 , "2018"));
    bkgfiles_H.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_H_2018 , "2018"));
    bkgfiles_ttH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttH_2018 , "2018"));
    bkgfiles_HH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_HH_2018 , "2018"));
  }
  if (option == 0) {
    datafiles.push_back(std::pair<std::vector<std::string> , std::string> ( datafiles_2016, "2016"));
    bkgfiles_qcd.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_qcd_2016 , "2016"));
    bkgfiles_ttbar.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttbar_2016 , "2016"));
    bkgfiles_VH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_VH_2016 , "2016"));
    bkgfiles_H.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_H_2016 , "2016"));
    bkgfiles_ttH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttH_2016 , "2016"));
    bkgfiles_HH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_HH_2016 , "2016"));
  }
  if (option == 1) {
    datafiles.push_back(std::pair<std::vector<std::string> , std::string> ( datafiles_2017, "2017"));
    bkgfiles_qcd.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_qcd_2017 , "2017"));
    bkgfiles_ttbar.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttbar_2017 , "2017"));
    bkgfiles_VH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_VH_2017 , "2017"));
    bkgfiles_H.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_H_2017 , "2017"));
    bkgfiles_ttH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttH_2017 , "2017"));
    bkgfiles_HH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_HH_2017 , "2017"));
  }
  if (option == 2) {
    datafiles.push_back(std::pair<std::vector<std::string> , std::string> ( datafiles_2018, "2018"));
    bkgfiles_qcd.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_qcd_2018 , "2018"));
    bkgfiles_ttbar.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttbar_2018 , "2018"));
    bkgfiles_VH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_VH_2018 , "2018"));
    bkgfiles_H.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_H_2018 , "2018"));
    bkgfiles_ttH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_ttH_2018 , "2018"));
    bkgfiles_HH.push_back(std::pair<std::vector<std::string> , std::string>( bkgfiles_HH_2018 , "2018"));
  }

  bkgfiles.push_back(bkgfiles_qcd);
  bkgfiles.push_back(bkgfiles_ttbar);
  bkgfiles.push_back(bkgfiles_H);
  bkgfiles.push_back(bkgfiles_VH);
  bkgfiles.push_back(bkgfiles_ttH);
  bkgfiles.push_back(bkgfiles_HH);

  processLabels.push_back("QCD");
  processLabels.push_back("TTJets");
  processLabels.push_back("H");
  processLabels.push_back("VH");
  processLabels.push_back("ttH");
  processLabels.push_back("HH");

  colors.push_back(kRed);
  colors.push_back(kAzure+10);
  colors.push_back(kGreen+2);
  colors.push_back(kGray);
  colors.push_back(kBlue);
  colors.push_back(kMagenta+2);

  scaleFactors.push_back(0.72);
  scaleFactors.push_back(1.0);
  scaleFactors.push_back(1.0);
  scaleFactors.push_back(1.0);
  scaleFactors.push_back(1.0);
  scaleFactors.push_back(1.0);

  std::string label = "";
  double totalLumi = 0;
  if (option == -1) { label = "Run2"; totalLumi = 136143; }
  if (option == 0)  { label = "2016"; totalLumi = 35922;  }
  if (option == 1)  { label = "2017"; totalLumi = 41480;  }
  if (option == 2)  { label = "2018"; totalLumi = 59741;  }

  //*********************************************************************
  //SR Selection
  //*********************************************************************
  RunSelectHHTo4B(datafiles, bkgfiles, processLabels, colors, scaleFactors, 1, totalLumi, label);

}


//**********************************
//E-Mu Yields :
//**********************************

//MuEG Triggers ( 30 - 20 )
// Data: 148
// MC: 156.16
