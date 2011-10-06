/*******************************************************************

 Module: Upgrade checker using function summaries.

 Author: Ondrej Sery

\*******************************************************************/

#include "upgrade_checker.h"

/*******************************************************************\

Function: check_initial

  Inputs:

 Outputs:

 Purpose: Check the whole system and prepare for incremental 
 check of upgrades via check_upgrade.

\*******************************************************************/

bool check_initial(const namespacet &ns,
  goto_programt &program,
  const goto_functionst &goto_functions,
  const optionst& options,
  bool show_progress)
{
  unsigned long max_mem_used;
  contextt temp_context;
  namespacet ns1(ns.get_context(), temp_context);
  summarizing_checkert sum_checker(program, value_set_analysist(ns1),
                         goto_functions, loopstoret(), loopstoret(),
                         ns1, temp_context, options, std::cout, max_mem_used);

  sum_checker.initialize();

  if(show_progress)
  {
    std::cout << std::endl << "    Checking all claims" << std::endl;
    std::cout.flush();
  }

  // Check all the assertions
  bool result = sum_checker.assertion_holds(assertion_infot());
  
  // TODO: Serialize the information needed for incremental verification.
  assert(false);
  
  return result;
}

/*******************************************************************\

Function: check_upgrade

  Inputs:

 Outputs:

 Purpose: Incremental check of the upgraded program.

\*******************************************************************/
bool check_upgrade(const namespacet &ns,
  goto_programt &program_old,
  const goto_functionst &goto_functions_old,
  goto_programt &program_new,
  const goto_functionst &goto_functions_new,
  const optionst& options,
  bool show_progress) 
{
  // 1. Make diff
  // 2. Construct changed summary_info tree
  // 3. Mark summaries as
  //     - valid: the function was not changed
  //     - invalid: interface change / ass_in_subtree change
  //     - unknown: function body changed
  // 3. From the bottom of the tree, reverify all changed nodes
  //    a. If the edge is unchanged, check implication of previously 
  //       used summaries
  //        - OK/already valid: summary valid, don't propagate check upwards
  //        - KO/already invalid: summary invalid, propagate check upwards
  //    b. If the edge is changed, propagate check upwards (we don't know which summary 
  //       to check).
  
  assert(false);
  return false;
}