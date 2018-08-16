/*******************************************************************\

Module: Wrapper for OpenSMT2. Based on satcheck_minisat.

Author: Grigory Fedyukovich

\*******************************************************************/
#include "smtcheck_opensmt2_lia.h"
#include <util/type.h>
#include "../hifrog.h"

/*******************************************************************\

Function: smtcheck_opensmt2t_lia::initializeSolver

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
void smtcheck_opensmt2t_lia::initializeSolver(const char* name)
{
    osmt = new Opensmt(opensmt_logic::qf_lia, name);
    lalogic = &(osmt->getLIALogic());
    logic = &(osmt->getLIALogic());
    mainSolver = &(osmt->getMainSolver());
    
    // KE: FIXME when OpenSMT has interpolation for LIA!
    //const char* msg=NULL;
    //osmt->getConfig().setOption(SMTConfig::o_produce_inter, SMTOption(true), msg);
    //if (msg==NULL) free((char *)msg);

    // KE: Fix a strange bug can be related to the fact we are pushing
    // a struct into std::vector and use [] before any push_back
    literals.push_back(PTRef());
    literalt l = new_variable(); // Shall be location 0, i.e., [l.var_no()] is [0] - NEVER COMMENT THIS LINE!!!
    literals[0] = logic->getTerm_true(); // Which is .x =0
    assert(l.var_no() != literalt::unused_var_no()); // KE: for cmake warnings
    // KE: End of fix
    
    // To avoid issues with type constraints for LIA
    if (type_constraints_level > 0)
        std::cout << "Adding Type Constraints (" << type_constraints_level << ")" 
                << ((type_constraints_level == 1 ? " for type constraints on non-deterministic input" : ""))
                << ((type_constraints_level == 2 ? " for type constraints on variables" : ""))
                << ((type_constraints_level >= 3  ? " ** ERROR ** Unknown Option" : ""))
                << std::endl;
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lia::~smtcheck_opensmt2t_lia

  Inputs:

 Outputs:

 Purpose: Free all inner objects

\*******************************************************************/
smtcheck_opensmt2t_lia::~smtcheck_opensmt2t_lia()
{
    // Shall/When need to: freeSolver() ?
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lia::type_cast

  Inputs:

 Outputs:

 Purpose:
 * 
 All is Integer in LIA so suppose to work id number to number
\*******************************************************************/
literalt smtcheck_opensmt2t_lia::type_cast(const exprt &expr) 
{
    std::cout << "*** BENCHMARK USES Type-CAST in LIA, test carefully! ***" << std::endl; // KE: remove after testing!
    
    // KE: New Cprover code - patching
    bool is_expr_bool = (expr.is_boolean() || (expr.type().id() == ID_c_bool)); 
    bool is_operands_bool = ((expr.operands())[0].is_boolean() 
                || ((expr.operands())[0].type().id() == ID_c_bool)); 

    // KE: Take care of type cast - recursion of convert take care of it anyhow
    // Unless it is constant bool, that needs different code:
    if (expr.type().id() == (expr.operands())[0].type().id()) {
        return convert((expr.operands())[0]);
    } else if (is_expr_bool && (expr.operands())[0].is_constant()) {
    	std::string val = extract_expr_str_number((expr.operands())[0]);
    	bool val_const_zero = (val.size()==0) || (stod(val)==0.0);
    	return const_var(!val_const_zero);
    } else if (is_number(expr.type()) && is_operands_bool) {
    	// Cast from Boolean to Integer - Add
    	literalt lt = convert((expr.operands())[0]); // Creating the Bool expression
    	PTRef ptl = logic->mkIte(literals[lt.var_no()], lalogic->mkConst("1"), lalogic->mkConst("0"));
      
#ifdef DISABLE_OPTIMIZATIONS
        if (dump_pre_queries)
        {
            // if the condition evaluated to constant, no ite was created
            if(logic->isIte(ptl))
            {
              char *s = logic->printTerm(logic->getTopLevelIte(ptl));
              ite_map_str.insert(make_pair(string(getPTermString(ptl)),std::string(s)));
              //cout << "; XXX oite symbol (type-cast): (" << ite_map_str.size() << ")"
              //    << string(getPTermString(ptl)) << endl << s << endl;
              free(s);
            }
        }
#endif        
        
    	return push_variable(ptl); // Keeps the new literal + index it
    } else if (is_expr_bool && is_number((expr.operands())[0].type())) {
    	// Cast from Integer to Boolean - Add
    	literalt lt = convert((expr.operands())[0]); // Creating the Bool expression
    	PTRef ptl = logic->mkNot(logic->mkEq(literals[lt.var_no()], lalogic->mkConst("0")));
    	return push_variable(ptl); // Keeps the new literal + index it
    } else { // All types of number to number, will take the inner value as the converted one
    	return convert((expr.operands())[0]);
    }
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lia::labs

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
literalt smtcheck_opensmt2t_lia::labs(const exprt &expr) 
{
    // ABS - all refers as real
    literalt lt = convert((expr.operands())[0]); // Create the inner part
    PTRef ptl = logic->mkIte(
                        lalogic->mkNumLt(literals[lt.var_no()], lalogic->getTerm_NumZero()),  // IF
                        lalogic->mkNumNeg(literals[lt.var_no()]),                 // Then
                        literals[lt.var_no()]);                                     // Else

#ifdef DISABLE_OPTIMIZATIONS
    if (dump_pre_queries)
    {
        char *s = logic->printTerm(logic->getTopLevelIte(ptl));
        ite_map_str.insert(make_pair(string(getPTermString(ptl)),std::string(s)));
        //cout << "; XXX oite symbol (labs):  (" << ite_map_str.size() << ")" 
        //            << string(getPTermString(ptl)) << endl << s << endl;
        free(s);        
    }
#endif
    
    literalt l = push_variable(ptl); // Keeps the new literal + index it

#ifdef SMT_DEBUG
    char* s = getPTermString(l);
    cout << "; (ABS) For " << expr.id() << " Created OpenSMT2 formula " << s << endl;
    free(s);
#endif

    return l;
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lia::getStringSMTlibDatatype

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
std::string smtcheck_opensmt2t_lia::getStringSMTlibDatatype(const typet& type)
{ 
    if ((type.id()==ID_bool) || (type.id() == ID_c_bool))
        return SMT_BOOL;
    if (is_number(type))
        return SMT_REAL;
    
    return SMT_UNKNOWN; // Shall not get here 
}

/*******************************************************************\

Function: smtcheck_opensmt2t_lia::getSMTlibDatatype

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/
SRef smtcheck_opensmt2t_lia::getSMTlibDatatype(const typet& type)
{ 
    if ((type.id()==ID_bool) || (type.id() == ID_c_bool))
        return lalogic->getSort_bool(); //SMT_BOOL
    if (is_number(type))
        return lalogic->getSort_num(); // SMT_INT

    throw std::logic_error("Unknown datatype encountered!");
}