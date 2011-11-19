/*******************************************************************
 Module: Refining schema for function summarization.
         Supposed to have different implementations.

 Author: Grigory Fedyukovich

 \*******************************************************************/

#ifndef CPROVER_REFINER_ASSERTION_SUM_H
#define CPROVER_REFINER_ASSERTION_SUM_H

#include "assertion_info.h"
#include "subst_scenario.h"
#include "summary_info.h"
#include "summarization_context.h"
#include "partitioning_target_equation.h"

class refiner_assertion_sumt
{
public:
  refiner_assertion_sumt(
          summarization_contextt &_summarization_context,
          subst_scenariot &_omega,
          partitioning_target_equationt &_target,
          refinement_modet _mode,
          std::ostream &_out,
          const unsigned _last_assertion_loc,
          bool _valid
          ) :
          summarization_context(_summarization_context),
          omega(_omega),
          summs(omega.get_call_summaries()),
          equation(_target),
          mode(_mode),
          out(_out),
          last_assertion_loc(_last_assertion_loc),
          valid (_valid)
          {};

  void refine(prop_convt& decider);
  std::list<summary_infot*>& get_refined_functions(){ return refined_functions; }
  void set_refine_mode(refinement_modet _mode){ mode = _mode; }

protected:
  // Shared information about the program and summaries to be used during
  // analysis
  summarization_contextt &summarization_context;

  // substituting scenario
  subst_scenariot &omega;

  // Which functions should be summarized, abstracted from, and which inlined
  std::vector<summary_infot*>& summs;

  // Store for the symex result
  partitioning_target_equationt &equation;

  // Mode of refinement
  refinement_modet mode;

  // Default output
  std::ostream &out;

  // Location of the last assertion to be checked
  const unsigned last_assertion_loc;

  // Mode of changing the summaries validity
  bool valid;

  std::list<summary_infot*> refined_functions;

  void reset_inline();
  void reset_random();
  void reset_depend(prop_convt& decider, bool do_callstart = true);

  void set_inline_sum(int i);
};

#endif
