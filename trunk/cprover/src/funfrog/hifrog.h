/* 
 * File:   hifrog.h
 * Author: karinek
 *
 * Created on 14 July 2017, 15:55
 * 
 * All constants of HiFrog 
 * And all things related to the interface to cprover framework
 */

#ifndef HIFROG_H
#define HIFROG_H

#include <ssa_expr.h>

// For now we have only one thread any hows
#define FUNC_RETURN "::#return_value!0"  // KE: appears in Cprover as "#return_value"
#define TMP_FUNC_RETURN "::$tmp::return_value!0"
#define OPENSMT_IN "#in"
#define OPENSMT_OUT "#out"
#define OPENSMT_INVS "#invs"

#define CALLSTART_SYMBOL "hifrog::fun_start"
#define CALLEND_SYMBOL "hifrog::fun_end"
#define ERROR_SYMBOL "hifrog::?err"

#define CPROVER_BUILDINS "__CPROVER_"
#define ROUNDING_MODE "__CPROVER_rounding_mode!"
#define INITIALIZE "__CPROVER_initialize"
#define DYNAMIC_OBJ "symex_dynamic::dynamic_object"
#define GOTO_GUARD "goto_symex::\\guard#"

#define NIL "nil"
#define NONDETv1 "symex::" // Cprover nondet symbol
#define NONDETv2 "symex::nondet" // Cprover nonder symbol too
#define COUNTER '#' // GOTO to SSA (e.g., hifrog::?fun_end to hifrog::?fun_end#1)
#define SYMEX_NONDET "nondet#" //"symex::nondet#" - fix to
#define IO_CONST "symex::io::" // Update according to goto_symex/symex_target_equation

// SMT consts of datatypes, add/change here only if needed
#define SMT_BOOL "Bool"
#define SMT_REAL "Real"
#define SMT_UREAL "UReal"
#define SMT_UNKNOWN "?"

irep_idt get_symbol_name(const exprt &expr);
bool is_hifrog_inner_symbol_name(const exprt &expr);
irep_idt extract_hifrog_inner_symbol_name(const exprt &expr);
unsigned get_symbol_L2_counter(const exprt &expr);
unsigned extract_hifrog_inner_symbol_L2_counter(const exprt &expr);
std::string fix_symex_nondet_name(const exprt &expr);

#endif /* HIFROG_H */
