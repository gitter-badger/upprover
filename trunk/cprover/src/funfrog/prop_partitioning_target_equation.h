/*******************************************************************\

Module: Symex target equation which tracks different partitions for
different deferred functions.

Author: Ondrej Sery

\*******************************************************************/

#ifndef CPROVER_PROP_PARTITIONING_TARGET_EQUATION_H
#define CPROVER_PROP_PARTITIONING_TARGET_EQUATION_H

#include "partitioning_target_equation.h"
#include "solvers/satcheck_opensmt2.h"
#include "partition_iface_fwd.h"


// Two classes for smt and prop   
class prop_partitioning_target_equationt:public partitioning_target_equationt
{
public:
  prop_partitioning_target_equationt(const namespacet &_ns, summary_storet & store,
          bool _store_summaries_with_assertion
  )
            : partitioning_target_equationt(_ns, store,
                       _store_summaries_with_assertion
                       ) {}
            
  // Convert all the SSA steps into the corresponding formulas in
  // the corresponding partitions
  void convert(prop_conv_solvert &prop_conv, interpolating_solvert &interpolator);
  
  // Extract interpolants corresponding to the created partitions
  void extract_interpolants(
    interpolating_solvert& interpolator, const prop_conv_solvert& decider);

protected:
  // Convert a specific partition of SSA steps
  void convert_partition(prop_conv_solvert &prop_conv_solvert,
    interpolating_solvert &interpolator, partitiont& partition);
  // Convert a specific partition guards of SSA steps
  void convert_partition_guards(prop_conv_solvert &prop_conv,
    partitiont& partition);
  // Convert a specific partition assignments of SSA steps
  void convert_partition_assignments(prop_conv_solvert &prop_conv,
    partitiont& partition);
  // Convert a specific partition assumptions of SSA steps
  void convert_partition_assumptions(prop_conv_solvert &prop_conv,
    partitiont& partition);
  // Convert a specific partition assertions of SSA steps
  void convert_partition_assertions(prop_conv_solvert &prop_conv,
    partitiont& partition);
  // Convert a specific partition io of SSA steps
  void convert_partition_io(prop_conv_solvert &prop_conv,
    partitiont& partition);
  // Convert a summary partition (i.e., assert its summary)
  void convert_partition_summary(prop_conv_solvert &prop_conv,
    partitiont& partition);
  // Convert a specific partition gotos of SSA steps
  void convert_partition_goto_instructions(prop_conv_solvert &prop_conv,
    partitiont& partition);
  
  // Override
  virtual void fill_partition_ids(partition_idt partition_id, fle_part_idst& part_ids);
};

#endif