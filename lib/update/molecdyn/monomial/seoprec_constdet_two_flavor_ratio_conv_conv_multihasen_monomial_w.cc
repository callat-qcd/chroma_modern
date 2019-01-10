/*! @file
 * @brief Two-flavor collection of unpreconditioned 4D ferm monomials
 */

#include "chromabase.h"
#include "update/molecdyn/monomial/seoprec_constdet_two_flavor_ratio_conv_conv_multihasen_monomial_w.h"
#include "update/molecdyn/monomial/monomial_factory.h"

#include "actions/ferm/fermacts/fermact_factory_w.h"
#include "actions/ferm/fermacts/fermacts_aggregate_w.h"

#include "update/molecdyn/predictor/chrono_predictor_factory.h"
#include "update/molecdyn/predictor/zero_guess_predictor.h"


namespace Chroma 
{ 

	namespace SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomialEnv
	{
		namespace
		{
			//! Callback function for the factory
			Monomial< multi1d<LatticeColorMatrix>,
				multi1d<LatticeColorMatrix> >* createMonomial(XMLReader& xml, const std::string& path) 
				{
					return new SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial(
							TwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomialParams(xml, path));
				}

			//! Local registration flag
			bool registered = false;
		}

		const std::string name("TWO_FLAVOR_SEOPREC_CONSTDET_RATIO_CONV_CONV_MULTIHASEN_FERM_MONOMIAL");

		//! Register all the factories
		bool registerAll() 
		{
			bool success = true; 
			if (! registered)
			{
				success &= WilsonTypeFermActs4DEnv::registerAll();
				success &= TheMonomialFactory::Instance().registerObject(name, createMonomial);
				registered = true;
			}
			return success;
		}
	} //end namespace SeoPrec TwoFlavorRatioConvConvWilsonFermMonomialEnv

	// Constructor
	SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial::
		SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial(
				const TwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomialParams& param) 
		{
			START_CODE();
			QDPIO::cout << "Constructor: " << __func__ << std::endl;

			if( param.fermactInv.invParam.id == "NULL" ) {
				QDPIO::cerr << "Fermact inverter params are NULL" << std::endl;
				QDP_abort(1);
			}

			invParam = param.fermactInv.invParam;
			// Fermion action (old shifted action)
		//	{
		//		std::istringstream is(param.fermactInv.fermact.xml);
		//		XMLReader fermact_reader(is);
		//		QDPIO::cout << "Construct fermion action= " << param.fermactInv.fermact.id << std::endl;

		//		WilsonTypeFermAct<T,P,Q>* tmp_act = 
		//			TheWilsonTypeFermActFactory::Instance().createObject(param.fermactInv.fermact.id, 
		//					fermact_reader, 
		//					param.fermactInv.fermact.path);

		//		ShiftSymEvenOddPrecCloverFermAct* downcast=dynamic_cast<ShiftSymEvenOddPrecCloverFermAct*>(tmp_act);

		//		// Check success of the downcast 
		//		if( downcast == 0x0 ) 
		//		{
		//			QDPIO::cerr << __func__ << ": unable to downcast FermAct to ShiftSymEvenOddPrecCloverFermAct" << std::endl;
		//			QDP_abort(1);
		//		}

		//		fermact = downcast;    
		//	}

			// Fermion action (with original seoprec action)
			{
				std::istringstream is(param.fermactInv.fermact.xml);
				XMLReader fermact_reader(is);
				QDPIO::cout << "Construct fermion action= " << param.fermactInv.fermact.id << std::endl;

				WilsonTypeFermAct<T,P,Q>* tmp_act = 
					TheWilsonTypeFermActFactory::Instance().createObject(param.fermactInv.fermact.id, 
							fermact_reader, 
							param.fermactInv.fermact.path);

				SymEvenOddPrecLogDetWilsonTypeFermAct<T,P,Q>* 
					downcast=dynamic_cast<SymEvenOddPrecLogDetWilsonTypeFermAct<T,P,Q>*>(tmp_act);

				// Check success of the downcast 
				if( downcast == 0x0 ) 
				{
					QDPIO::cerr << __func__ << 
						": unable to downcast FermAct to SymEvenOddPrecWilsonTypeFermAct" << std::endl;
					QDP_abort(1);
				}
				fermact = downcast;    
			}

			// Number of Hasenbusch term
			numHasenTerms = param.numHasenTerms;

			// Shifted mass
			mu.resize(numHasenTerms+1);
			for(int i=0; i< numHasenTerms+1; ++i){
				mu[i] = param.mu[i];
			}

			// Pseudofermion field phi
			phi.resize(numHasenTerms);
			for(int i=0; i<numHasenTerms; ++i)
				phi[i] = zero;

			// Get Chronological predictor
			{
				AbsChronologicalPredictor4D<LatticeFermion>* tmp = 0x0;
				if( param.predictor.xml == "" ) {
					// No predictor specified use zero guess
					tmp = new ZeroGuess4DChronoPredictor();
				}else {
					try { 
						std::istringstream chrono_is(param.predictor.xml);
						XMLReader chrono_xml(chrono_is);
						tmp = The4DChronologicalPredictorFactory::Instance().createObject(param.predictor.id, 
								chrono_xml, 
								param.predictor.path);
					}catch(const std::string& e ) { 
						QDPIO::cerr << "Caught Exception Reading XML: " << e << std::endl;
						QDP_abort(1);
					}
				}
				if( tmp == 0x0 ) { 
					QDPIO::cerr << "Failed to create the 4D ChronoPredictor" << std::endl;
					QDP_abort(1);
				}
				chrono_predictor = tmp;
			}

			QDPIO::cout << "Finished constructing: " << __func__ << std::endl;
			END_CODE();
		}
	// Sum over all Hasenbusch term to get total action
// old shifted fermact method
#if 0
	Double SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial::
		S(const AbsFieldState<P,Q>& s){

			START_CODE();
			XMLWriter& xml_out = TheXMLLogWriter::Instance();
			push(xml_out, "SeoprecConstDetTwoFlavorRatioConvConvMultihasenFermMonomial");

			// Fermion action
			ShiftSymEvenOddPrecCloverFermAct& FA = getFermAct();
			ShiftSymEvenOddPrecCloverFermAct FA_prec(FA);
			Double S=0;
			// Get the X fields
			T X;
			for(int i=0; i<numHasenTerms; ++i){

				FA.setMu(mu[i]);
				FA_prec.setMu(mu[i+1]);

				// Create a state for linop
				Handle<FermState<T,P,Q> > state(FA.createState(s.getQ()));
				// Need way to get gauge state from AbsFieldState<P,Q>
				Handle<SymEvenOddPrecLogDetLinearOperator<T,P,Q> > M(FA.linOp(state));
				Handle<SymEvenOddPrecLogDetLinearOperator<T,P,Q> > M_prec(FA_prec.linOp(state));

				// Action calc doesnt use chrono predictor use zero guess
				X[M->subset()] = zero;
				// Reset chrono predictor
				QDPIO::cout << "TwoFlavorRatioConvConvMultihasenWilson4DMonomial: resetting Predictor before energy calc solve" << std::endl;
				(getMDSolutionPredictor()).reset();
				// Get system solver
				Handle<MdagMSystemSolver<T> > invMdagM(FA.invMdagM(state,getInvParams()));
				// M_dag_prec phi = M^{dag}_prec \phi - the RHS
				T M_dag_prec_phi;
				(*M_prec)(M_dag_prec_phi, getPhi(i), MINUS);

				// Solve MdagM X = eta
				SystemSolverResults_t res = (*invMdagM)(X, M_dag_prec_phi);

				T phi_tmp = zero;
				(*M_prec)(phi_tmp, X, PLUS);
				Double action = innerProductReal(getPhi(i), phi_tmp, M->subset());
				// Write out inversion number and action for every hasenbusch term 
				std::string n_count = "n_count_hasenterm_" + std::to_string(i+1);
				std::string s_action = "S_hasenterm_" + std::to_string(i+1);

				write(xml_out, n_count, res.n_count);
				write(xml_out, s_action, action);

				S += action;
			}
			write(xml_out, "S", S);
			pop(xml_out);

			END_CODE();
			return S;
		}

	// Sum over all Force terms
	void SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial::
		dsdq(P& F, const AbsFieldState<P,Q>& s){
			START_CODE();
			XMLWriter& xml_out = TheXMLLogWriter::Instance();
			push(xml_out, "TwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial");

			P F_t;
			F_t.resize(Nd);
			F.resize(Nd);
			for(int i=0; i<Nd; ++i)
			{
				F_t[i] = zero;
				F[i] = zero;
			}

			// Fermion action
			ShiftSymEvenOddPrecCloverFermAct& FA = getFermAct();
			ShiftSymEvenOddPrecCloverFermAct FA_prec(FA);
			// X = (M^\dag M)^(-1) M_prec^\dag \phi
			// Y = M X
			T X = zero;
			T Y = zero;
			// M_dag_prec_phi = M^\dag_prec \phi
			T M_dag_prec_phi;

			for(int i=0; i<numHasenTerms; ++i){
				FA.setMu(mu[i]);
				FA_prec.setMu(mu[i+1]);

				// Create a state for linop
				Handle<FermState<T,P,Q> > state(FA.createState(s.getQ()));
				// Need way to get gauge state from AbsFieldState<P,Q>
				Handle<SymEvenOddPrecLogDetLinearOperator<T,P,Q> > M(FA.linOp(state));
				Handle<SymEvenOddPrecLogDetLinearOperator<T,P,Q> > M_prec(FA_prec.linOp(state));
				// Get system solver
				Handle<MdagMSystemSolver<T> > invMdagM(FA.invMdagM(state,getInvParams()));
				
				(*M_prec)(M_dag_prec_phi, getPhi(i), MINUS);
				SystemSolverResults_t res = (*invMdagM)(X, M_dag_prec_phi, getMDSolutionPredictor());
				(*M)(Y, X, PLUS);

				// deriv part 1: \phi^\dag \dot(M_prec) X
				M_prec->deriv(F_t, getPhi(i), X, PLUS);

				// deriv part 2: - X^\dag \dot(M^\dag) Y
				P F_tmp;
				M->deriv(F_tmp, X, Y, MINUS);
				F_t -= F_tmp;

				// deriv part 3: - Y^\dag \dot(M) X
				M->deriv(F_tmp, Y, X, PLUS);
				F_t -= F_tmp;

				// deriv part 4: X^\dag \dot(M_prec)^\dag \phi
				M_prec->deriv(F_tmp, X, getPhi(i), MINUS);
				F_t += F_tmp;

				// total force from all Hasenbusch terms
				F += F_t;
				
				// F now holds derivative with respect to possibly fat links
				// now derive it with respect to the thin links if needs be
				state->deriv(F);

				// Write out inversion number and action for every hasenbusch term 
				std::string n_count = "n_count_hasenterm_" + std::to_string(i+1);
				std::string force = "Forces_hasenterm_" + std::to_string(i+1);
				
				write(xml_out, n_count, res.n_count);
				monitorForces(xml_out, force, F_t);
			}

			// Total force from all Hasenbusch terms
			monitorForces(xml_out, "Forces", F);
			pop(xml_out);
			END_CODE();
		}
#endif

	// based on TwistedShiftedLinOp
	Double SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial::
		S(const AbsFieldState<P,Q>& s){
			START_CODE();
			XMLWriter& xml_out = TheXMLLogWriter::Instance();
			push(xml_out, "SeoprecConstDetTwoFlavorRatioConvConvMultihasenFermMonomial");

			// Fermion action
			//SymEvenOddPrecWilsonTypeFermAct<T,P,Q>& FA = getFermAct();
			SymEvenOddPrecLogDetWilsonTypeFermAct<T,P,Q>& FA = getFermAct();
			Double S=0;
			// Get the X fields
			T X;
			Handle<FermState<T,P,Q> > state(FA.createState(s.getQ()));
			Handle<SymEvenOddPrecLogDetLinearOperator<T,P,Q> > base_op(FA.linOp(state));

			for(int i=0; i<numHasenTerms; ++i){
				Handle<LinearOperator<T> > 
					M(new TwistedShiftedLinOp<T,P,Q,SymEvenOddPrecLogDetLinearOperator>(*base_op, mu[i]));
				Handle<LinearOperator<T> > 
					M_prec(new TwistedShiftedLinOp<T,P,Q,SymEvenOddPrecLogDetLinearOperator>(*base_op, mu[i+1]));

				// Action calc doesnt use chrono predictor use zero guess
				X[M->subset()] = zero;
				// Reset chrono predictor
				QDPIO::cout << 
					"TwoFlavorRatioConvConvMultihasenWilson4DMonomial: resetting Predictor before energy calc solve" << std::endl;
				(getMDSolutionPredictor()).reset();
				// Get system solver
				const GroupXML_t& invParam = getInvParams();
				std::istringstream xml(invParam.xml);
				XMLReader paramtop(xml);
				Handle<MdagMSystemSolver<T> > 
					invMdagM(TheMdagMFermSystemSolverFactory::Instance().createObject(
								invParam.id, paramtop, invParam.path, state, M));
				// M_dag_prec phi = M^{dag}_prec \phi - the RHS
				T M_dag_prec_phi;
				(*M_prec)(M_dag_prec_phi, getPhi(i), MINUS);

				// Solve MdagM X = eta
				SystemSolverResults_t res = (*invMdagM)(X, M_dag_prec_phi);

				T phi_tmp = zero;
				(*M_prec)(phi_tmp, X, PLUS);
				Double action = innerProductReal(getPhi(i), phi_tmp, M->subset());
				// Write out inversion number and action for every hasenbusch term 
				std::string n_count = "n_count_hasenterm_" + std::to_string(i+1);
				std::string s_action = "S_hasenterm_" + std::to_string(i+1);

				write(xml_out, n_count, res.n_count);
				write(xml_out, s_action, action);
				S += action;
			}
			write(xml_out, "S", S);
			pop(xml_out);

			END_CODE();
			return S;
		}
	
	void SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial::
		dsdq(P& F, const AbsFieldState<P,Q>& s){
			START_CODE();
			XMLWriter& xml_out = TheXMLLogWriter::Instance();
			push(xml_out, "TwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial");

			P F_t;
			P F_tmp;
			F_t.resize(Nd);
			F_tmp.resize(Nd);
			F.resize(Nd);
			for(int i=0; i<Nd; ++i)
			{
				F_t[i] = zero;
				F_tmp[i] = zero;
				F[i] = zero;
			}
			// Fermion action
			SymEvenOddPrecLogDetWilsonTypeFermAct<T,P,Q>& FA = getFermAct();
			// X = (M^\dag M)^(-1) M_prec^\dag \phi
			// Y = M X
			T X = zero;
			T Y = zero;
			// M_dag_prec_phi = M^\dag_prec \phi
			T M_dag_prec_phi;

			Handle<FermState<T,P,Q> > state(FA.createState(s.getQ()));
			Handle<SymEvenOddPrecLogDetLinearOperator<T,P,Q> > base_op(FA.linOp(state));
			
			for(int i=0; i<numHasenTerms; ++i){
				Handle<LinearOperator<T> > 
					M(new TwistedShiftedLinOp<T,P,Q,SymEvenOddPrecLogDetLinearOperator>(*base_op, mu[i]));
				Handle<LinearOperator<T> > 
					M_prec(new TwistedShiftedLinOp<T,P,Q,SymEvenOddPrecLogDetLinearOperator>(*base_op, mu[i+1]));
				// Get system solver
				const GroupXML_t& invParam = getInvParams();
				std::istringstream xml(invParam.xml);
				XMLReader paramtop(xml);
				Handle<MdagMSystemSolver<T> > 
					invMdagM(TheMdagMFermSystemSolverFactory::Instance().createObject(
								invParam.id,paramtop, invParam.path, state, M));
				
				(*M_prec)(M_dag_prec_phi, getPhi(i), MINUS);
				SystemSolverResults_t res = (*invMdagM)(X, M_dag_prec_phi, getMDSolutionPredictor());
				(*M)(Y, X, PLUS);
				// cast linearop to difflinearop
				const DiffLinearOperator<T,P,Q>& diffM = dynamic_cast<const DiffLinearOperator<T,P,Q>&>(*M);
				const DiffLinearOperator<T,P,Q>& diffM_prec = dynamic_cast<const DiffLinearOperator<T,P,Q>&>(*M_prec);
#if 0
				// deriv part 1: \phi^\dag \dot(M_prec) X
				M_prec->deriv(F_t, getPhi(i), X, PLUS);

				// deriv part 2: - X^\dag \dot(M^\dag) Y
				M->deriv(F_tmp, X, Y, MINUS);
				F_t -= F_tmp;

				// deriv part 3: - Y^\dag \dot(M) X
				M->deriv(F_tmp, Y, X, PLUS);
				F_t -= F_tmp;

				// deriv part 4: X^\dag \dot(M_prec)^\dag \phi
				M_prec->deriv(F_tmp, X, getPhi(i), MINUS);
				F_t += F_tmp;
#endif
				// deriv part 1: \phi^\dag \dot(M_prec) X
				diffM_prec.deriv(F_t, getPhi(i), X, PLUS);

				// deriv part 2: - X^\dag \dot(M^\dag) Y
				diffM.deriv(F_tmp, X, Y, MINUS);
				F_t -= F_tmp;

				// deriv part 3: - Y^\dag \dot(M) X
				diffM.deriv(F_tmp, Y, X, PLUS);
				F_t -= F_tmp;

				// deriv part 4: X^\dag \dot(M_prec)^\dag \phi
				diffM_prec.deriv(F_tmp, X, getPhi(i), MINUS);
				F_t += F_tmp;
				
				// total force from all Hasenbusch terms
				F += F_t;
				
				// F now holds derivative with respect to possibly fat links
				// now derive it with respect to the thin links if needs be
				state->deriv(F);

				// Write out inversion number and action for every hasenbusch term 
				std::string n_count = "n_count_hasenterm_" + std::to_string(i+1);
				std::string force = "Forces_hasenterm_" + std::to_string(i+1);
				
				write(xml_out, n_count, res.n_count);
				monitorForces(xml_out, force, F_t);
			}
			// Total force from all Hasenbusch terms
			monitorForces(xml_out, "Forces", F);
			pop(xml_out);
			END_CODE();
		}

	// Refresh pseudofermion field of all Hasenbusch term
	void SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial::
		refreshInternalFields(const AbsFieldState<P,Q>& s){
			START_CODE();
			SymEvenOddPrecLogDetWilsonTypeFermAct<T,P,Q>& FA = getFermAct();

			Handle<FermState<T,P,Q> > state(FA.createState(s.getQ()));
			Handle<SymEvenOddPrecLogDetLinearOperator<T,P,Q> > base_op(FA.linOp(state));
			for(int i=0; i<numHasenTerms; ++i){
				Handle<LinearOperator<T> > 
					M(new TwistedShiftedLinOp<T,P,Q,SymEvenOddPrecLogDetLinearOperator>(*base_op, mu[i]));
				Handle<LinearOperator<T> > 
					M_prec(new TwistedShiftedLinOp<T,P,Q,SymEvenOddPrecLogDetLinearOperator>(*base_op, mu[i+1]));
				T eta = zero;
				gaussian(eta, M->subset());

				// Account for fermion BC by modifying the proposed field
				FA.getFermBC().modifyF(eta);

				eta *= sqrt(0.5);
				// Now we have to get the refreshment right:
				// We have  \eta^{\dag} \eta = \phi M_prec (M^dag M )^-1 M^dag_prec \phi
				//  so that \eta = (M^\dag)^{-1} M^{dag}_prec \phi
				//  So need to solve M^{dag}_prec \phi = M^{\dag} \eta
				// Which we can solve as 
				//      M^{dag}_prec M_prec ( M_prec^{-1} ) \phi = M^{\dag} \eta
				//
				// Zero out everything - to make sure there is no junk
				// in uninitialised places
				T eta_tmp = zero;
				T phi_tmp = zero;
				getPhi(i) = zero;

				(*M)(eta_tmp, eta, MINUS); // M^\dag \eta

				// Get system solver
				const GroupXML_t& invParam = getInvParams();
				std::istringstream xml(invParam.xml);
				XMLReader paramtop(xml);
				Handle<MdagMSystemSolver<T> > 
					invMdagM(TheMdagMFermSystemSolverFactory::Instance().createObject(
								invParam.id,paramtop, invParam.path, state, M_prec));
				
				// Solve MdagM_prec X = eta
				SystemSolverResults_t res = (*invMdagM)(phi_tmp, eta_tmp);
				(*M_prec)(getPhi(i), phi_tmp, PLUS); // (Now get phi = M_prec (M_prec^{-1}\phi)
				QDPIO::cout<<"TwoFlavRatioConvConvMultihasenWilson4DMonomial: resetting Predictor at end of field refresh"<<std::endl;
				getMDSolutionPredictor().reset();
				XMLWriter& xml_out = TheXMLLogWriter::Instance();
				std::string field_refrs = "FieldRefreshment_"+std::to_string(i);
				std::string n_count = "n_count_"+std::to_string(i);
				push(xml_out, field_refrs);
				write(xml_out, n_count, res.n_count);
				pop(xml_out);
			}
			END_CODE();
		}
	
	//! Copy pseudofermions if any
	void SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial::
		setInternalFields(const Monomial<P,Q>& m){
			START_CODE();
			try{
				const SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial& fm =
					dynamic_cast<const SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial&>(m);
				for(int i=0; i<numHasenTerms; ++i)
					getPhi(i) = fm.getPhi(i);
			}catch(std::bad_cast){
				QDPIO::cerr<<"Failed to cast input Monomial to SymEvenOddPrecConstDetTwoFlavorRatioConvConvMultihasenWilsonTypeFermMonomial"
					<<std::endl;
				QDP_abort(1);
			}
			END_CODE();
		}

} //end namespace Chroma
