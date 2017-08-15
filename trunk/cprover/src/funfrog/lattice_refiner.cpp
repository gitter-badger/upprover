/* 
 * File:   lattice_refinert.cpp
 * Author: karinek
 * 
 * Created on 18 July 2017, 14:21
 */

#include "lattice_refiner.h"
#include <list>

void lattice_refinert::initialize() {
    if (!is_lattice_ref_on) 
        return; // Not active if the user didn't add a model
    
    // Read the model from file
    load_models(options.get_option("load-sum-model"));
}

/*******************************************************************

 Function: lattice_refinert::load_models

 Inputs: 

 Outputs: 

 Purpose: Load all the models (according to input --load-sum-model)

\*******************************************************************/
void lattice_refinert::load_models(std::string list_of_models_fs) {

    // Supports many model-files for lattice refinement (will create several models)
    factory_lattice_refiner_modelt factory;
    std::list<std::string> model_files;
    factory.split(model_files, list_of_models_fs, ",");
    for(auto it = model_files.begin(); it != model_files.end() ; ++it)
    {
        models.insert(factory.load_model(*it));
    }
}

/*******************************************************************

 Function: lattice_refinert::can_refine

 Inputs: 

 Outputs: true if we can try to refine an instruction

 Purpose: Check if there is any instruction to refine - only if possible

\*******************************************************************/
bool lattice_refinert::can_refine(const symex_assertion_sumt& symex) const
{
    if (!is_lattice_ref_on)
        return false;
    if (!decider.has_unsupported_info() && !symex.has_missing_decl_func2refine())
        return false; // Exit, no refinement is needed! (no flag on or no abstractions done)
        
    return true;
}

/*******************************************************************

 Function: lattice_refinert::get_refined_functions_size

 Inputs: 

 Outputs: how many functions are in the queue to be refined

 Purpose: 

\*******************************************************************/
unsigned int lattice_refinert::get_refined_functions_size( 
        const symex_assertion_sumt& symex, bool is_first_iteration){ 
    if (!can_refine(symex))
        return 0;
    else if (is_first_iteration)
        return 1;
    
    if (refineTryNum > 10)
        return 0; // Debug mode
    
    int size_total = expr2refine.size(); 
    for (auto it : expr2refine) {
        if (it->is_SAT() || it->is_UNSAT()) size_total--;
    } // If we have an answer, sat or unsat it is one less to refine.
       
    return size_total;
}

/*******************************************************************

 Function: lattice_refinert::refine

 Inputs: Decider's unsupported expressions and symex abstracted functions

 Outputs: 

 Process: Push at arbitrary order all exprt to refine to some set
 * Note: If we have a better huristics to do so, we can add it here

 Purpose: Select which expression to refine, and will store the data 
 * for later, when we will do the actual refinement in refine_SSA

\*******************************************************************/
void lattice_refinert::refine(smt_partitioning_target_equationt &equation,
              symex_assertion_sumt& symex)
{
    // Shall we refine?
    if (!can_refine(symex))
        return;
    
    // Start a new cycle of refinement
    ++refineTryNum;
    
    #ifdef DEBUG_LATTICE 
    status () << "Start refinement with " << models.size() 
                << " lattice model(s), cycle(" << refineTryNum << ")." << eom;
    #endif

    // Add all expr to refine to the table (all candidates)
    add_expr_to_refine(symex); 
    
    // Pick one to refine
    set_front_heuristic();
    
} // End this cycle of refinement

/*******************************************************************

 Function: lattice_refinert::add_expr_to_refine

 Inputs: all the candidate expressions to refine

 Outputs: add all of them to the table expr2refine

 Purpose: 

\*******************************************************************/
void lattice_refinert::add_expr_to_refine(symex_assertion_sumt& symex) {
    if (refineTryNum > 1) return; // TODO: keep a list of which expr to refine are already in expr2refine
    
    // Refine functions abstracted by the solver (no OpenSMT support)
    const map<PTRef,exprt>::const_iterator begin = decider.get_itr_unsupported_info_map();
    const map<PTRef,exprt>::const_iterator end = decider.get_itr_end_unsupported_info_map();
    for (auto it = begin; it != end; it++) {
        const exprt &call_info = it->second; // function and arguments
        const PTRef lhs = it->first; // lhs
        const exprt::operandst &call_info_operands = call_info.operands(); 
        std::string key_entry = gen_entry_point_name(call_info.id().c_str(), call_info, call_info_operands);

        // if function has a definition, refine and add the refined term to a new partition
        if (get_entry_point(key_entry, call_info, call_info_operands) != SymRef_Undef) {
            // ADD to the list to refine such as lhs = refine(key_entry, call_info);
            exprt temp; temp.make_true();
            expr2refine.insert(new lattice_refiner_exprt(models.at(key_entry), temp, lhs, call_info_operands, key_entry));
        }
    }
    
    // Refine functions abstracted by the SSA translation (no function body)
    const map<exprt,pair<irep_idt, code_function_callt::argumentst>>::const_iterator 
            begin_symex = symex.get_itr_nobody_func_info_map();
    const map<exprt,pair<irep_idt, code_function_callt::argumentst>>::const_iterator 
            end_symex = symex.get_itr_end_nobody_func_info_map();
    for (auto it = begin_symex; it != end_symex; it++) {
        const pair<irep_idt, code_function_callt::argumentst> &call_info = it->second; // function and arguments
        const exprt &lhs = it->first; // lhs
        const exprt::operandst &call_info_operands = call_info.second; 
        std::string key_entry = gen_entry_point_name(call_info.first.c_str(), lhs, call_info_operands);

        // if function has a definition, refine and add the refined term to a new partition
        if (get_entry_point(key_entry, lhs, call_info_operands) != SymRef_Undef) {
            // ADD to the list to refine, such as lhs = refine(key_entry, call_info);
            expr2refine.insert(new lattice_refiner_exprt(models.at(key_entry), lhs, decider.getLogic()->getTerm_true(), call_info_operands, key_entry));
        }
    }    
}

/*******************************************************************

 Function: lattice_refinert::get_entry_point

 Inputs: the name, in and out parameters of the original SSA expression 
 * we wish to refine

 Outputs: literal of the entry point
 * e.g. (declare-fun |_mod#0| (UReal UReal) UReal)

 Purpose: Add the entry point so the SSA translation can add
 * the summaries related to it - or the added one will be with meaning

\*******************************************************************/
SymRef lattice_refinert::get_entry_point(
                const std::string key_entry, 
                const exprt &expr, 
                const exprt::operandst &operands)
{
    assert(models.size() > 0); // No meaning if there are no models
    
    // Check against a map
    if (declare2literal.count(key_entry) > 0) {
      return declare2literal.at(key_entry);
    }
    
    // If not in the map create it, add to the map and return it
    SymRef decl_func = SymRef_Undef;
    if (models.find(key_entry) != models.end()) {
      // Got at least one model!
      SRef in =  decider.getSMTlibDatatype(expr);
      vec<SRef> args;
      for (auto it : operands) {
        args.push(decider.getSMTlibDatatype(it));
      }
      decl_func = decider.get_smt_func_decl(key_entry.c_str(), in, args);
      declare2literal.insert(pair<string, SymRef> (key_entry, decl_func));
    }

    return decl_func;
}

/*******************************************************************

 Function: lattice_refinert::gen_entry_point_name

 Inputs: id of the function call, in parameter, out parameter

 Outputs: string with the function decl - name + operands + data types
 * e.g. (declare-fun |_mod#0| (UReal UReal) UReal) or
 * (declare-fun |_xor#0| (Bool Bool) Bool)

 Purpose: Get the key for searching entry model, note that we can have
 * two lattices for two different data type (inputs or output)

\*******************************************************************/
std::string lattice_refinert::gen_entry_point_name(
                const std::string key_entry_orig, 
                const exprt &expr, 
                const exprt::operandst &operands)
{    
    std::string key_entry = "(declare-fun |_" + key_entry_orig + "#0| (";
    
    for (auto it : operands) {
        key_entry += decider.getStringSMTlibDatatype(it) + " ";
    }
    
    key_entry += ") " + decider.getStringSMTlibDatatype(expr) + ")";
    
    #ifdef DEBUG_LATTICE    
    status() << "Start processing the creation of an entry-point of the function " 
            << key_entry_orig << " with " << operands.size() << " operands. Function signature is " 
            << key_entry << ((declare2literal.count(key_entry) > 0) ? " exist" : " new") 
            << " in the map" << eom;
    #endif    
    
    return key_entry;
}
 
/*******************************************************************

 Function: lattice_refinert::refine_single_statement

 Inputs: current SSA to SMT-lib translation with its original SSA expression

 Outputs: ? maybe the new refined ptref?

 Purpose: Refine too abstract translation of the SSA to SMT-lib

\*******************************************************************/
literalt lattice_refinert::refine_single_statement(const exprt &expr, const PTRef var)
{
    status() << "Refine original translation " << decider.getPTermString(expr) 
            << " of " << expr.id() << " with " << expr.operands().size() << " operands" << eom;
    
    
    // Get next entry of refined functions
    //lattice_refiner_modelt *curr_node = get_refine_function(expr);
    
    // Create a new PTRef which refine the original expression
    PTRef refined_var; // will add a call to the refined func, e.g., mod_C3(a,n)
    //forall_operands(it, expr) {
    //    literalt param_in = decider.convert(expr);
    //    literalt arg_in;
    //   literalt bind_param = decider.set_equal(param_in, arg_in);
    //    decider.land(bind_param);
    //}
    
    // Return (= var refined_var)
    return decider.bind_var2refined_var(var, refined_var);
}

/*******************************************************************

 Function: lattice_refinert::process_SAT_result

 Inputs: 

 Outputs: 

 Purpose: if all reach to top - SAT, if SAT but not top, try the childs
 * of the current node. If reached to top (in all paths) the expression 
 * is removed from the refined data (since it cannot be refined).
 * 
 * Going down the lattice
 * 
 * If SAT according to the model - return true, else false

\*******************************************************************/
bool lattice_refinert::process_SAT_result() {  
    bool ret = false;
    for (auto it : expr2refine) {
        it->process_SAT_result();
        ret = ret || it->is_SAT();
    }
       
    return ret;
}

/*******************************************************************

 Function: lattice_refinert::process_UNSAT_result

 Inputs: 

 Outputs: 

 Purpose: change the state of the model only - the SSA changes will 
 * happen in refine_SSA, not here.
 * 
 * Going backward

\*******************************************************************/
bool lattice_refinert::process_UNSAT_result() {
    bool ret = true;
    for (auto it : expr2refine) {
        it->process_UNSAT_result();
        ret = ret && it->is_UNSAT();
    }
       
    return ret;
}

/*******************************************************************

 Function: lattice_refinert::refine_SSA

 Inputs: 

 Outputs: false if we shall continue and refining, true else.

 Purpose: move down/up in the lattice

\*******************************************************************/
bool lattice_refinert::process_solver_result(bool is_solver_ret_SAT) {
    if (is_solver_ret_SAT) {
        return process_SAT_result(); // return true if SAT
    } else {
        return process_UNSAT_result(); // return true if UNSAT
    }
    // Both will return false if there is no decision yet
}

/*******************************************************************

 Function: lattice_refinert::refine_SSA

 Inputs: the last result of the query to the solver, decider and symex objects

 Outputs: 

 Purpose: Add the needed SSA instructions - refine
 * 
 * main refine, add the smt side, and it is also where we shall
 * use the lattice model to refine the code
 * 
 * Here we do: unsupported#20 = "call of the set of functions that refines it"
 * The extract of the functions (which are summaries) is done not here
 * but in refine_SSA

\*******************************************************************/
bool lattice_refinert::refine_SSA(
            const smtcheck_opensmt2t &decider, 
            symex_assertion_sumt& symex) 
{
    // Shall we refine?
    if (!can_refine(symex))
        return true;
    
    // Keep all the expression we can refine, which we didn't yet kept
    ///////////////////////////////////////////////////////////////////    
    
    // 1. from the solver side
    //const map<PTRef,exprt>::const_iterator begin = decider.get_itr_unsupported_info_map();
    //const map<PTRef,exprt>::const_iterator end = decider.get_itr_end_unsupported_info_map();
    //for (auto it = begin; it != end; it++) {   
        // if function has a definition, refine and add the refined term to a new partition
    //    if (get_entry_point(it->second) != SymRef_Undef) {
    //      decider.new_partition();  
    //      decider.set_to_true(refine_single_statement(it->second, it->first));
          
    //      decider.close_partition(); 
          //close the partition (but will solve later, after refine_SSA)
    //    }
    //}
    
    
    // TODO:
    // Else change the encoding, maybe only to add new partitions? KE
    
    
    return false;
}