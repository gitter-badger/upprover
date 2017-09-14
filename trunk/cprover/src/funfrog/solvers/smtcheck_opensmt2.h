/*******************************************************************\

Module: Wrapper for OpenSMT2

\*******************************************************************/

#ifndef CPROVER_SMTCHECK_OPENSMT2_H
#define CPROVER_SMTCHECK_OPENSMT2_H

//#define DEBUG_SMT4SOLVER // TO PRINT FROM HIFROG ENCODING + ITE DEF.

#include <map>
#include <vector>

#include <util/threeval.h>
#include "check_opensmt2.h"
#include "interpolating_solver.h"
#include "smt_itp.h"
#include <opensmt/opensmt2.h>
#include <expr.h>
#include "../hifrog.h"

// Cache of already visited interpolant literals
typedef std::map<PTRef, literalt> ptref_cachet;

class smtcheck_opensmt2t : public check_opensmt2t
{
public:
  // Defualt C'tor
  smtcheck_opensmt2t(bool _store_unsupported_info=false) :
      no_literals(0),
      pushed_formulas(0),
      is_var_constraints_empty(true),
      store_unsupported_info(_store_unsupported_info),
      check_opensmt2t(false, 3, 2) // Is last always!
  {
    /* No init of solver - done for inherit check_opensmt2 */
  }

  // C'tor to pass the value to main interface check_opensmt2
  smtcheck_opensmt2t(bool reduction, int reduction_graph, int reduction_loops, bool _store_unsupported_info=false) :
        no_literals(0),
        pushed_formulas(0),
        is_var_constraints_empty(true),
        store_unsupported_info(_store_unsupported_info),
        check_opensmt2t(reduction, reduction_graph, reduction_loops)
  { /* No init of solver - done for inherit check_opensmt2 */}
    
  virtual ~smtcheck_opensmt2t(); // d'tor

  virtual prop_conv_solvert* get_prop_conv_solver(){return NULL;} // Common to all

  bool solve(); // Common to all
  
  bool is_assignemt_true(literalt a) const; // Common to all

  virtual exprt get_value(const exprt &expr)=0;

  virtual literalt lassert_var() { assert(0);}

  bool is_exist_var_constraints() { return !is_var_constraints_empty;}

  virtual literalt convert(const exprt &expr) =0;

  void set_to_false(const exprt &expr); // Common to all
  void set_to_true(const exprt &expr); // Common to all
  void set_to_true(literalt refined_l); // Common to all
  void set_to_true(PTRef); // Common to all
  void set_equal(literalt l1, literalt l2); // Common to all

  PTRef convert_symbol(const exprt &expr); // Common to all 

  literalt const_var(bool val); // Common to all

  virtual literalt const_var_Real(const exprt &expr)=0;
  
  virtual literalt type_cast(const exprt &expr)=0;

  literalt limplies(literalt l1, literalt l2); // Common to all

  virtual literalt lnotequal(literalt l1, literalt l2)=0;

  literalt land(literalt l1, literalt l2); // Common to all

  literalt land(bvt b); // Common to all

  literalt lor(literalt l1, literalt l2); // Common to all

  literalt lor(bvt b); // Common to all

  literalt lnot(literalt l); // Common to all

  virtual literalt lvar(const exprt &expr)=0;

  literalt lconst(const exprt &expr); // Common to all

  fle_part_idt new_partition(); // Common to all

  void close_partition(); // Common to all

#ifdef PRODUCE_PROOF  
  void get_interpolant(const interpolation_taskt& partition_ids,
      interpolantst& interpolants); // Common to all

  bool can_interpolate() const; // Common to all

  // Extract interpolant form OpenSMT files/data
  void extract_itp(PTRef ptref, smt_itpt& target_itp) const; // Common to all
  
  void adjust_function(smt_itpt& itp, std::vector<symbol_exprt>& common_symbols, std::string fun_name, bool substitute = true); // Common to all
#endif
  
  static int get_index(const string& varname);
  static std::string insert_index(const string& varname, const string& idx); // Common to all
  static std::string insert_index(const string& varname, int idx); // Common to all
  static std::string quote_varname(const string& varname); // Common to all
  static std::string unquote_varname(const string& varname); // Common to all
  
  static std::string remove_index(std::string); // Common to all
  static std::string remove_invalid(const string& varname); // Common to all

  static bool is_quoted_var(const string& varname); // Common to all

  // Common to all
  void start_encoding_partitions() {
	  if (partition_count > 0){
#ifdef PRODUCE_PROOF              
		  if (ready_to_interpolate) cout << "EXIT WITH ERROR: Try using --claim parameter" << std::endl;
		  assert (!ready_to_interpolate); // GF: checking of previous assertion run was successful (unsat)
#endif		  	  	  	  	  	  	  	  	  // TODO: reset opensmt context

		  std::cout << "Incrementally adding partitions to the SMT solver\n";
	  }
  }

  /* The data: lhs, original function data */
  bool has_unsupported_info() const { return store_unsupported_info && has_unsupported_vars(); } // Common to all
  bool has_unsupported_vars() const { return (unsupported2var > 0); } // Common to all, affects several locations!
  string create_new_unsupported_var(); // Common to all
  map<PTRef,exprt>::const_iterator get_itr_unsupported_info_map() const { return unsupported_info_map.begin(); }
  map<PTRef,exprt>::const_iterator get_itr_end_unsupported_info_map() const { return unsupported_info_map.end(); }
  /* End of unsupported data for refinement info and data */
  
  
  static bool is_cprover_rounding_mode_var(const exprt& e)
  {
      return is_cprover_rounding_mode_var(id2string(e.get(ID_identifier)));
  }
  static bool is_cprover_rounding_mode_var(const std::string str)
  {
      return (str.find(ROUNDING_MODE) != std::string::npos);
  }
  static bool is_cprover_builtins_var(const exprt& e)
  {
      return is_cprover_builtins_var(id2string(e.get(ID_identifier)));
  }
  static bool is_cprover_builtins_var(const std::string str)
  {
      return (str.find(CPROVER_BUILDINS) != std::string::npos);
  }
  
  // Common to all
  std::set<PTRef>* getVars(); // Get all variables from literals for the counter example phase

  /* For unsupported var creation */
  static const string _unsupported_var_str;
  
public:
  literalt bind_var2refined_var(PTRef ptref_coarse, PTRef ptref_refined); // common to all
  
  SymRef get_smt_func_decl(const char* op, SRef& in_dt, vec<SRef>& out_dt); // common to all
  
  PTRef mkCustomFunction(SymRef decl, vec<PTRef>& args); // common to all
  
  virtual std::string getStringSMTlibDatatype(const exprt& expr)=0;
  virtual SRef getSMTlibDatatype(const exprt& expr)=0; 

  void init_unsupported_counter() { unsupported2var=0; } // KE: only for re-init solver use. Once we have pop in OpenSMT, please discard.
  
protected:
  
  vec<PTRef> top_level_formulas;

  bool is_var_constraints_empty;

  map<size_t, literalt> converted_exprs;

  unsigned no_literals;

  //  Mapping from boolean variable indexes to their PTRefs
  std::vector<PTRef> literals;
  typedef std::vector<PTRef>::iterator it_literals;

  unsigned pushed_formulas;

  static unsigned unsupported2var; // Create a new var hifrog::c::unsupported_op2var#i - smtcheck_opensmt2t::_unsupported_var_str
  bool store_unsupported_info;
  map<PTRef,exprt> unsupported_info_map;
  
  literalt store_new_unsupported_var(const exprt& expr, const PTRef var); // common to all 
  
  virtual literalt lunsupported2var(const exprt &expr)=0; // for isnan, mod, arrays ect. that we have no support (or no support yet) create over-approx as nondet
  
  literalt new_variable(); // Common to all

  literalt push_variable(PTRef ptl); // Common to all
  
#ifdef PRODUCE_PROOF  
  void setup_reduction();

  void setup_interpolation();

  void setup_proof_transformation();
  
  void produceConfigMatrixInterpolants (const vector< vector<int> > &configs, vector<PTRef> &interpolants); // Common to all
#endif  
  
  virtual void initializeSolver(const char*)=0;

  virtual void freeSolver(); // Common to all

  void fill_vars(PTRef, std::map<std::string, PTRef>&); // Common to all

  // Common to all
  std::string extract_expr_str_number(const exprt &expr); // Our conversion of const that works also for negative numbers + check of result

  // Common to all
  std::string extract_expr_str_name(const exprt &expr); // General method for extracting the name of the var
  
  irep_idt get_value_from_solver(PTRef ptrf)
  {
    if (logic->hasSortBool(ptrf)) 
    {
        lbool v1 = mainSolver->getTermValue(ptrf);
        int int_v1 = toInt(v1);
        irep_idt value(std::to_string(int_v1).c_str());
        
        return value;
    } 
    else
    {
        ValPair v1 = mainSolver->getValue(ptrf);
        assert(v1.val != NULL);
        irep_idt value(v1.val);
        
        return value;
    }
  }

  bool is_value_from_solver_false(PTRef ptrf)
  {
    assert(logic->hasSortBool(ptrf));
    
    lbool v1 = mainSolver->getTermValue(ptrf);
    return (toInt(v1) == 0);
  }

#ifdef DEBUG_SMT4SOLVER
  std::map <std::string,std::string> ite_map_str;
  std::set <std::string> var_set_str;
  typedef std::map<std::string,std::string>::iterator it_ite_map_str;
  typedef std::set<std::string>::iterator it_var_set_str;

  std::string getVarData(const PTRef &var) {
	  return string(logic->getSortName(logic->getSortRef(var)));
  }
#endif
  void dump_on_error(std::string location);

  // Basic prints for debug - KE: Hope I did it right :-)
  char* getPTermString(const PTRef &term) { return logic->printTerm(term);}
  
  // build the string of the upper and lower bounds
  std::string create_bound_string(std::string base, int exp); 

public:
  char* getPTermString(const literalt &l) { return getPTermString(literals[l.var_no()]); }
  char* getPTermString(const exprt &expr) {
	  if(converted_exprs.find(expr.hash()) != converted_exprs.end())
		  return getPTermString(converted_exprs[expr.hash()]);
	  return 0;
  }
};

#endif
