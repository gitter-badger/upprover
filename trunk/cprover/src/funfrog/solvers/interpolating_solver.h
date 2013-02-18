/*******************************************************************\

Module: Interface for a decision procedure with (symmetric) 
interpolation.

Author: Ondrej Sery

\*******************************************************************/

#ifndef CPROVER_INTERPOLATING_SOLVER_H
#define CPROVER_INTERPOLATING_SOLVER_H

#include <set>

#include <decision_procedure.h>

#include "prop_itp.h"

//#define DEBUG_COLOR_ITP
#define PRODUCE_PROOF
#include "Global.h"


typedef int fle_part_idt;
typedef std::vector<fle_part_idt> fle_part_idst;
typedef std::vector<fle_part_idst> interpolation_taskt;

class interpolating_solvert
{
public:
  virtual ~interpolating_solvert() {};

  // Begins a partition of formula for latter reference during 
  // interpolation extraction. All assertions made until
  // next call of new_partition() will be part of this partition.
  //
  // returns a unique partition id
  virtual fle_part_idt new_partition()=0;

  // Extracts the symmetric interpolant of the specified set of
  // partitions. This method can be called only after solving the
  // the formula with an UNSAT result
  virtual void get_interpolant(const interpolation_taskt& partition_ids,
      interpolantst& interpolants,
      double reduction_timeout, int reduction_loops, int reduction_graph)=0;
  
  virtual void get_interpolant(opensmt::InterpolationTree*, const interpolation_taskt& partition_ids,
    interpolantst& interpolants)=0;

  // Is the solver ready for interpolation? I.e., the solver was used to decide
  // a problem and the result was UNSAT
  virtual bool can_interpolate() const=0;

  virtual void addAB(const std::vector<unsigned>& symbolsA, const std::vector<unsigned>& symbolsB,
      const std::vector<unsigned>& symbolsAB)=0;

# ifdef DEBUG_COLOR_ITP
  virtual std::vector<unsigned>& get_itp_symb(unsigned i)=0;
# endif

};

#endif
