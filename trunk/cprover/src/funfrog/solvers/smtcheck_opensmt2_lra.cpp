/*******************************************************************\

Module: Wrapper for OpenSMT2. Based on satcheck_minisat.

Author: Grigory Fedyukovich

\*******************************************************************/
#include "smtcheck_opensmt2_lra.h"
#include <util/type.h>
#include <funfrog/utils/naming_helpers.h>

/*******************************************************************\

Function: smtcheck_opensmt2t_lra::initializeSolver

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
void smtcheck_opensmt2t_lra::initializeSolver(const char* name)
{
    osmt = new Opensmt(opensmt_logic::qf_lra, name);
    lalogic = &(osmt->getLRALogic());
    logic = &(osmt->getLRALogic());
    mainSolver = &(osmt->getMainSolver());
    
    const char* msg = nullptr;
    osmt->getConfig().setOption(SMTConfig::o_produce_inter, SMTOption(true), msg);
    // msg is not allocated, do not free it!
    assert(strcmp(msg, "ok") == 0);

#ifndef NDEBUG
    // To avoid issues with type constraints for LRA
    if (type_constraints_level > 0)
        std::cout << "Adding Type Constraints (" << type_constraints_level << ")" 
                << ((type_constraints_level == 1 ? " for type constraints on non-deterministic input" : ""))
                << ((type_constraints_level == 2 ? " for type constraints on variables" : ""))
                << ((type_constraints_level >= 3  ? " ** ERROR ** Unknown Option" : ""))
                << std::endl;
#endif // NDEBUG not defined
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lra::~smtcheck_opensmt2t_lra

  Inputs:

 Outputs:

 Purpose: Free all inner objects

\*******************************************************************/
smtcheck_opensmt2t_lra::~smtcheck_opensmt2t_lra()
{
    // Shall/When need to: freeSolver() ?
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lra::check_ce

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
void smtcheck_opensmt2t_lra::check_ce(std::vector<exprt>& exprs)
{
	// this method is used for testing mostly
	char *msg = nullptr;

	for (int i = 0; i < top_level_formulas.size(); i++){
	    char *s = logic->printTerm(top_level_formulas[i]);
		std::cout << "\nCE:  " << s << '\n';
        free(s);
		mainSolver->insertFormula(top_level_formulas[i], &msg);
		if (msg !=nullptr) { free(msg); msg = nullptr; }
	}
	mainSolver->push();

	bool res = true;
	unsigned int i = 0;
	while (i < exprs.size() && res){
	    PTRef lp = expression_to_ptref(exprs[i]);
	    mainSolver->insertFormula(lp, &msg);
	    if (msg != nullptr) { free(msg); msg = nullptr; }
	    res = (s_True == mainSolver->check());
	    if (!res){
                char *s = logic->printTerm(lp);
	    	std::cout << "\n  Problem could be here: " << s << '\n';
                free(s);
	    }
//	    mainSolver->pop();  // TODO: uncomment this line and comment "&& res" in the guard
	    					// to get a segmfalut in the incremental solver
	    i++;
    }
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lra::getStringSMTlibDatatype

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
std::string smtcheck_opensmt2t_lra::getStringSMTlibDatatype(const typet& type)
{ 
    if ((type.id()==ID_bool) || (type.id() == ID_c_bool))
        return SMTConstants::SMT_BOOL;
    if (is_number(type))
        return SMTConstants::SMT_REAL;
    
    return SMTConstants::SMT_UNKNOWN; // Shall not get here
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lra::getSMTlibDatatype

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
SRef smtcheck_opensmt2t_lra::getSMTlibDatatype(const typet& type)
{ 
    if ((type.id()==ID_bool) || (type.id() == ID_c_bool))
        return lalogic->getSort_bool(); //SMT_BOOL
    if (is_number(type))
        return lalogic->getSort_num(); // SMT_REAL

    throw std::logic_error("Unknown datatype encountered!");
}
