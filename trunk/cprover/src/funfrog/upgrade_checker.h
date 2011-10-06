/*******************************************************************

 Module: Upgrade checker using function summaries

 Author: Ondrej Sery

\*******************************************************************/

#ifndef CPROVER_UPGRADE_CHECKER_H
#define	CPROVER_UPGRADE_CHECKER_H

#include "summarizing_checker.h"

class upgrade_checkert : summarizing_checkert
{
public:
  upgrade_checkert(
    const goto_programt &_goto_program,
    const value_setst &_value_sets,
    const goto_functionst &_goto_functions,
    const loopstoret &_imprecise_loops,
    const loopstoret &_precise_loops,
    const namespacet &_ns,
    contextt &_context,
    const optionst& _options,
    std::ostream &_out,
    unsigned long &_max_memory_used
    ) : summarizing_checkert (_goto_program, _value_sets, _goto_functions,
    _imprecise_loops, _precise_loops, _ns, _context, _options, _out,
    _max_memory_used)
  {};
  
  bool check_initial();
  bool check_upgrade();

protected:

};

bool check_initial(const namespacet &ns,
  goto_programt &program,
  const goto_functionst &goto_functions,
  const optionst& options,
  bool show_progress = true);

bool check_upgrade(const namespacet &ns,
  goto_programt &program_old,
  const goto_functionst &goto_functions_old,
  goto_programt &program_new,
  const goto_functionst &goto_functions_new,
  const optionst& options,
  bool show_progress = true);

#endif