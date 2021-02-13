/*******************************************************************\

Module: Counterexample Beautification

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Counterexample Beautification

#ifndef CPROVER_CBMC_COUNTEREXAMPLE_BEAUTIFICATION_H
#define CPROVER_CBMC_COUNTEREXAMPLE_BEAUTIFICATION_H

#include <util/namespace.h>

#include <goto-symex/symex_target_equation.h>

#include <solvers/flattening/bv_minimize.h>

class counterexample_beautificationt
{
public:
  virtual ~counterexample_beautificationt()
  {
  }

  void
  operator()(boolbvt &boolbv, const symex_target_equationt &equation);

protected:
  void get_minimization_list(
    prop_convt &prop_conv,
    const symex_target_equationt &equation,
    minimization_listt &minimization_list);

  void minimize(
    const exprt &expr,
    class prop_minimizet &prop_minimize);

  symex_target_equationt::SSA_stepst::const_iterator get_failed_property(
    const prop_convt &prop_conv,
    const symex_target_equationt &equation);

  // the failed property
  symex_target_equationt::SSA_stepst::const_iterator failed;
};

#endif // CPROVER_CBMC_COUNTEREXAMPLE_BEAUTIFICATION_H
