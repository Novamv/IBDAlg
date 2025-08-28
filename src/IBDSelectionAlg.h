#ifndef IBDSELECTIONALG
#define IBDSELECTIONALG

#include "SniperKernel/AlgBase.h"
#include <vector>
#include <algorithm>
#include <unordered_map> 
#include <math.h>
#include "TTree.h"
#include "TFile.h"
#include "TVector3.h"
#include "TTimeStamp.h"
#include "TGraph.h"

#include "EvtNavigator/NavBuffer.h"
#include "EvtNavigator/EvtNavigator.h"
#include "EvtNavigator/EvtNavHelper.h"
#include "Event/OecHeader.h"
#include "Event/OecEvt.h"
#include "OECTagSvc/OECTagSvc.h"
#include "OECTagID/OECTagID.h"

#include "SpmtElecConfigSvc/SpmtElecConfigSvc.h"


class IBDSelectionAlg : public AlgBase
{
	private :
		
		double ComputeLTOF(double pmtid, double evtx, double evty, double evtz);
		double ComputeSTOF(double pmtid, double evtx, double evty, double evtz);


		JM::NavBuffer::Iterator PromptIt;
		JM::NavBuffer::Iterator DelayIt;
		// JM::EvtNavigator* PostDelayNav;
		bool foundDelay;
		int m_DelayEvt;
		int m_DelayIBD;
		std::string m_DelayType;

		bool isMuonVetoed(JM::NavBuffer::Iterator);
		bool isIsolated(JM::NavBuffer::Iterator);
		bool findCorrelation(JM::NavBuffer::Iterator, std::vector<JM::OecEvt*>&);
		bool FillPromptDelay(JM::NavBuffer::Iterator, JM::NavBuffer::Iterator);

    public :

        IBDSelectionAlg(const std::string& name);

        bool initialize();
        bool execute();
        bool finalize();

		bool Book_tree();
	
	private:

	    int m_iEvt;

		bool IBDSelection, saveCalib, saveBiPo;

		TGraph* gInterfaceLevel;

        JM::NavBuffer* m_buf;
		SpmtElecConfigSvc* m_spmtSvc;
		OECTagSvc* m_tagsvc;

		uint32_t i_pBiPo214;
		uint32_t i_dBiPo214;


		unsigned int TotalLPMT = 17612;
        unsigned int TotalSPMT = 25600;
		
		std::vector<TVector3> ALL_LPMT_pos;
		std::vector<TVector3> ALL_SPMT_pos;


    private :

        int m_EntryNb;

        TTree *m_ntuple; //simulation tree 
		int m_NbHitLPMTSim;
		int m_NbHitSPMTSim;
		int m_TotalPESim;
		double m_Vtx; 
		double m_Vty;
		double m_Vtz; 
		std::vector<double> m_Qedep;
		std::vector<double> m_Edep;
		std::vector<int> m_PDGID;
		std::vector<int> m_ParentID;
		std::vector<int> m_TrackID;
		std::vector<int> m_PE;
		unsigned long m_TimeStampInNanoSec; 
		std::vector<int> m_AbcChannel;
		std::vector<int> m_AbcNb;
		std::vector<int> m_PmtIdSim;
		std::vector<int> m_PmtHitTrackID;
		std::vector<double> m_HitTimeSim;
		std::vector<double> m_ChargeSim;
		
		TTree *m_ntuple1; // calibration tree
		int m_iRun;
		unsigned long m_TimeStamp;
		uint64_t m_TriggerTime;
		std::vector<std::string> m_TriggerType;
		double m_ChargeTotLPMT;
		int m_NbHitLPMTCalib;
		int m_NbHitSPMTCalib;
		std::vector<int> m_PmtIdCalib;
		// std::vector<int> m_PMTCircle;
		// std::vector<std::string> m_PMTType;
		std::vector<double> m_HitTimeCalib;
		std::vector<double> m_ChargeCalib;
		
		TTree *m_ntuple4; // reco tree
		double m_TotalPE;
		int m_NFiredPMT;
		double m_RecE;
		double m_RecX;
		double m_RecY;
		double m_RecZ;
		double m_T0;
		std::vector<double> m_HitTimeTOF;

		TTree *m_ntuple5; 
		double m_WpNPE;
		std::vector<int> m_WpPmtId;
		std::vector<double> m_WpCharge; 
		std::vector<double> m_WpHitTime; 

		TTree* m_ntuple6;
		double m_OecTotCharge;
		double m_OecEnergy;
		double m_OecX;
		double m_OecY;
		double m_OecZ;
		TString m_EventTag;


		TTree* m_ntuple7;
		int m_PromptEvt;
		TString m_File;
		unsigned long m_PromptTimeStamp;
		double m_PromptCharge;
		double m_PromptX;
		double m_PromptY;
		double m_PromptZ;
		// std::vector<float> m_PromptPmtId;
		std::vector<double> m_PromptHitTimeTOF;
		// std::vector<float> m_PromptHitCharge;

		TTree* m_ntuple8;		
		double m_DelayCharge;
		unsigned long m_DelayTimeStamp;
		double m_DelayX;
		double m_DelayY;
		double m_DelayZ;
		double m_TimeDifference;
		int m_DelayEntry;
		int m_CorPrompt;
		std::vector<double> m_DelayHitTimeTOF;



		double PMT_R;
		double LS_R;

		double RfrIndxLS;
		double RfrIndxWR;
		double c;


};

#endif
