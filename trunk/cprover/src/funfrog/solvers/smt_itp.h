#ifndef SMT_ITP_H
#define SMT_ITP_H

#include <ostream>
#include <util/std_expr.h>
#include <solvers/prop/literal.h>
#include <solvers/flattening/boolbv.h>

#include <opensmt/opensmt2.h>
#include <opensmt/Tterm.h>
#include "itp.h"

class smt_itpt: public itpt
{
public:
  smt_itpt() = default;
  ~smt_itpt() override = default;

  virtual  bool is_trivial() const override { return false; }

  virtual void print(std::ostream& out) const override;

  void setLogic(Logic *_l) { logic = _l; }

  Tterm & getTempl() {return templ;}

  static void reserve_variables(prop_conv_solvert& decider,
    const std::vector<symbol_exprt>& symbols, std::map<symbol_exprt, std::vector<unsigned> >& symbol_vars);

  void generalize(const prop_conv_solvert& mapping,
    const std::vector<symbol_exprt>& symbols);

//  void substitute(smtcheck_opensmt2t& decider,
//    const std::vector<symbol_exprt>& symbols,
//    bool inverted = false) const;

  virtual literalt raw_assert(propt& decider) const override;

  // Serialization
  virtual void serialize(std::ostream& out) const override;
  virtual void deserialize(std::istream& in) override;

  virtual bool usesVar(symbol_exprt&, unsigned) override;

  virtual bool check_implies(const itpt& second) const override { return false;}
  
  virtual itpt* get_nodet() override { return new smt_itpt(); }

protected:
  typedef std::vector<bvt> clausest;

  // Clauses of the interpolant representation
  clausest clauses;

  // TODO: figure out better way how to store the interpolants
  Tterm templ;

  Logic *logic;

    // Mask for used symbols
  std::vector<bool> symbol_mask;

};

typedef smt_itpt smt_interpolantt;
#endif
