#include "hifrog.h"
#include <string.h>
#include <algorithm>
//#include <iostream> // Comment only for debug

/* Get the name of an SSA expression without the instance number 
 *    
 * KE: please use it, as cprover framework keeps changing all the time
 */
irep_idt get_symbol_name(const exprt &expr) {
    //std::cout << "Get symbol name called for:\n" << expr.pretty() << '\n';
    return to_ssa_expr(expr).get_original_name();
}
//
//
//bool is_hifrog_inner_symbol_name(const exprt &expr) {
//    std::string test4inned_hifrog = id2string(expr.get(ID_identifier));
//    if (test4inned_hifrog.find(CALLSTART_SYMBOL) != std::string::npos)
//        return true;
//    if (test4inned_hifrog.find(CALLEND_SYMBOL) != std::string::npos)
//        return true;
//    if (test4inned_hifrog.find(ERROR_SYMBOL) != std::string::npos)
//        return true;
//
//    return false;
//}
//
//irep_idt extract_hifrog_inner_symbol_name(const exprt &expr){
//    std::string test4inned_hifrog = id2string(expr.get(ID_identifier));
//    if (test4inned_hifrog.find(CALLSTART_SYMBOL) != std::string::npos)
//        return CALLSTART_SYMBOL;
//    if (test4inned_hifrog.find(CALLEND_SYMBOL) != std::string::npos)
//        return CALLEND_SYMBOL;
//    if (test4inned_hifrog.find(ERROR_SYMBOL) != std::string::npos)
//        return ERROR_SYMBOL;
//
////    assert(0); // Add constants if needed
//    throw std::logic_error("Unknown symbol encountered!");
//}
//
//unsigned get_symbol_L2_counter(const exprt &expr) {
//    if (is_hifrog_inner_symbol_name(expr))
//        return extract_hifrog_inner_symbol_L2_counter(expr);
//
//    return atoi(to_ssa_expr(expr).get_level_2().c_str());
//}
//
//unsigned extract_hifrog_inner_symbol_L2_counter(const exprt &expr){
//    std::string test4inned_hifrog = id2string(expr.get(ID_identifier));
//    size_t pos = extract_hifrog_inner_symbol_name(expr).size();
//    if ((test4inned_hifrog.find(CALLSTART_SYMBOL) != std::string::npos) ||
//        (test4inned_hifrog.find(CALLEND_SYMBOL) != std::string::npos) ||
//        (test4inned_hifrog.find(ERROR_SYMBOL) != std::string::npos))
//        return atoi(test4inned_hifrog.substr(pos+1).c_str());
//
//    throw std::logic_error("Unknown symbol encountered!");
//    //assert(0); // Add constants if needed
//}

/* Assure the name is always symex::nondet#number */
std::string fix_symex_nondet_name(const exprt &expr) {
    // Fix Variable name - sometimes "nondet" name is missing, add it for these cases
    
    std::string name_expr = id2string(expr.get(ID_identifier));
    assert (name_expr.size() != 0); // Check the we really got something
    if (expr.id() == ID_nondet_symbol)
    {
        if (name_expr.find(NONDETv2) != std::string::npos) {
            name_expr = name_expr.insert(13,1, '#');
        } else if (name_expr.find(NONDETv1) != std::string::npos) {
            name_expr = name_expr.insert(7, SYMEX_NONDET);
        }  
    }
    
    return name_expr;
}

// Get a unique index per query's dump
unsigned int get_dump_current_index()
{
    static unsigned int index=0;
    index+=1;
    return index;
}

// Test if name is without L2 level
bool is_L2_SSA_symbol(const exprt& expr)
{
    if (expr.id() == ID_nondet_symbol)
        return true; // KE: need to be tested!
    
    // Else check program symbols
    if (expr.id()!=ID_symbol)
        return false;
    if (!expr.get_bool(ID_C_SSA_symbol))
        return false;
    if (expr.has_operands())
        return false;
    if (expr.get(ID_identifier) == get_symbol_name(expr)){
        return false;
    }else if (expr.get(ID_identifier) == get_symbol_L1_name(expr)){
        return false;
    }
    
    return true;
}
