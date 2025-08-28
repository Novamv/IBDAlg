#include "SNiPERToPlainTree.h"
#include "TOF.h"

#include "EvtNavigator/NavBuffer.h"
#include "EvtNavigator/EvtNavHelper.h"
#include "SniperKernel/AlgFactory.h"
#include "SniperKernel/SniperDataPtr.h"
#include "SniperKernel/SniperPtr.h"
#include "Event/CdVertexRecHeader.h"
// Calib Evt
#include "Event/CdLpmtCalibHeader.h"
#include "Event/CdSpmtCalibHeader.h"
#include "Event/WpCalibHeader.h"

#include "Event/CdLpmtElecTruthHeader.h"
#include "Event/CdSpmtElecTruthHeader.h"
#include "Event/CdWaveformHeader.h"
#include "Event/SimHeader.h"
#include "Event/CdLpmtElecEvt.h"
#include "Event/CdSpmtElecEvt.h"
#include "Event/CdLpmtElecHeader.h"
#include "Event/CdSpmtElecHeader.h"
#include "Event/CdTriggerHeader.h"
#include "Event/CdTriggerEvt.h"
#include "Event/CdVertexRecHeader.h"
#include "Event/OecHeader.h"
#include "Event/OecEvt.h"
#include "OECTagSvc/OECTagSvc.h"
#include "OECTagID/OECTagID.h"

#include "Identifier/Identifier.h"
#include "Identifier/CdID.h"
#include "Identifier/WpID.h"
#include "RootWriter/RootWriter.h"
#include "TTree.h"
#include "BufferMemMgr/IDataMemMgr.h"
#include "SpmtElecConfigSvc/SpmtElecConfigSvc.h"
#include "Geometry/IPMTParamSvc.h"

DECLARE_ALGORITHM(SNiPERToPlainTree);

<<<<<<< HEAD
IBDSelectionAlg::IBDSelectionAlg(const std::string& name)
=======
SNiPERToPlainTree::SNiPERToPlainTree(const std::string& name)
>>>>>>> parent of 2c56e88 (Switched SNiPERToPlainTree to IBDSelectionAlg)
: AlgBase(name),
	m_iEvt(-1),
	m_buf(0),
	m_spmtSvc(0),
	m_tagsvc(0)
{
	declProp("enableIBDSelection", IBDSelection=false);
	declProp("saveCalib", saveCalib=false);
	declProp("saveBiPo", saveBiPo=false);

	m_DelayIBD = -1;
	m_DelayEvt = -1;
	foundDelay = false;
}

bool SNiPERToPlainTree::initialize()
{
	//----------------------------------------------------------------------------
	const std::string compilation_date = __DATE__;
	const std::string compilation_time = __TIME__;
	std::cout <<"##################################################################"<<std::endl
	<<"The source file was compiled on " << compilation_date<< " at " << compilation_time <<std::endl
	<<"##################################################################"<<std::endl;
	//----------------------------------------------------------------------------
	
    // =======================================================================
    // Loading PMT positions
    // =======================================================================
	
	SniperPtr<IPMTParamSvc> pmtsvc(getParent(), "PMTParamSvc");
	if (ALL_LPMT_pos.size()==0 && pmtsvc.valid()) {
		TotalLPMT = pmtsvc->get_NTotal_CD_LPMT();
		
		std::cout << " PMT Information " << std::endl;
		
		for (unsigned int ith = 0; ith < TotalLPMT; ith++)
		{
			TVector3 all_pmtCenter(pmtsvc->getPMTX(ith), pmtsvc->getPMTY(ith), pmtsvc->getPMTZ(ith));
			ALL_LPMT_pos.push_back(all_pmtCenter);
		}
	}
	if (ALL_SPMT_pos.size()==0 && pmtsvc.valid()) {
		TotalSPMT = pmtsvc->get_NTotal_CD_SPMT();
		
		for (unsigned int ith = 0; ith < TotalSPMT; ith++)
		{
			TVector3 all_pmtCenter(pmtsvc->getPMTX(ith+20000), pmtsvc->getPMTY(ith+20000), pmtsvc->getPMTZ(ith+20000));
			ALL_SPMT_pos.push_back(all_pmtCenter);
		}
	}
	
	// =======================================================================
    // GET INTERFACE LEVEL
    // =======================================================================
	
	TFile* f = TFile::Open("/sps/juno/mlecocq/Commissioning/massVolumeLS.root", "READ");
	if (!f || f->IsZombie()) {
		std::cerr << "Error opening file!" << std::endl;
		return false;
    }

	gInterfaceLevel = (TGraph*)f->Get("inter_level_s1_graph");
	if (!gInterfaceLevel) {
        std::cerr << "TGraph not found in file!" << std::endl;
        f->Close();
        return false;
    }

	f->Close();
	
	
	// =======================================================================
    // GET EVENT
    // =======================================================================
	
	
	LogDebug << "initializing" << std::endl;
	std::cout<<"36"<<std::endl;
	gDirectory->pwd();
	
	SniperDataPtr<JM::NavBuffer> navBuf(getRoot(),"/Event");
	if ( navBuf.invalid() ) {
		LogError << "cannot get the NavBuffer @ /Event" << std::endl;
		return false;
	}
	m_buf = navBuf.data();
	// std::cout << "Buffer size: " << navBuf->size() <<std::endl;
	
	
	std::cout<<"46"<<std::endl;
	gDirectory->pwd();
	
	SniperPtr<SpmtElecConfigSvc> svc(*getRoot(), "SpmtElecConfigSvc");
	if (svc.invalid()) {
		LogError << "can't find service SpmtElecConfigSvc" << std::endl;
		return false;
	}
	m_spmtSvc = svc.data();

	std::cout<<"47"<<std::endl;

	SniperPtr<OECTagSvc> tagsvc(getParent(), "OECTagSvc");
	if(tagsvc.invalid()){
		LogError << "Unable to locate tagsvc" << std::endl;
		return false;
	}
	std::cout<<"48"<<std::endl;
	m_tagsvc = tagsvc.data();
	std::cout<<"49"<<std::endl;
	
	i_pBiPo214 = m_tagsvc->getpTag("BiPo214Pair");
	i_dBiPo214 = m_tagsvc->getdTag("BiPo214Pair");

	std::cout<<"50"<<std::endl;


	Book_tree();
	
	return true;
}

bool SNiPERToPlainTree::execute()
{
	std::cout << "Buffer size: " << m_buf->size() <<std::endl;

	std::cout << "executing: " << ++m_iEvt
		<< std::endl;
	std::cout<<"54"<<std::endl;
	gDirectory->pwd();

	m_File = gDirectory->GetPath();

	JM::EvtNavigator* nav = 0;
	JM::SimEvt* simevent = 0;
	JM::SimEvt* simevent_orig = 0;
	JM::CdVertexRecEvt* recevent = 0;
	JM::CdLpmtCalibEvt* calibeventLPMT = 0;
	JM::CdSpmtCalibEvt* calibeventSPMT = 0;
	JM::WpCalibEvt* wpcalibevt = 0;
	JM::CdWaveformEvt* elecevent = 0;
	JM::CdLpmtElecTruthEvt *trutheventLPMT = 0;
	JM::CdSpmtElecTruthEvt *trutheventSPMT = 0;
	JM::CdLpmtElecEvt *eventLPMT = 0;
	JM::CdSpmtElecEvt *eventSPMT = 0;
	JM::CdTriggerEvt *triggerevent = 0;
	JM::OecEvt *oecevt = 0;

	nav = m_buf->curEvt();

	std::cout << "IBD Selection: " << IBDSelection << std::endl;
	std::cout << "Delay Entry: " << m_DelayEvt << std::endl;

	std::cout<<"71"<<std::endl;
	gDirectory->pwd();
	const auto& paths = nav->getPath();
	const auto& refs = nav->getRef();

	LogInfo << "Detector type is  " << nav->getDetectorType()<<std::endl;
	LogInfo << "Start to Explore SmartRef: " << std::endl;
	LogInfo << "Size of paths: " << paths.size() << std::endl;
	LogInfo << "Size of refs: " << refs.size() << std::endl;

	for (size_t i = 0; i < paths.size(); ++i) {
		LogInfo << refs[i]<<" -> ref: " << std::endl;
		const std::string& path = paths[i];
		JM::SmartRef* ref = refs[i];
		JM::EventObject* evtobj = ref->GetObject();

		LogInfo << " path: " << path
			<< " ref->entry(): " << ref->entry()
			<< " evtobj: " << evtobj;

		if (path=="/Event/Sim") {
			auto hdr = dynamic_cast<JM::SimHeader*>(evtobj);
			LogInfo <<i<<" SimHeader: " << hdr;
		}
		LogInfo << std::endl;
	}

	// Event After Split
	auto simheader = JM::getHeaderObject<JM::SimHeader>(nav);
	if (simheader) {
		simevent  = (JM::SimEvt*)simheader->event();
		LogInfo << "SimEvent Read in: " << simevent << std::endl;
		LogInfo << "SimEvent Track: " << simevent->getTracksVec().size() << std::endl;
		LogInfo << "SimEvent Hits: " << simevent->getCDHitsVec().size() << std::endl;
	}
	// Event Before Split
	auto simheader_orig = JM::getHeaderObject<JM::SimHeader>(nav, "/Event/SimOrig");
	if (simheader_orig) {
		simevent_orig  = (JM::SimEvt*)simheader_orig->event();
		LogInfo << "SimEvent (Orig) Read in: " << simevent_orig << std::endl;
		LogInfo << "SimEvent (Orig) Track: " << simevent_orig->getTracksVec().size() << std::endl;
		LogInfo << "SimEvent (Orig) Hits: " << simevent_orig->getCDHitsVec().size() << std::endl;
	}

	// ===================================================================
	// TRIGGEREVT
	// ===================================================================

	auto triggerheader = JM::getHeaderObject<JM::CdTriggerHeader>(nav);
	if(triggerheader){triggerevent = triggerheader->event();
	  std::cout<<" CD TriggerEvent Read in: " << triggerevent <<std::endl;	
	}
	
	// ===================================================================
	// CALIBEVT
	// ===================================================================

	auto calibheaderLPMT = JM::getHeaderObject<JM::CdLpmtCalibHeader>(nav);
	if (calibheaderLPMT) {
		calibeventLPMT = calibheaderLPMT->event();
		LogInfo << "CalibEventLPMT Read in: " << calibeventLPMT << std::endl;
	}
	auto calibheaderSPMT = JM::getHeaderObject<JM::CdSpmtCalibHeader>(nav);
	if (calibheaderSPMT) {
		calibeventSPMT = calibheaderSPMT->event();
		LogInfo << "CalibEventSPMT Read in: " << calibeventSPMT << std::endl;
	}
	auto wpcalibhdr = JM::getHeaderObject<JM::WpCalibHeader>(nav);
        if (wpcalibhdr) {
            wpcalibevt = dynamic_cast<JM::WpCalibEvt*>(wpcalibhdr->event());
    }

	// ===================================================================
	// RECEVT
	// ===================================================================

	auto recheader = JM::getHeaderObject<JM::CdVertexRecHeader>(nav);
	if(recheader){recevent = recheader->event();
		LogInfo << "VertexRecEvent Read in: " << recevent << std::endl;
	}

	// ===================================================================
	// OECEVT
	// ===================================================================

	auto oecheader = JM::getHeaderObject<JM::OecHeader>(nav);
	if(oecheader){
		oecevt = dynamic_cast<JM::OecEvt*>(oecheader->event("JM::OecEvt"));
		LogInfo << "OecEvent Read in: " << oecevt << std::endl;
	}


	m_EntryNb=0;
	m_NbHitLPMTSim=0;
	m_NbHitSPMTSim=0;
	m_Vtx=0;
	m_Vty=0;
	m_Vtz=0;
	m_TimeStampInNanoSec=0;

	m_TriggerType.clear();

	m_Qedep.clear();
	m_Edep.clear();
	m_PDGID.clear();
	m_ParentID.clear();
	m_TrackID.clear();
	m_PE.clear();
	m_PmtIdSim.clear();
	m_PmtHitTrackID.clear();
	m_AbcChannel.clear();
	m_AbcNb.clear();
	m_HitTimeSim.clear();
	m_ChargeSim.clear();

	m_ChargeTotLPMT=0.;
	m_NbHitLPMTCalib=0;
	m_NbHitSPMTCalib=0;
	m_PmtIdCalib.clear();
	// m_PMTCircle.clear();
	// m_PMTType.clear();
	m_HitTimeCalib.clear();
	m_ChargeCalib.clear();

	m_WpNPE = 0.0;
	m_WpCharge.clear();
	m_WpHitTime.clear();

	m_TotalPE=0.0;
	m_NFiredPMT=0;
	m_RecE=0.0;
	m_RecX=0.0;
	m_RecY=0.0;
	m_RecZ=0.0;
	m_T0=0.0;
	m_HitTimeTOF.clear();

	PMT_R = 35.4; //m
	LS_R = 17.7; //m
	RfrIndxLS = 1.5;
    RfrIndxWR = 1.355;

	c = 299792458.0; //m/s


	if(triggerevent){

		const auto& type = triggerevent->triggerType();
		const auto& pmtFired = triggerevent->nHitMultiplicity();
		// const auto& volID = triggerevent->volumeId();
		const auto& trigTime = triggerevent->triggerTime();

		m_TriggerTime = trigTime.GetSec()*1000000000ULL + trigTime.GetNanoSec();

		std::cout<< "Trigger type size " <<type.size()<<std::endl;
		std::cout<< "Triggered PMT size " <<pmtFired<<std::endl;
		// std::cout<< "volume size " <<volID.size()<<std::endl;
		std::cout<< "Trigger Time size " <<trigTime.GetNanoSec()<<std::endl;

		for(auto it = 0; it<type.size(); it++){
			std::cout<<"Trigger type = "<<type[it]<<std::endl;
			m_TriggerType.push_back(type[it]);

		}
	}

	const auto& timestamp = nav->TimeStamp();
	int RunNumber = nav->RunID();
	int EventNumber = nav->EventID();

	m_iRun = RunNumber;
	m_TimeStamp = timestamp.GetSec()*1000000000ULL + timestamp.GetNanoSec();

	if(simevent)
	{
		m_TimeStampInNanoSec = timestamp.GetSec()*1000000000ULL + timestamp.GetNanoSec() ;
		const auto& tracks = simevent->getTracksVec();
		//		std::cout<<"tracks::"<<tracks.size()<<std::endl;
		const auto& pmthit = simevent->getCDHitsVec();
		//		std::cout<<"pmthit::"<<pmthit.size()<<std::endl;
		for(auto it = tracks.begin(); it != tracks.end(); ++it){
			int pdg = (*it)->getPDGID();
			m_PDGID.push_back(pdg);
			m_ParentID.push_back((*it)->getParentID());
			m_TrackID.push_back((*it)->getTrackID());
			m_Qedep.push_back((*it)->getQEdep());
			m_Edep.push_back((*it)->getEdep());

			if(it == tracks.begin() )
			{
				m_Vtx = (*it)->getInitX();
				m_Vty = (*it)->getInitY();
				m_Vtz = (*it)->getInitZ();
				std::cout<<"Initial track "<<" pdg "<<pdg<<" Vtx "<<m_Vtx<<" Vty "<<m_Vty<<" Vtz "<<m_Vtz<< " edep "<< (*it)->getEdep()<<std::endl;
			}
		}

		std::vector<double> tempHitTime;
		int tempPE[m_TrackID.size()] = {0,0,0};

		m_TotalPESim = 0;
		std::cout << "SIM NPE AND HIT TIME" << std::endl;
		for (auto it = pmthit.begin(); it < pmthit.end(); it++)
		{
			int trkid = (*it)->getTrackID();
			auto iit = std::find(m_TrackID.begin(), m_TrackID.end(), trkid);
			if(iit != m_TrackID.end()){
				int index = std::distance(m_TrackID.begin(), iit);
				tempPE[index] += (*it)->getNPE();
			}

			m_TotalPESim += (*it)->getNPE();

			tempHitTime.push_back((*it)->getHitTime()+m_TimeStampInNanoSec);
			m_PmtIdSim.push_back((*it)->getPMTID());
			m_PmtHitTrackID.push_back((*it)->getPMTID());
			m_HitTimeSim.push_back((*it)->getHitTime());
			m_ChargeSim.push_back((*it)->getNPE());

			if((*it)->getPMTID()<18000){m_NbHitLPMTSim++;}
			else if((*it)->getPMTID()>18000){m_NbHitSPMTSim++;}
		}

		m_PE.resize(m_TrackID.size());
		for (size_t i = 0; i < m_TrackID.size(); i++){
			m_PE[i] = tempPE[i];
		}

		std::cout << "END OF SIMEVENT" << std::endl;
	}

	double charge_pmt;

	if(calibeventLPMT)
	{
		std::cout << "CALIBEVENT LPMT" << std::endl;
		std::vector<int> tempPmtIds;
		std::vector<double> tempHitTimes;
		std::vector<double> tempCharges;

		const auto& chhlistLPMT = calibheaderLPMT->event()->calibPMTCol();
				// std::cout<<"NLPMT Hitted "<<chhlistLPMT.size()<<std::endl;
		double ChargeTot = 0.0;
		int NLPMT=0;
		for (auto chit = chhlistLPMT.begin(); chit!=chhlistLPMT.end(); ++chit){
			NLPMT++;
			charge_pmt = 0.0;

			auto calib = *chit;

			unsigned int pmtId = calib->pmtId();
			Identifier id = Identifier(pmtId);
			int TruePM=CdID::module(id);
			// std::cout << " - PMTID: " << TruePM << std::endl;

			m_NbHitLPMTCalib+=calib->size();
			for(unsigned int j=0;j<calib->size();j++)
			{
				m_PmtIdCalib.push_back(TruePM);
				m_HitTimeCalib.push_back(calib->time(j));
				m_ChargeCalib.push_back(calib->charge(j));

				ChargeTot+=calib->charge(j);
				charge_pmt+=calib->charge(j);
			}

		}

		m_ChargeTotLPMT = ChargeTot;
	}
	if(calibeventSPMT)
	{
		std::cout << "CALIBEVENT SPMT" << std::endl;

		const auto& chhlistSPMT = calibheaderSPMT->event()->calibPMTCol();
		for (auto cchit = chhlistSPMT.begin(); cchit!=chhlistSPMT.end(); ++cchit) {
			auto calibSPMT = *cchit;

			unsigned int pmtId = calibSPMT->pmtId();
			Identifier id = Identifier(pmtId);
			int TruePM=CdID::module(id);
			TruePM+=20000-17612;

			m_NbHitSPMTCalib+=calibSPMT->size();
			for(unsigned int j=0;j<calibSPMT->size();j++)
			{
				m_PmtIdCalib.push_back(TruePM);
				m_HitTimeCalib.push_back(calibSPMT->time(j));
				m_ChargeCalib.push_back(calibSPMT->charge(j));
			}
		}
	}
	if(wpcalibevt){
		std::cout << "WP CALIBEVENT" << std::endl;

		const auto& channels = wpcalibevt->calibPMTCol();
		for(auto channel : channels){
			unsigned int pmtid = channel->pmtId();
			Identifier id = Identifier(pmtid);
			int pmt = WpID::module(id);

			m_WpNPE += channel->nPE();

			for (unsigned int i = 0; i < channel->size(); i++){
				m_WpPmtId.push_back(pmt);
				m_WpCharge.push_back(channel->charge(i));
				m_WpHitTime.push_back(channel->time(i));
			}

		}
	}

	// if (recevent)
	// {
	// 	std::cout << "ADDING REC INFO TO PLAIN" << std::endl;
	// 	const auto& recvertices = recevent->vertices();
	// 	LogInfo << " CdVertexRecEvt: " << std::endl;
    //     LogInfo << " - number of vertices: " << recevent->nVertices() << std::endl;

	// 	for(auto vertex: recvertices)
	// 	{
	// 		m_TotalPE = vertex->peSum();
	// 		m_NFiredPMT = vertex->nfiredpmts();
	// 		m_RecE = vertex->energy();
	// 		m_RecX = vertex->x();
	// 		m_RecY = vertex->y();
	// 		m_RecZ = vertex->z();
	// 		m_T0 = vertex->t0();

	// 		// double Vtx[3] = {m_RecX, m_RecY, m_RecZ};
	// 		for (size_t i = 0; i < m_HitTimeCalib.size(); i++)
	// 		{
	// 			double timeTOF2 = 0.0;
	// 			int pmtid = m_PmtIdCalib.at(i);
	// 			if(pmtid <= 18000){
	// 				timeTOF2 = m_HitTimeCalib.at(i) - ComputeLTOF(pmtid, m_RecX, m_RecY, m_RecZ);
	// 			}
	// 			else if(pmtid > 18000 && pmtid <= 45600){
	// 				timeTOF2 = m_HitTimeCalib.at(i) - ComputeSTOF(pmtid, m_RecX, m_RecY, m_RecZ);
	// 			}
	// 			m_HitTimeTOF.push_back(timeTOF2);
	// 		}

	// 	}
	// }



	// if((oecevt && calibeventLPMT) && m_iEvt == m_DelayEvt){
		
	// 	for (size_t i = 0; i < m_HitTimeCalib.size(); i++){
	// 		double timeTOF2 = 0.0;
	// 		int pmtid = m_PmtIdCalib.at(i);
	// 		if(pmtid <= 18000){
	// 			double interface = (gInterfaceLevel->Eval(m_TimeStamp*1e-9) - 17.7) * 1e3; //convert to mm and 0 is set to the center of the sphere
	// 			TVector3 vertex(m_DelayX, m_DelayY, m_DelayZ);
	// 			TOFCalculator TOF(vertex, ALL_LPMT_pos.at(pmtid), interface);
	// 			timeTOF2 = m_HitTimeCalib.at(i) - ComputeLTOF(pmtid, m_DelayX, m_DelayY, m_DelayZ);
	// 		}
	// 		m_HitTimeTOF.push_back(timeTOF2);
	// 	}
	// 	m_EventTag.push_back(m_DelayType);
	// 	m_ntuple8->Fill();
	// }




	if(oecevt){
		std::cout << "ADDING OEC EVENT" << std::endl;

		m_OecTotCharge = oecevt->getTotalCharge();
		std::cout << "OecCharge " << m_OecTotCharge << std::endl;
		m_OecX = oecevt->getVertexX();
		m_OecY = oecevt->getVertexY();
		m_OecZ = oecevt->getVertexZ();
		m_OecEnergy = oecevt->getEnergy();
	}



	if(saveBiPo && oecevt && calibeventLPMT && (m_TriggerType[0] == "nHit" || m_TriggerType[0] == "Multiplicity")){

		uint32_t tag = oecevt->getTag();
		if((tag & i_pBiPo214) == i_pBiPo214){
			// m_CorPrompt = m_iEvt;

			bool foundBiPoDelay = false;

			int offset = 0;
			for(JM::NavBuffer::Iterator tmpit = (m_buf->current() + 1); tmpit != m_buf->end(); ++tmpit){
				
				++offset;

				JM::OecHeader* aHeaderOEC = JM::getHeaderObject<JM::OecHeader>(tmpit->get());
				JM::OecEvt* aEventOEC = dynamic_cast<JM::OecEvt*>(aHeaderOEC->event("JM::OecEvt"));

				if((aEventOEC->getTag() & i_dBiPo214) == i_dBiPo214){
					const TTimeStamp& atime = aEventOEC->getTime();

					DelayIt = tmpit;
					m_DelayEvt = m_iEvt + offset;
					m_DelayTimeStamp = atime;
					m_DelayCharge = aEventOEC->getTotalCharge();
					m_DelayX = aEventOEC->getVertexX();
					m_DelayY = aEventOEC->getVertexY();
					m_DelayZ = aEventOEC->getVertexZ();
					m_TimeDifference = (atime.GetSec() - timestamp.GetSec())*1000000000ULL + (atime.GetNanoSec() - timestamp.GetNanoSec());

					foundBiPoDelay = true;

					break;
				}
			}

			if(foundBiPoDelay){

				m_EventTag = "BiPo214";

				PromptIt = m_buf->current();
				m_PromptEvt = m_iEvt;
				m_PromptCharge = m_OecTotCharge;
				m_PromptTimeStamp = m_TimeStamp;
				m_PromptX = m_OecX;
				m_PromptY = m_OecY;
				m_PromptZ = m_OecZ;

				std::cout << "\tFilling BiPo214 Prompt/Delay" << std::endl;
				FillPromptDelay(PromptIt, DelayIt);
			}

		} 

	}

	if(IBDSelection){

		std::cout << "Event " << m_iEvt << std::endl;
		std::cout << "Delay Event " << m_DelayIBD << std::endl;
		bool EventCheck = (m_iEvt - (m_DelayIBD + 1)) == 0;
		std::cout << "Delay Event Condition " << EventCheck << std::endl;

		if(oecevt && calibeventLPMT && EventCheck && (m_TriggerType[0] == "nHit" || m_TriggerType[0] == "Multiplicity")){
			// Select Prompt

			std::cout << "Selecting Prompt " << std::endl;
			
			double r_pos = sqrt( pow(m_OecX,2) + pow(m_OecY,2) + pow(m_OecZ,2) );
			bool r_cut = r_pos < 16500 && m_OecZ < 15500;
			
			if( (m_OecTotCharge >= 2000 && m_OecTotCharge <= 20000 && m_OecEnergy != -1) && r_cut ){
				
				std::cout << "Looking for IBD Signal" << std::endl;
				
				std::vector<JM::OecEvt*> prompt_Evt;
				prompt_Evt.push_back(oecevt);
				
				// Check if Delay
				if(findCorrelation(m_buf->current(), prompt_Evt)){
					std::cout << "Found IBD with dt = " << m_TimeDifference <<std::endl;
					
					PromptIt = m_buf->current();
					m_PromptEvt = m_iEvt;
					m_PromptTimeStamp = oecevt->getTime();
					m_PromptCharge = oecevt->getTotalCharge();
					m_PromptX = oecevt->getVertexX();
					m_PromptY = oecevt->getVertexY();
					m_PromptZ = oecevt->getVertexZ();

					m_EventTag = "IBD";

					std::cout << "\tFilling Prompt/Delay Tree" << std::endl;
					FillPromptDelay(PromptIt, DelayIt);
				}
				else{
					std::cout << "No Delay Signal Found " <<std::endl;
				}
			}
		}
		else{
			std::cout << "No Prompt Signal found" << std::endl;
		}

		if(!foundDelay){
			++m_DelayIBD;
		}

	}

	// std::cout << "Fill Sim " << std::endl;
	// m_ntuple->Fill();
	if (calibeventLPMT && oecevt && saveCalib){
		std::cout << "Fill Calib " << std::endl;
		m_ntuple1->Fill();
		std::cout << "Fill OEC" << std::endl;
		m_ntuple6->Fill();
	}
	// if(recevent){
	// 	std::cout << "Fill Reco " << std::endl;
	// 	m_ntuple4->Fill();
	// }
	// if(wpcalibevt){
	// 	std::cout << "Fill WpCalib" << std::endl;
	// 	m_ntuple5->Fill();
	// }



	return true;

}

bool SNiPERToPlainTree::Book_tree()
{

	SniperPtr<RootWriter> svc(*getRoot(),"RootWriter");

	// m_ntuple = svc->bookTree(*m_par,"Data/Sim", "SimTree");

	// m_ntuple->Branch("EntryNb", &m_iEvt, "EntryNb/I");
	// //	m_ntuple->Branch("pdgid",&m_pdgid);
	// m_ntuple->Branch("PDGID",&m_PDGID);
	// m_ntuple->Branch("ParentID",&m_ParentID);
	// m_ntuple->Branch("TrackID",&m_TrackID);
	// m_ntuple->Branch("nPE", &m_PE);
	// m_ntuple->Branch("TotalPE", &m_TotalPESim);
	// // m_ntuple->Branch("Qedep",&m_Qedep);
	// // m_ntuple->Branch("Edep",&m_Edep);
	// // m_ntuple->Branch("Vtx", &m_Vtx, "Vtx/D");
	// // m_ntuple->Branch("Vty", &m_Vty, "Vty/D");
	// // m_ntuple->Branch("Vtz", &m_Vtz, "Vtz/D");
	// m_ntuple->Branch("NbHitLPMTSim", &m_NbHitLPMTSim, "NbHitLPMTSim/I");
	// m_ntuple->Branch("NbHitSPMTSim", &m_NbHitSPMTSim, "NbHitSPMTSim/I");
	// m_ntuple->Branch("TimeStampInNanoSec", &m_TimeStampInNanoSec, "TimeStampInNanoSec/l");
	// m_ntuple->Branch("PmtIdSim",&m_PmtIdSim);
	// m_ntuple->Branch("PmtHitTrackID",&m_PmtHitTrackID);
	// m_ntuple->Branch("HitTimeSim", &m_HitTimeSim);
	// m_ntuple->Branch("ChargeSim", &m_ChargeSim);

	if(saveCalib){
		m_ntuple1 = svc->bookTree(*m_par,"Data/Calib", "CalibTree");
		m_ntuple1->Branch("EntryNb", &m_iEvt, "EntryNb/I");
		m_ntuple1->Branch("RunID", &m_iRun);
		m_ntuple1->Branch("TriggerTime", &m_TriggerTime);
		m_ntuple1->Branch("TriggerType", &m_TriggerType);
		m_ntuple1->Branch("TimeStamp", &m_TimeStamp);
		m_ntuple1->Branch("ChargeTotLPMT", &m_ChargeTotLPMT);
		m_ntuple1->Branch("NbHitLPMTCalib", &m_NbHitLPMTCalib, "NbHitLPMTCalib/I");
		m_ntuple1->Branch("NbHitSPMTCalib", &m_NbHitSPMTCalib, "NbHitSPMTCalib/I");
		m_ntuple1->Branch("PmtIDCalib", &m_PmtIdCalib);
		// m_ntuple1->Branch("PmtCircle", &m_PMTCircle);
		// m_ntuple1->Branch("PmtType", &m_PMTType);
		m_ntuple1->Branch("HitTimeCalib", &m_HitTimeCalib);
		m_ntuple1->Branch("ChargeCalib", &m_ChargeCalib);


		m_ntuple6 = svc->bookTree(*m_par, "Data/OEC", "OEC Event tree");
		m_ntuple6->Branch("EntryNb", &m_iEvt, "EntryNb/I");
		m_ntuple6->Branch("RunId", &m_iRun, "RunID/I");
		m_ntuple6->Branch("TimeStamp", &m_TimeStamp);
		m_ntuple6->Branch("OecTotCharge", &m_OecTotCharge);
		m_ntuple6->Branch("OecEnergy", &m_OecEnergy);
		m_ntuple6->Branch("OecX", &m_OecX);
		m_ntuple6->Branch("OecY", &m_OecY);
		m_ntuple6->Branch("OecZ", &m_OecZ);
	}


	// m_ntuple4 = svc->bookTree(*m_par, "Data/Reco", "Reconstruction Tree");
	// m_ntuple4->Branch("EntryNb", &m_iEvt, "EntryNb/I");
	// m_ntuple4->Branch("NFiredPMT", &m_NFiredPMT, "NFiredPMT/I");
	// m_ntuple4->Branch("TotalPE", &m_TotalPE);
	// m_ntuple4->Branch("RecEnergy", &m_RecE, "RecEnergy/D");
	// m_ntuple4->Branch("Recx", &m_RecX, "Recx/D");
	// m_ntuple4->Branch("Recy", &m_RecY, "Recy/D");
	// m_ntuple4->Branch("Recz", &m_RecZ, "Recz/D");
	// m_ntuple4->Branch("RecT0", &m_T0, "RecT0/D");
	// m_ntuple4->Branch("HitTimeTOF", &m_HitTimeTOF);

	// m_ntuple5 = svc->bookTree(*m_par, "Data/WpCalib", "Water Pool calibration tree");
	// m_ntuple5->Branch("EntryNb", &m_iEvt, "EntryNb/I");
	// m_ntuple5->Branch("TimeStamp", &m_TimeStamp);
	// m_ntuple5->Branch("nPE", &m_WpNPE);
	// m_ntuple5->Branch("PmtID", &m_WpPmtId);
	// m_ntuple5->Branch("Charge", &m_WpCharge);
	// m_ntuple5->Branch("Time", &m_WpHitTime);

	m_ntuple7 = svc->bookTree(*m_par, "Data/IBD", "Condensed tree of Prompt information");
	m_ntuple7->Branch("RunId", &m_iRun, "RunId/I");
	m_ntuple7->Branch("File", &m_File);
	m_ntuple7->Branch("Tag", &m_EventTag);
	m_ntuple7->Branch("TimeDifference", &m_TimeDifference);
	m_ntuple7->Branch("PromptEntry", &m_PromptEvt);
	m_ntuple7->Branch("PromptCharge", &m_PromptCharge);
	m_ntuple7->Branch("PromptTimeStamp", &m_TimeStamp);
	// m_ntuple7->Branch("CorDelay", &m_DelayEvt);
	// m_ntuple7->Branche("DelayCharge", &m_DelayCharge);
	m_ntuple7->Branch("PromptX", &m_PromptX);
	m_ntuple7->Branch("PromptY", &m_PromptY);
	m_ntuple7->Branch("PromptZ", &m_PromptZ);
	m_ntuple7->Branch("PromptHitTimeTOF", &m_PromptHitTimeTOF);
	m_ntuple7->Branch("DelayEntry", &m_DelayEvt);
	m_ntuple7->Branch("DelayCharge", &m_DelayCharge);
	m_ntuple7->Branch("DelayTimeSamp", &m_DelayTimeStamp);
	m_ntuple7->Branch("DelayX", &m_DelayX);
	m_ntuple7->Branch("DelayY", &m_DelayY);
	m_ntuple7->Branch("DelayZ", &m_DelayZ);
	m_ntuple7->Branch("DelayHitTimeTOF", &m_DelayHitTimeTOF);
	// m_ntuple7->Branch("PmtId", &m_PmtIdCalib);
	// m_ntuple7->Branch("HitTime", &m_HitTimeCalib);
	// m_ntuple7->Branch("HitCharge", &m_ChargeCalib);

	// m_ntuple8 = svc->bookTree(*m_par, "Data/Delay", "Condensed tree of Delay information");
	// m_ntuple8->Branch("EntryNb", &m_DelayEvt, "EntryNb/I");
	// m_ntuple8->Branch("RunId", &m_iRun, "RunId/I");
	// m_ntuple8->Branch("File", &m_File);
	// m_ntuple8->Branch("Tag", &m_EventTag);
	// m_ntuple8->Branch("TimeStamp", &m_TimeStamp);
	// m_ntuple8->Branch("DelayCharge", &m_DelayCharge);
	// m_ntuple8->Branch("TimeDifference", &m_TimeDifference);
	// m_ntuple8->Branch("CorPrompt", &m_PromptEvt);
	// m_ntuple8->Branch("DelayX", &m_DelayX);
	// m_ntuple8->Branch("DelayY", &m_DelayY);
	// m_ntuple8->Branch("DelayZ", &m_DelayZ);
	// // m_ntuple8->Branch("PmtId", &m_PmtIdCalib);
	// // m_ntuple8->Branch("HitTime", &m_HitTimeCalib);
	// m_ntuple8->Branch("HitTimeTOF", &m_HitTimeTOF);
	// // m_ntuple8->Branch("HitCharge", &m_ChargeCalib);
	
	return true;
}


bool SNiPERToPlainTree::finalize()
{
	LogDebug << "finalizing" << std::endl;
	return true;
}






// --------------------------------------------------------------------------
// For IBD Selection
// --------------------------------------------------------------------------


bool SNiPERToPlainTree::findCorrelation(JM::NavBuffer::Iterator navit, std::vector<JM::OecEvt*>& pEvt){

	std::cout << "Running IBD selection " << std::endl;
	foundDelay = false;

	const TTimeStamp& ttime = pEvt[0]->getTime();

	int DelayCount = 0;
	int bCount = 0;
	int offset = 0;

	unsigned long dtime = 0.0;

	for(JM::NavBuffer::Iterator tmpit = ++navit; tmpit != m_buf->end(); ++tmpit){

		std::cout << "Searching..." << std::endl;

		JM::OecHeader* aHeaderOEC = JM::getHeaderObject<JM::OecHeader>(tmpit->get());
		JM::OecEvt* aEventOEC = dynamic_cast<JM::OecEvt*>(aHeaderOEC->event("JM::OecEvt"));

		const TTimeStamp& afterTime = aEventOEC->getTime();
		double aEnergy = aEventOEC->getEnergy();
		double distance = sqrt(
			pow(aEventOEC->getVertexX() - pEvt.at(0)->getVertexX(), 2) +
			pow(aEventOEC->getVertexY() - pEvt.at(0)->getVertexY(), 2) +
			pow(aEventOEC->getVertexZ() - pEvt.at(0)->getVertexZ(), 2)
		);

		dtime = ((afterTime.GetSec() - ttime.GetSec())*1000000000ULL + (afterTime.GetNanoSec() - ttime.GetNanoSec())) * 1e-3; //in us
		std::cout << "Current event dt from prompt: " << dtime << " us" << std::endl;

		double afterTotalCharge = aEventOEC->getTotalCharge();

		bool dt_cut = dtime > 10 && dtime < 1000;
		bool position_cut = distance < 1500;

		if(dtime > 1000){
			break;
		}
		offset++;
		if(position_cut && dt_cut && (afterTotalCharge > 3700 && afterTotalCharge < 6000) && aEnergy != -1){
			m_DelayCharge = afterTotalCharge;
			m_DelayTimeStamp = afterTime;
			m_TimeDifference = dtime * 1e3;
			m_DelayX = aEventOEC->getVertexX();
			m_DelayY = aEventOEC->getVertexY();
			m_DelayZ = aEventOEC->getVertexZ();
			m_CorPrompt = m_PromptEvt;
			DelayIt = tmpit;
			std::cout << "Delay Nav " << tmpit->get() << std::endl; 

			foundDelay = isMuonVetoed(DelayIt) && isIsolated(DelayIt);
			break;
		}
	}

	if(foundDelay){
		m_DelayIBD = m_iEvt + offset;
		m_DelayEvt = m_DelayIBD;
		std::cout << " Offset " << offset << "\n" << std::endl;
	}
	
	return foundDelay;
}


bool SNiPERToPlainTree::isMuonVetoed(JM::NavBuffer::Iterator navit){

	JM::OecHeader* tHeaderOEC = JM::getHeaderObject<JM::OecHeader>(navit->get());
	JM::OecEvt* tEventOEC = dynamic_cast<JM::OecEvt*>(tHeaderOEC->event("JM::OecEvt"));

	const TTimeStamp& ttime = tEventOEC->getTime();
	
	uint64_t dtime = 0.0;
	bool isMuon = false;
	
	std::cout << "\n Muon Search..." << std::endl;
	for(JM::NavBuffer::Iterator tmpit = --navit; tmpit != m_buf->begin(); --tmpit){

		JM::OecHeader* bHeaderOEC = JM::getHeaderObject<JM::OecHeader>(tmpit->get());
		JM::OecEvt* bEventOEC = dynamic_cast<JM::OecEvt*>(bHeaderOEC->event("JM::OecEvt"));

		const TTimeStamp& beforetime = bEventOEC->getTime();
		dtime = ((ttime.GetSec() -  beforetime.GetSec())*1000000000ULL + (ttime.GetNanoSec() - beforetime.GetNanoSec())) * 1e-3; // in us

		if(tmpit == m_buf->current()){
			continue;
		}
		if(dtime > 2000){
			std::cout << "-- No Muon found!" << std::endl;
			break;
		}
		if(bEventOEC->getTotalCharge() > 50000 && dtime < 2000){
			isMuon = true;
			break;
		}
	}

	std::cout << " No Muon " << !isMuon << std::endl;
	return !isMuon;

}

bool SNiPERToPlainTree::isIsolated(JM::NavBuffer::Iterator navit){

	
	JM::OecHeader* tHeaderOEC = JM::getHeaderObject<JM::OecHeader>(navit->get());
	JM::OecEvt* tEventOEC = dynamic_cast<JM::OecEvt*>(tHeaderOEC->event("JM::OecEvt"));
	
	const TTimeStamp& ttime = tEventOEC->getTime();
	uint64_t dtime = 0.0;
	double distance = 0.0;
	
	bool isMultiplicity = false;
	
	std::cout << "\n Multiplicity Search..." << std::endl;
	
	//---------------------- Check Multiplicity 2ms before Delay ---------------------
	for(JM::NavBuffer::Iterator preIt = (navit - 1); preIt != m_buf->begin(); --preIt){

		JM::OecHeader* bHeaderOEC = JM::getHeaderObject<JM::OecHeader>(preIt->get());
		JM::OecEvt* bEventOEC = dynamic_cast<JM::OecEvt*>(bHeaderOEC->event("JM::OecEvt"));

		const TTimeStamp& beforetime = bEventOEC->getTime();
		dtime = ((ttime.GetSec() -  beforetime.GetSec())*1000000000ULL + (ttime.GetNanoSec() - beforetime.GetNanoSec())) * 1e-3; // in us

		distance = sqrt(
			pow(tEventOEC->getVertexX() - bEventOEC->getVertexX(), 2) +
			pow(tEventOEC->getVertexY() - bEventOEC->getVertexY(), 2) +
			pow(tEventOEC->getVertexZ() - bEventOEC->getVertexZ(), 2)
		);

		bool charge_cut = (bEventOEC->getTotalCharge() >= 2000 && bEventOEC->getTotalCharge() <= 20000) && bEventOEC->getEnergy() != 1;

		if(preIt == m_buf->current()){
			continue;
		}
		if(dtime > 2000){
			std::cout << "-- No Multiplicity before !" << std::endl;
			break;
		}
		if(distance < 1500 && dtime < 2000 && charge_cut){
			isMultiplicity = true;
			break;
		}
	}

	
	//---------------------- Check Multiplicity 1ms after Delay ---------------------

	for(JM::NavBuffer::Iterator postIt = (navit + 1); postIt != m_buf->end(); ++postIt){

		std::cout << "Post Delay Nav " << postIt->get() << std::endl; 

		JM::OecHeader* aHeaderOEC = JM::getHeaderObject<JM::OecHeader>(postIt->get());
		JM::OecEvt* aEventOEC = dynamic_cast<JM::OecEvt*>(aHeaderOEC->event("JM::OecEvt"));

		const TTimeStamp& aftertime = aEventOEC->getTime();
		dtime = ((aftertime.GetSec() - ttime.GetSec())*1000000000ULL + (aftertime.GetNanoSec() -  ttime.GetNanoSec())) * 1e-3; // in us

		std::cout << "Time difference " << dtime << std::endl;

		distance = sqrt(
			pow(aEventOEC->getVertexX() - tEventOEC->getVertexX(), 2) +
			pow(aEventOEC->getVertexY() - tEventOEC->getVertexY(), 2) +
			pow(aEventOEC->getVertexZ() - tEventOEC->getVertexZ(), 2)
		);

		std::cout << "Distance " << distance << std::endl;


		bool charge_cut = (aEventOEC->getTotalCharge() > 3700 && aEventOEC->getTotalCharge() < 6000) && aEventOEC->getEnergy() != 1;
		if(dtime > 1000){
			std::cout << "-- No Multiplicity after !" << std::endl;
			break;
		}
		if(distance < 1500 && dtime < 1000 && charge_cut){
			isMultiplicity = true;
			break;
		}
	}

	std::cout << " No Multiplicity " << !(isMultiplicity) << std::endl;
	return !(isMultiplicity);
}

bool SNiPERToPlainTree::FillPromptDelay(JM::NavBuffer::Iterator pIt, JM::NavBuffer::Iterator dIt){

	// ========================= Fill Prompt =========================

	auto calibheader = JM::getHeaderObject<JM::CdLpmtCalibHeader>(pIt->get());
	auto calibevt = dynamic_cast<JM::CdLpmtCalibEvt*>(calibheader->event("JM::CdLpmtCalibEvt"));

	const auto& chhlistLPMT = calibheader->event()->calibPMTCol();
	m_PromptHitTimeTOF.clear();
	for(auto chit = chhlistLPMT.begin(); chit != chhlistLPMT.end(); ++chit){
		
		unsigned int pmtId = (*chit)->pmtId();
		Identifier id = Identifier(pmtId);
		int TruePM = CdID::module(id);

		for (unsigned int j = 0; j < (*chit)->size(); j++){

			double interface = (gInterfaceLevel->Eval(m_TimeStamp*1e-9) - 17.7) * 1e3;
			TVector3 vertex(m_PromptX, m_PromptY, m_PromptZ);
			TOFCalculator TOF(vertex, ALL_LPMT_pos.at(TruePM), interface);

			double time = (*chit)->time(j) - TOF.CalLTOF();
			m_PromptHitTimeTOF.push_back(time);
		}
	}

	// ========================= Fill Delay =========================

	calibheader = JM::getHeaderObject<JM::CdLpmtCalibHeader>(dIt->get());
	calibevt = dynamic_cast<JM::CdLpmtCalibEvt*>(calibheader->event("JM::CdLpmtCalibEvt"));

	const auto& chhlistLPMTDelay = calibheader->event()->calibPMTCol();
	m_DelayHitTimeTOF.clear();
	for(auto chit = chhlistLPMTDelay.begin(); chit != chhlistLPMTDelay.end(); ++chit){

		unsigned int pmtId = (*chit)->pmtId();
		Identifier id = Identifier(pmtId);
		int TruePM = CdID::module(id);

		for (unsigned int j = 0; j < (*chit)->size(); j++){

			double interface = (gInterfaceLevel->Eval(m_TimeStamp*1e-9) - 17.7) * 1e3;
			TVector3 vertex(m_DelayX, m_DelayY, m_DelayZ);
			TOFCalculator TOF(vertex, ALL_LPMT_pos.at(TruePM), interface);

			double time = (*chit)->time(j) - TOF.CalLTOF();
			m_DelayHitTimeTOF.push_back(time);
		}
	}

	m_ntuple7->Fill();
	
	return true;
}










// --------------------------------------------------------------------------
// TOF Calculation
// --------------------------------------------------------------------------

double SNiPERToPlainTree::ComputeLTOF(double pmtid, double evtx, double evty, double evtz){
	double pmt_pos_x = ALL_LPMT_pos.at(pmtid).X();
	double pmt_pos_y = ALL_LPMT_pos.at(pmtid).Y();
	double pmt_pos_z = ALL_LPMT_pos.at(pmtid).Z();

	double dx = (pmt_pos_x - evtx);
	double dy = (pmt_pos_y - evty);
	double dz = (pmt_pos_z - evtz);

	double Evt = sqrt(evtx*evtx + evty*evty + evtz*evtz);
	double Dist = sqrt(dx*dx + dy*dy + dz*dz);
	double costheta = (Dist*Dist + PMT_R*PMT_R*1e6 - Evt*Evt)/(2.*Dist*PMT_R*1e3); //Al Kashi
	double LengthWater = 1e3*PMT_R*costheta - 1e3*sqrt(PMT_R*costheta*PMT_R*costheta - PMT_R*PMT_R + LS_R*LS_R);

	return RfrIndxLS*(Dist-LengthWater)*1e6/c + RfrIndxWR*LengthWater*1e6/c;
}

double SNiPERToPlainTree::ComputeSTOF(double pmtid, double evtx, double evty, double evtz){
	pmtid = pmtid - 20000;
	double pmt_pos_x = ALL_SPMT_pos.at(pmtid).X();
	double pmt_pos_y = ALL_SPMT_pos.at(pmtid).Y();
	double pmt_pos_z = ALL_SPMT_pos.at(pmtid).Z();

	double dx = (pmt_pos_x - evtx);
	double dy = (pmt_pos_y - evty);
	double dz = (pmt_pos_z - evtz);

	double Evt = sqrt(evtx*evtx + evty*evty + evtz*evtz);
	double Dist = sqrt(dx*dx + dy*dy + dz*dz);
	double costheta = (Dist*Dist + PMT_R*PMT_R*1e6 - Evt*Evt)/(2.*Dist*PMT_R*1e3); //Al Kashi
	double LengthWater = 1e3*PMT_R*costheta - 1e3*sqrt(PMT_R*costheta*PMT_R*costheta - PMT_R*PMT_R + LS_R*LS_R);

	return RfrIndxLS*(Dist-LengthWater)*1e6/c + RfrIndxWR*LengthWater*1e6/c;
}
