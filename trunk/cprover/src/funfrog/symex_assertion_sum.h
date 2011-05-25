/*******************************************************************
 Module: Symbolic execution and deciding of a given goto-program
 using and generating function summaries. Based on symex_asserion.h.

 Author: Ondrej Sery

 \*******************************************************************/

#ifndef CPROVER_SYMEX_ASSERTION_SUM_H
#define CPROVER_SYMEX_ASSERTION_SUM_H

#include <queue>

#include <goto-programs/goto_program.h>
#include <goto-programs/goto_functions.h>
#include <goto-symex/goto_symex.h>
#include <cbmc/symex_bmc.h>
#include <namespace.h>
#include <symbol.h>

#include <base_type.h>
#include <time_stopping.h>

#include "assertion_info.h"
#include "summary_info.h"
#include "summarization_context.h"
#include "partitioning_target_equation.h"

class symex_assertion_sumt : public symex_bmct //goto_symext
{
public:
  symex_assertion_sumt(
          summarization_contextt &_summarization_context,
          const summary_infot &_summary_info,
          const namespacet &_ns,
          contextt &_context,
          partitioning_target_equationt &_target,
          std::ostream &_out,
          const goto_programt &_goto_program,
          bool _use_slicing=true
          ) :
          // goto_symext(_ns, _context, _target),
          symex_bmct(_ns, _context, _target),
          summarization_context(_summarization_context),
          summary_info(_summary_info),
          current_summary_info(&_summary_info),
          equation(_target),
          current_assertion(NULL),
          out(_out),
          goto_program(_goto_program),
          use_slicing(_use_slicing)
          {};

  void loop_free_check();

  bool prepare_SSA(const assertion_infot &assertion);
  
  virtual void symex_step(
    const goto_functionst &goto_functions,
    statet &state);

  unsigned sum_count;

private:

  class deferred_functiont {
  public:

    deferred_functiont(const summary_infot &_summary_info) :
            summary_info(_summary_info), 
            callstart_symbol(typet(ID_bool)),
            callend_symbol(typet(ID_bool)),
            returns_value(false),
            partition_id(partitioning_target_equationt::NO_PARTITION),
            assert_stack_match(false)
            {
    }

    const summary_infot& summary_info;
    // TODO: Deprecate it! Split into iface vars and in_arg_symbols
    std::vector<symbol_exprt> argument_symbols;
    std::vector<symbol_exprt> in_arg_symbols;
    std::vector<symbol_exprt> out_arg_symbols;
    symbol_exprt retval_symbol;
    symbol_exprt retval_tmp;
    symbol_exprt callstart_symbol;
    symbol_exprt callend_symbol;
    bool returns_value;
    partition_idt partition_id;
    call_stackt::const_iterator assert_stack_it;
    bool assert_stack_match;
  };

  // Shared information about the program and summaries to be used during
  // analysis
  summarization_contextt &summarization_context;

  // Which functions should be summarized, abstracted from, and which inlined
  const summary_infot &summary_info;

  // Summary info of the function being currently processed. Set to NULL when
  // no deferred function are left
  const summary_infot *current_summary_info;

  // Wait queue for the deferred functions (for other partitions)
  std::queue<deferred_functiont> deferred_functions;

  // Store for the symex result
  partitioning_target_equationt &equation;

  // Artificial identifiers for which we do not need Phi function
  std::set<irep_idt> dead_identifiers;

  // Current assertion
  const assertion_infot* current_assertion;

  std::ostream &out;

  const goto_programt &goto_program;


  bool use_slicing;

  // Add function to the wait queue to be processed by symex later and to
  // create a separate partition for interpolation
  void defer_function(const deferred_functiont &deferred_function, 
    irep_idt function_id);

  // Are there any more instructions in the current function or at least
  // a deferred function to dequeue?
  bool has_more_steps(const statet &state) {
    return current_summary_info != NULL;
  }

  // Take a deferred function from the queue and prepare it for symex
  // processing. This would also mark a corresponding partition in
  // the target equation.
  void dequeue_deferred_function(statet &state);

  // The currently processed deferred function
  const deferred_functiont& get_current_deferred_function() const {
    return deferred_functions.front();
  }

  // Processes a function call based on the corresponding
  // summary type
  void handle_function_call(statet &state,
    code_function_callt &function_call);

  // Summarizes the given function call
  void summarize_function_call(
        deferred_functiont& deferred_function,
        statet& state,
        const irep_idt& function_id);

  // Inlines the given function call
  void inline_function_call(
        deferred_functiont& deferred_function,
        statet& state,
        const irep_idt& function_id);

  // Abstract from the given function call (nondeterministic assignment to
  // all the possibly modified variables)
  void havoc_function_call(
        deferred_functiont& deferred_function,
        statet& state,
        const irep_idt& function_id);

  // Assigns function arguments to new SSA symbols, also makes
  // assignment of the new SSA symbol of return value to the lhs of
  // the call site (if any)
  void assign_function_arguments(statet &state,
    code_function_callt &function_call,
    deferred_functiont &deferred_function);
  
  // Marks the SSA symbols of function arguments
  void mark_argument_symbols(
    const code_typet &function_type,
    statet &state,
    deferred_functiont &deferred_function);

  // Marks the SSA symbols of accessed globals
  void mark_accessed_global_symbols(
    const irep_idt &function_id,
    statet &state,
    deferred_functiont &deferred_function);

  // Assigns values from the modified global variables. Marks the SSA symbol 
  // of the global variables for later use when processing the deferred function
  void modified_globals_assignment_and_mark(
    const irep_idt &function_id,
    statet &state,
    deferred_functiont &deferred_function);

  // Assigns return value from a new SSA symbols to the lhs at
  // call site. Marks the SSA symbol of the return value temporary
  // variable for later use when processing the deferred function
  void return_assignment_and_mark(
    const code_typet &function_type,
    statet &state,
    const exprt &lhs,
    deferred_functiont &deferred_function);

  // Assigns modified globals to the corresponding temporary SSA symbols
  void store_modified_globals(
    statet &state,
    const deferred_functiont &deferred_function);

  // Assigns return value to the corresponding temporary SSA symbol
  void store_return_value(
    statet &state,
    const deferred_functiont &deferred_function);

  // Clear local symbols from the l2 cache.
  void clear_locals_versions(statet &state);
  
  // Creates new call site (start & end) symbols for the given
  // deferred function
  void produce_callsite_symbols(deferred_functiont& deferred_function,
    statet& state,
    const irep_idt& function_id);

  // Inserts assumption that a given call ended (i.e., an assumption of
  // the callend symbol)
  void produce_callend_assumption(
        const deferred_functiont& deferred_function, statet& state);

  // Helper function for renaming of an identifier without
  // assigning to it. Constant propagation is stopped for the given symbol.
  irep_idt get_new_symbol_version(
        const irep_idt& identifier,
        statet &state);

  // Makes an assignment without increasing the version of the
  // lhs symbol (make sure that lhs symbol is not assigned elsewhere)
  void raw_assignment(
        statet &state,
        exprt &lhs,
        const exprt &rhs,
        const namespacet &ns,
        bool record_value);

  // Adds the given symbol to the current context. If dead, the identifier
  // is only marked as dead (it is not added as a new symbol).
  void add_symbol(const irep_idt& id, const typet& type, bool dead) {
    if (dead) {
      dead_identifiers.insert(id);
    } else if (!new_context.has_symbol(id)) {
      symbolt s;
      s.base_name = id;
      s.name = id;
      s.type = type;
      new_context.add(s);
    }
  }

  // Dead identifiers do not need to be considered in Phi function generation
  bool is_dead_identifier(const irep_idt& identifier) {
    if (identifier == guard_identifier)
      return true;

    return dead_identifiers.find(identifier) != dead_identifiers.end();
  }

protected:
  virtual void phi_function(
    const statet::goto_statet &goto_state,
    statet &state);
};
#endif
