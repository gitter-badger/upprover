/****logic***************************************************************\

Module: Wrapper for OpenSMT2. Based on satcheck_minisat.

Author: Grigory Fedyukovich

\*******************************************************************/
#include <queue>

#include "smtcheck_opensmt2_cuf.h"

//#define SMT_DEBUG
//#define DEBUG_SSA_SMT
//#define DEBUG_SSA_SMT_NUMERIC_CONV
//#define DEBUG_ITP_VARS
//#define DEBUG_SMT_EUF
//#define DEBUG_SMT_ITP
//#define DEBUG_SMT_BB

void smtcheck_opensmt2t_cuf::initializeSolver()
{
  osmt = new Opensmt(opensmt_logic::qf_cuf);
  logic = &(osmt->getCUFLogic());
  cuflogic = &((BVLogic&)osmt->getLogic());
  mainSolver = &(osmt->getMainSolver());

  SolverId id = { 0 };
  vec<PtAsgn> asgns;
  vec<DedElem> deds;
  vec<PTRef> foo;
  bitblaster = new BitBlaster(id, osmt->getConfig(), *mainSolver, *cuflogic, asgns, deds, foo);

  const char* msg2;
  osmt->getConfig().setOption(SMTConfig::o_produce_inter, SMTOption(true), msg2);

  // KE: Fix a strange bug can be related to the fact we are pushing
  // a struct into std::vector and use [] before any push_back
  literals.push_back(PTRef());
  literalt l = new_variable(); // Shall be location 0, i.e., [l.var_no()] is [0]
  literals[0] = logic->getTerm_true(); // Which is .x =0
  // KE: End of fix
}

// Free all inner objects
smtcheck_opensmt2t_cuf::~smtcheck_opensmt2t_cuf()
{
    // Shall/When need to: freeSolver() ?
}


PTRef smtcheck_opensmt2t_cuf::get_bv_var(const char* name)
{
    //std::cout << "Creating new var name " << bv_name << std::endl;
    return cuflogic->mkBVNumVar(name);
}

PTRef smtcheck_opensmt2t_cuf::get_bv_const(int val)
{
    return cuflogic->mkBVConst(val);
}

void smtcheck_opensmt2t_cuf::set_equal_bv(PTRef l1, PTRef l2)
{
    current_partition->push(cuflogic->mkBVEq(l1, l2));
}

bool smtcheck_opensmt2t_cuf::convert_eq_ite(const exprt &expr, PTRef& ptl)
{
    assert (expr.id() == ID_equal);
    exprt sing;
    exprt ite;
    if (expr.operands()[0].id() == ID_if){
        ite = expr.operands()[0];
        sing = expr.operands()[1];
    } else if (expr.operands()[1].id() == ID_if){
        ite = expr.operands()[1];
        sing = expr.operands()[0];
    } else {
        return false;
    }
    exprt ite_guard = ite.operands()[0];
    exprt ite_tru_choice = ite.operands()[1];
    exprt ite_fls_choice = ite.operands()[2];

    PTRef sing_bv = convert_bv(sing);
    PTRef guard_bv = convert_bv(ite_guard);
    PTRef tru_eq = cuflogic->mkBVEq(sing_bv, convert_bv(ite_tru_choice));
    PTRef fls_eq = cuflogic->mkBVEq(sing_bv, convert_bv(ite_fls_choice));
    PTRef guard_tru = cuflogic->mkBVEq(guard_bv, get_bv_const(1));
    PTRef guard_fls = cuflogic->mkBVEq(guard_bv, get_bv_const(0));

    ptl = cuflogic->mkBVLor(
            cuflogic->mkBVLand(guard_tru, tru_eq),
            cuflogic->mkBVLand(guard_fls, fls_eq));
    return true;
}

PTRef smtcheck_opensmt2t_cuf::convert_bv(const exprt &expr)
{
#ifdef DEBUG_SMT_BB
        std::cout << "Bit-blasting expression type " << expr.id() << " "
               << ((expr.id()==ID_symbol || expr.id()==ID_nondet_symbol) ?
                   expr.get("identifier") : "") << std::endl;
#endif
    
    PTRef ptl;
    if (expr.id()==ID_symbol || expr.id()==ID_nondet_symbol || expr.id() == ID_typecast) {

        if (expr.id() == ID_typecast) {
            ptl = get_bv_var(expr.operands()[0].get("identifier").c_str());
        } else {
            ptl = get_bv_var(expr.get("identifier").c_str());
        }
        PTRef ptrf_cuf;
        if (converted_exprs.find(expr.hash()) != converted_exprs.end()) {
            literalt l = converted_exprs[expr.hash()]; // TODO: might be buggy
            ptrf_cuf = literals[l.var_no()];
        } else {
            // Some bug in saving the terms
            //assert(0); // TODO: fix
            
            literalt l = convert(expr);
            ptrf_cuf = literals[l.var_no()];
        }
        
    } else if (expr.id()==ID_constant) {
        if ("true" == id2string(to_constant_expr(expr).get_value())) {
            ptl = get_bv_const(1);
        } else if ("false" == id2string(to_constant_expr(expr).get_value())) {
            ptl = get_bv_const(0);
        } else {
            ptl = get_bv_const(stoi(expr.print_number_2smt()));
        }
    } else if (expr.id() == ID_index) {
        
        ptl = logic->getTerm_true(); // stub for now
            
    } else if ((expr.id() == ID_equal) ||
               (expr.id() == ID_ieee_float_equal) || 
               (expr.id() == ID_assign)) {
        if (! convert_eq_ite (expr, ptl))
            ptl = cuflogic->mkBVEq(
                    convert_bv(expr.operands()[0]),
                    convert_bv(expr.operands()[1]));

    } else if (expr.id() == ID_not) {

        ptl = cuflogic->mkBVNot(
                    convert_bv(expr.operands()[0]));

    } else if ((expr.id()==ID_notequal) || 
               (expr.id() == ID_ieee_float_notequal)) {

        ptl = cuflogic->mkBVNot(
                    cuflogic->mkBVEq(convert_bv(expr.operands()[0]),
                                    convert_bv(expr.operands()[1])));
        
    } else if (expr.id() == ID_mod) {
        
        ptl = cuflogic->mkBVMod(convert_bv(expr.operands()[0]),
                                    convert_bv(expr.operands()[1]));
    
    } else if ((expr.id() == ID_div) || (expr.id() == ID_floatbv_div)) {

        ptl = cuflogic->mkBVDiv(convert_bv(expr.operands()[0]),
                                    convert_bv(expr.operands()[1]));
    
    } else {

        // For all operators that can have more than 2 args
        vec<PTRef> args;
        int i = 0;
        forall_operands(it, expr)
        {
            PTRef cp = convert_bv(*it);
            assert(cp != PTRef_Undef);
            args.push(cp);

            i++;
        }

        if (expr.id() == ID_if) {
            assert(0);
            // GF: this should be handled by convert_eq_ite.
            //     but if ID_if appears in any other type of expr than equality,
            //     then we should handle it in a somewhat way.
        } else if (expr.id() == ID_ifthenelse) {
            assert(0);
            // GF: TODO
        } else if (expr.id() ==  ID_implies) {
            ptl = cuflogic->mkBVLor(cuflogic->mkBVNot(args[0]), args[1]);
        } else if (expr.id() ==  ID_and) {

            ptl = (args.size() > 2) ?
                split_exprs_bv(expr.id(), args) : cuflogic->mkBVLand(args);

        } else if (expr.id() ==  ID_or) {

            ptl = (args.size() > 2) ?
                split_exprs_bv(expr.id(), args) : cuflogic->mkBVLor(args);

        } else if (expr.id() == ID_ge ||
                    expr.id() ==  ID_le ||
                    expr.id() ==  ID_gt ||
                    expr.id() ==  ID_lt) {  
            // Signed/unsigend ops.
            const irep_idt &type_id = expr.type().id();
            assert(type_id != ID_pointer); // TODO

            bool is_unsigned = (type_id == ID_unsignedbv ||
                            type_id == ID_natural);

            if (expr.id() == ID_ge) {
                ptl = (is_unsigned) ? 
                    cuflogic->mkBVUgeq(args) : cuflogic->mkBVSgeq(args);
            } else if (expr.id() == ID_le) {
                ptl = (is_unsigned) ?
                    cuflogic->mkBVUleq(args) : cuflogic->mkBVSleq(args);
            } else if (expr.id() == ID_gt) {
                ptl = (is_unsigned) ?
                    cuflogic->mkBVUgt(args) : cuflogic->mkBVSgt(args);
            } else if (expr.id() == ID_lt) {
                ptl = (is_unsigned) ?
                    cuflogic->mkBVUlt(args) : cuflogic->mkBVSlt(args);
            } else {
                assert(0);
            } 
            
        } else if (expr.id() == ID_plus ||
                    expr.id() == ID_unary_plus ||
                    expr.id() == ID_floatbv_plus) {
            ptl = (args.size() > 2) ?
                split_exprs_bv(expr.id(), args) : cuflogic->mkBVPlus(args);
            
        } else if (expr.id() == ID_minus ||
                    expr.id() == ID_unary_minus || 
                    expr.id() == ID_floatbv_minus) {
            
            ptl = (args.size() > 2) ?
                split_exprs_bv(expr.id(), args) : cuflogic->mkBVMinus(args);
                
        } else if (expr.id() == ID_mult ||
                    expr.id() == ID_floatbv_mult) {
            
            ptl = (args.size() > 2) ?
                split_exprs_bv(expr.id(), args) : cuflogic->mkBVTimes(args);
                
        } else {
            
            //GF: to continue...
            ptl = logic->getTerm_true(); // stub for now

        }
    }
    
//    converted_bitblasted_exprs[expr.hash()] = ptl;
    return ptl;
}

PTRef smtcheck_opensmt2t_cuf::split_exprs_bv(irep_idt id, vec<PTRef>& args)
{
    vec<PTRef> args_current;
    args_current.push(args.last()); args.pop();
    args_current.push(args.last()); args.pop();
	
    // Do like convert
    PTRef ptl;
    if (id == ID_plus ||
        id == ID_unary_plus ||
        id == ID_floatbv_plus) {
  
        ptl = cuflogic->mkBVPlus(args_current);
  
    } else if (id == ID_minus ||
                id == ID_unary_minus || 
                id == ID_floatbv_minus) {
                    
        ptl = cuflogic->mkBVMinus(args_current);
        
    } else if (id == ID_mult || id == ID_floatbv_mult) { 
        
        ptl = cuflogic->mkBVTimes(args_current);
    
    } else if (id == ID_and) {

        ptl = cuflogic->mkBVLand(args_current);

    } else if (id == ID_or) {

        ptl = cuflogic->mkBVLor(args_current);

    } else {
        
        assert(0); // need to add the case!
    }

    // Recursive call and tail of the recursion
    if (args.size() > 0) 
    {
        args.push(ptl);
        return split_exprs_bv(id, args); // recursive call
    } else {
        //std::cout << "build " << logic->printTerm(ptl) << std::endl;
        return ptl; // tail
    }
}


exprt smtcheck_opensmt2t_cuf::get_value(const exprt &expr)
{
    PTRef ptrf;
    
    // Check if it was bit-blasted or else, check if in the cuf values
    bool is_expr_bb = (converted_bitblasted_exprs.find(expr.hash()) != converted_bitblasted_exprs.end());
    bool is_expr_uf = (converted_exprs.find(expr.hash()) != converted_exprs.end());
    
    if (is_expr_bb || is_expr_uf) {
        if (is_expr_bb)
            ptrf = converted_bitblasted_exprs[expr.hash()];
        else {
            literalt l = converted_exprs[expr.hash()]; // TODO: might be buggy
            ptrf = literals[l.var_no()];
        }

#ifdef DEBUG_SMT_BB
        std::cout << "Getting value for " << logic->printTerm(ptrf) 
                << " which " << ((is_expr_bb)? "was bb" : "was not bb") 
                << std::endl;
#endif
        
        // Get the value of the PTRef
        if (is_expr_bb) {
            bitblaster->computeModel();
            ValPair v1 = bitblaster->getValue(ptrf);
            assert(v1.val != NULL);
            irep_idt value(v1.val);
            
            // Create the expr with it
            constant_exprt tmp = constant_exprt();
            tmp.set_value(value);

            return tmp;
        }
        else if (logic->isIteVar(ptrf)) // true/false - evaluation of a branching
        {
            if (smtcheck_opensmt2t::is_value_from_solver_false(ptrf))
                return false_exprt();
            else
                return true_exprt();
        }
        else if (logic->isTrue(ptrf)) //true only
        {
            return true_exprt();
        }
        else if (logic->isFalse(ptrf)) //false only
        {
            return false_exprt();
        }
        else if (logic->isVar(ptrf)) // Constant value
        {
            // Create the value
            irep_idt value = 
                    smtcheck_opensmt2t::get_value_from_solver(ptrf);

            // Create the expr with it
            constant_exprt tmp = constant_exprt();
            tmp.set_value(value);

            return tmp;
        }
        else if (logic->isConstant(ptrf))
        {
            // Constant?
            irep_idt value = 
                    smtcheck_opensmt2t::get_value_from_solver(ptrf);

            // Create the expr with it
            constant_exprt tmp = constant_exprt();
            tmp.set_value(value);

            return tmp;
        }
        else
        {
            assert(0);
        }
    }
    else // Find the value inside the expression - recursive call
    {
        exprt tmp=expr;

        Forall_operands(it, tmp)
        {
            exprt tmp_op=get_value(*it);
            it->swap(tmp_op);
        }

        return tmp;
    }
}

literalt smtcheck_opensmt2t_cuf::const_var_Real(const exprt &expr)
{
    //TODO: Check this
    literalt l;
    string num = extract_expr_str_number(expr);
    PTRef rconst = PTRef_Undef;
    if (num.size() <= 0)
    {
        if (expr.type().id() == ID_c_enum)
        {
        	num = expr.type().find(ID_tag).pretty();
        }
        else
        {
            assert(0);
        }
    }

    rconst = cuflogic->mkCUFConst(atoi(num.c_str()));

    assert(rconst != PTRef_Undef);

    l = push_variable(rconst); // Keeps the new PTRef + create for it a new index/literal

    return l;
}

literalt smtcheck_opensmt2t_cuf::type_cast(const exprt &expr) {
    literalt l;

    // KE: Take care of type cast - recursion of convert take care of it anyhow
    // Unless it is constant bool, that needs different code:
    if (expr.is_boolean() && (expr.operands())[0].is_constant()) {
        std::string val = extract_expr_str_number((expr.operands())[0]);
        bool val_const_zero = (val.size()==0) || (stod(val)==0.0);
        l = const_var(!val_const_zero);
    } else if (is_number(expr.type()) && (expr.operands())[0].is_boolean()) {
        // Cast from Boolean to Real - Add
        literalt lt = convert((expr.operands())[0]); // Creating the Bool expression
        PTRef ptl = logic->mkIte(literals[lt.var_no()], cuflogic->mkCUFConst(1), cuflogic->mkCUFConst(0));
        l = push_variable(ptl); // Keeps the new literal + index it
    } else if (expr.is_boolean() && is_number((expr.operands())[0].type())) {
        // Cast from Real to Boolean - Add
        literalt lt = convert((expr.operands())[0]); // Creating the Bool expression
        PTRef ptl = logic->mkNot(logic->mkEq(literals[lt.var_no()], cuflogic->mkCUFConst(0)));
        l = push_variable(ptl); // Keeps the new literal + index it
    } else {
        l = convert((expr.operands())[0]);
    }

#ifdef SMT_DEBUG
    char* s = getPTermString(l);
    cout << "; (TYPE_CAST) For " << expr.id() << " Created OpenSMT2 formula " << s << endl;
    free(s);
#endif

    return l;
}

literalt smtcheck_opensmt2t_cuf::convert(const exprt &expr)
{
// GF: disabling hash for a while, since it leads to bugs at some particular cases,
//     e.g., for (= |goto_symex::guard#3| (< |c::f::a!0#7| 10))
//           and (= |goto_symex::guard#4| (< |c::f::a!0#11| 10))
//
//    if(converted_exprs.find(expr.hash()) != converted_exprs.end())
//        return converted_exprs[expr.hash()];

#ifdef SMT_DEBUG
    cout << "\n\n; ON PARTITION " << partition_count << " CONVERTING with " << expr.has_operands() << " operands "<< /*expr.pretty() << */ endl;
#endif

    /* Check which case it is */
    literalt l;
    if (expr.id()==ID_symbol || expr.id()==ID_nondet_symbol) {
#ifdef SMT_DEBUG
        cout << "; IT IS A VAR" << endl;
#endif
        l = lvar(expr);
    } else if (expr.id()==ID_constant) {
#ifdef SMT_DEBUG
        cout << "; IT IS A CONSTANT " << endl;
#endif
        l = lconst(expr);
    } else if (expr.id() == ID_typecast && expr.has_operands()) {
#ifdef SMT_DEBUG
        bool is_const =(expr.operands())[0].is_constant(); // Will fail for assert(0) if code changed here not carefully!
        cout << "; IT IS A TYPECAST OF " << (is_const? "CONST " : "") << expr.type().id() << endl;
#endif
        // KE: Take care of type cast - recursion of convert take care of it anyhow
        // Unless it is constant bool, that needs different code:
        l = type_cast(expr);
    } else if (expr.id() == ID_typecast) {
#ifdef SMT_DEBUG
        cout << "EXIT WITH ERROR: operator does not yet supported in the QF_UF version (token: " << expr.id() << ")" << endl;
        assert(false); // Need to take care of - typecast no operands
#else
        l = lunsupported2var(expr);
#endif
    } else {
#ifdef SMT_DEBUG
        cout << "; IT IS AN OPERATOR" << endl;
#endif
        vec<PTRef> args;
        int i = 0;
        forall_operands(it, expr)
        {
            // KE: recursion in case the expr is not simple - shall be in a visitor
            if (id2string(it->get(ID_identifier)).find("__CPROVER_rounding_mode#")!=std::string::npos) {
                // Skip - we don't need the rounding variable for non-bv logics + assure it is always rounding thing
            } else { // All the rest of the operators
                literalt cl = convert(*it);
                PTRef cp = literals[cl.var_no()];
                assert(cp != PTRef_Undef);
                args.push(cp);
#ifdef DEBUG_SMT_LRA
                char *s = logic->printTerm(cp);
                cout << "; On inner iteration " << i
                     << " Op to command is var no " << cl.var_no()
                     << " inner index " << cp.x
                     << " with hash code " << (*it).full_hash()
                     << " and the other one " << (*it).hash()
                     << " in address " << (void *)&(*it)
                     << " of term " << s
                     << " from |" << (*it).get(ID_identifier)
                     << "| these should be the same !" << endl; // Monitor errors in the table!

                // Auto catch this kind if problem and throws and assert!
                if ((*it).id()==ID_symbol || (*it).id()==ID_nondet_symbol) {
                    std::stringstream convert, convert2; // stringstream used for the conversion
                    convert << s; std::string str_expr1 = convert.str();
                    convert2 << "|" << (*it).get(ID_identifier) << "|"; std::string str_expr2 = convert2.str();
                    str_expr2.erase(std::remove(str_expr2.begin(),str_expr2.end(),'\\'),str_expr2.end());
                    if ((*it).id() == ID_nondet_symbol && str_expr2.find("nondet") == std::string::npos)
                        str_expr2 = str_expr2.replace(1,7, "symex::nondet");
                    assert(str_expr1.compare(str_expr2) == 0);
                }
                free(s);
#endif
            }
            i++;
        }

        PTRef ptl;
        if ((args.size() > 2) &&
            ((expr.id() == ID_plus) ||
             (expr.id() == ID_minus) ||
             (expr.id() == ID_unary_minus) ||
             (expr.id() == ID_unary_plus) ||
             (expr.id() == ID_mult) ||
             (expr.id() == ID_floatbv_plus) ||
             (expr.id() == ID_floatbv_minus) ||
             (expr.id() == ID_floatbv_mult)))
        {
            //std::cout << "Before build size of " << args.size() << " items " << std::endl;
            // KE:  patching code - check when it is fixed in OpenSMT2 and disable it here.
            ptl = split_exprs(expr.id(), args);
        } else if (expr.id()==ID_notequal) {
            ptl = logic->mkNot(logic->mkEq(args));
        } else if (expr.id() == ID_equal) {
            ptl = logic->mkEq(args);
        } else if (expr.id()==ID_if) {
            ptl = logic->mkIte(args);
#ifdef DEBUG_SMT_LRA
            ite_map_str.insert(make_pair(string(getPTermString(ptl)), logic->printTerm(logic->getTopLevelIte(ptl))));
#endif
        } else if (expr.id() == ID_ifthenelse) {
            ptl = logic->mkIte(args);
#ifdef DEBUG_SMT2SOLVER
            ite_map_str.insert(make_pair(string(getPTermString(ptl)),logic->printTerm(logic->getTopLevelIte(ptl))));
#endif
        } else if (expr.id() == ID_and) {
            ptl = logic->mkAnd(args);
        } else if (expr.id() == ID_or) {
            ptl = logic->mkOr(args);
        } else if (expr.id() == ID_not) {
            ptl = logic->mkNot(args);
        } else if (expr.id() == ID_implies) {
            ptl = logic->mkImpl(args);
        } else if (expr.id() == ID_ge) {
            ptl = cuflogic->mkCUFGeq(args);
        } else if (expr.id() == ID_le) {
            ptl = cuflogic->mkCUFLeq(args);
        } else if (expr.id() == ID_gt) {
            ptl = cuflogic->mkCUFGt(args);
        } else if (expr.id() == ID_lt) {
            ptl = cuflogic->mkCUFLt(args);
        } else if (expr.id() == ID_plus) {
            ptl = cuflogic->mkCUFPlus(args);
        } else if (expr.id() == ID_minus) {
            ptl = cuflogic->mkCUFMinus(args);
        } else if (expr.id() == ID_unary_minus) {
            ptl = cuflogic->mkCUFMinus(args);
        } else if (expr.id() == ID_unary_plus) {
            ptl = cuflogic->mkCUFPlus(args);
        } else if (expr.id() == ID_mult) {
            ptl = cuflogic->mkCUFTimes(args);
        } else if (expr.id() == ID_div) {
            ptl = cuflogic->mkCUFDiv(args);
        } else if (expr.id() == ID_assign) {
            ptl = cuflogic->mkEq(args);
        } else if (expr.id() == ID_ieee_float_equal) {
            ptl = cuflogic->mkEq(args);
        } else if (expr.id() == ID_ieee_float_notequal) {
            ptl = cuflogic->mkCUFNeq(args);
        } else if (expr.id() == ID_floatbv_plus) {
            ptl = cuflogic->mkCUFPlus(args);
        } else if (expr.id() == ID_floatbv_minus) {
            ptl = cuflogic->mkCUFMinus(args);
        } else if (expr.id() == ID_floatbv_div) {
            ptl = cuflogic->mkCUFDiv(args);
        } else if (expr.id() == ID_floatbv_mult) {
            ptl = cuflogic->mkCUFTimes(args);
        } else if(expr.id() == ID_index) {
#ifdef SMT_DEBUG
            cout << "EXIT WITH ERROR: Arrays and index of an array operator have no support yet in the UF version (token: "
                 << expr.id() << ")" << endl;
            assert(false); // No support yet for arrays
#else
            ptl = literals[lunsupported2var(expr).var_no()];
#endif
        } else {
#ifdef SMT_DEBUG // KE - Remove assert if you wish to have debug info
            cout << expr.id() << ";Don't really know how to deal with this operation:\n" << expr.pretty() << endl;
            cout << "EXIT WITH ERROR: operator does not yet supported in the LRA version (token: "
                        << expr.id() << ")" << endl;
            assert(false);
#else
            ptl = literals[lunsupported2var(expr).var_no()];
#endif
            // KE: Missing float op: ID_floatbv_sin, ID_floatbv_cos
            // Do we need them now?
        }
        l = push_variable(ptl); // Keeps the new PTRef + create for it a new index/literal
    }
    converted_exprs[expr.hash()] = l;
#ifdef SMT_DEBUG
    PTRef ptr = literals[l.var_no()];
    char *s = logic->printTerm(ptr);
    cout << "; For " << expr.id() << " Created OpenSMT2 formula " << s << endl;
    free(s);
#endif
    return l;
}

PTRef smtcheck_opensmt2t_cuf::split_exprs(irep_idt id, vec<PTRef>& args)
{
    vec<PTRef> args_current;
    args_current.push(args.last()); args.pop();
    args_current.push(args.last()); args.pop();
	
    // Do like convert
    PTRef ptl;

    if (id == ID_plus) {
        ptl = cuflogic->mkCUFPlus(args_current);
    } else if (id == ID_minus) {
        ptl = cuflogic->mkCUFMinus(args_current);
    } else if (id == ID_unary_minus) {
        ptl = cuflogic->mkCUFMinus(args_current);
    } else if (id == ID_unary_plus) {
        ptl = cuflogic->mkCUFPlus(args_current);
    } else if (id == ID_mult) {
        ptl = cuflogic->mkCUFTimes(args_current);
    } else if (id == ID_floatbv_plus) {
        ptl = cuflogic->mkCUFPlus(args_current);
    } else if (id == ID_floatbv_minus) {
        ptl = cuflogic->mkCUFMinus(args_current);
    } else if (id == ID_floatbv_mult) {
        ptl = cuflogic->mkCUFTimes(args_current);
    } else {
        assert(0); // need to add the case!
    }

    // Recursive call and tail of the recursion
    if (args.size() > 0) 
    {
        args.push(ptl);
        return split_exprs(id, args); // recursive call
    } else {
        //std::cout << "build " << logic->printTerm(ptl) << std::endl;
        return ptl; // tail
    }
}

literalt smtcheck_opensmt2t_cuf::lunsupported2var(exprt expr)
{
    literalt l;
    PTRef var;

    const string str = smtcheck_opensmt2t::_unsupported_var_str + std::to_string(unsupported2var++);
    if (expr.is_boolean())
        var = logic->mkBoolVar(str.c_str());
    else if (expr.type().id() == ID_c_bool) 
    { // KE: New Cprover code - patching
        std::string num(expr.get_string(ID_value));
        var = logic->mkBoolVar(num.c_str());
    }
    else
        var = cuflogic->mkCUFNumVar(str.c_str());

    l = push_variable(var);

    return l;
}

literalt smtcheck_opensmt2t_cuf::lnotequal(literalt l1, literalt l2){
    literalt l;
    vec<PTRef> args;
    PTRef pl1 = literals[l1.var_no()];
    PTRef pl2 = literals[l2.var_no()];
    args.push(pl1);
    args.push(pl2);
    PTRef ans = cuflogic->mkCUFNeq(args);
    l = push_variable(ans); // Keeps the new PTRef + create for it a new index/literal

    return l;
}

literalt smtcheck_opensmt2t_cuf::lvar(const exprt &expr)
{
    const string _str = extract_expr_str_name(expr); // NOTE: any changes to name - please added it to general method!
    string str = remove_invalid(_str);
    str = quote_varname(str);

    // Nil is a special case - don't create a var but a val of true
    if (_str.compare("nil") == 0) return const_var(true);

#ifdef SMT_DEBUG
    cout << "; (lvar) Create " << str << endl;
#endif

    // Else if it is really a var, continue and declare it!
    literalt l;
    PTRef var;
    if(is_number(expr.type()))
        //TODO: Check this
        var = cuflogic->mkCUFNumVar(str.c_str());
    else if (expr.is_boolean())
        var = logic->mkBoolVar(str.c_str());
    else if (expr.type().id() == ID_c_bool) 
    { // KE: New Cprover code - patching
        std::string num(expr.get_string(ID_value));
        var = logic->mkBoolVar(num.c_str());
    }
    else { // Is a function with index, array, pointer
#ifdef SMT_DEBUG
        cout << "EXIT WITH ERROR: Arrays and index of an array operator have no support yet in the UF version (token: "
             << expr.type().id() << ")" << endl;
        assert(false); // No support yet for arrays
#else
        var = literals[lunsupported2var(expr).var_no()];
#endif
    }

    l = push_variable(var); // Keeps the new PTRef + create for it a new index/literal

#ifdef DEBUG_SMT2SOLVER
    std::string add_var = str + " () " + getVarData(var);
    if (var_set_str.end() == var_set_str.find(add_var)) {
        var_set_str.insert(add_var);
    }
#endif

    return l;
}

void smtcheck_opensmt2t_cuf::bindBB(const exprt& expr, PTRef pt1, PTRef pt2){
  if (converted_bitblasted_exprs.find(expr.hash()) != converted_bitblasted_exprs.end()) return;

#ifdef DEBUG_SMT_BB
  std::cout << " -- Bind terms " << logic->printTerm(pt1) << " and "
          << logic->printTerm(pt2) << std::endl;
#endif

  bitblaster->bindCUFToBV(pt1, pt2);

  converted_bitblasted_exprs[expr.hash()] = pt2;
}

void getVarsInExpr(exprt& e, std::set<exprt>& vars)
{
  if(e.id()==ID_symbol){
    vars.insert(e);
  } else if (e.has_operands()){
    for (unsigned int i = 0; i< e.operands().size();i++){
      getVarsInExpr(e.operands()[i], vars);
    }
  }
}

int smtcheck_opensmt2t_cuf::check_ce(std::vector<exprt>& exprs)
{
    for (int i = 0; i < top_level_formulas.size(); i++){
#ifdef DEBUG_SMT_BB
        cout << "  " << logic->printTerm(top_level_formulas[i]) << "\n";
#endif
        BVRef tmp;
        bitblaster->insertEq(top_level_formulas[i], tmp);
    }
    mainSolver->push();

    bool res = true;
    unsigned int i = 0;
    while (i < exprs.size() && res){
        PTRef lp = convert_bv(exprs[i]);

#ifdef DEBUG_SMT_BB
            cout <<  "  Validating: " << logic->printTerm(lp) << endl;
#endif

        BVRef tmp;
        if (cuflogic->isBVLor(lp)){
            bitblaster->insertOr(lp, tmp);
        } else if (cuflogic->isBVEq(lp)){
            bitblaster->insertEq(lp, tmp);
        } else {
            assert(0);
        }

        res = (s_True == mainSolver->check());
        if (!res){
            cout << "\nWeak statement encoding found" << endl;
            return i;
        }
        i++;
    }
    return -1;
}

bool smtcheck_opensmt2t_cuf::refine_ce(std::vector<exprt>& exprs, int i)
{
    std::set<exprt> se;
    if (!exprs[i].has_operands() || exprs[i].operands().size() < 2){
        cout << "what should we do with it?" << endl;
        return true;
    }

    PTRef lp = convert_bv(exprs[i]);

    // do binding for lhs
    PTRef lhs = literals[convert(exprs[i].operands()[0]).var_no()];
    BVRef tmp;
    PTRef lhs_bv = convert_bv(exprs[i].operands()[0]);

    if (cuflogic->isBVLor(lp)){
        bitblaster->insertOr(lp, tmp);
    } else if (cuflogic->isBVEq(lp)){
        bitblaster->insertEq(lp, tmp);
    } else {
        assert(0);
    }

    bindBB(exprs[i].operands()[0], lhs, lhs_bv);

    // keep binding for rhs

    getVarsInExpr(exprs[i].operands()[1], se);

    for (auto it = se.begin(); it != se.end(); ++it){
        PTRef rhs = literals[convert(*it).var_no()];
        PTRef rhs_bv = convert_bv(*it);
        bindBB(*it, rhs, rhs_bv);
    }

    return solve();
}
