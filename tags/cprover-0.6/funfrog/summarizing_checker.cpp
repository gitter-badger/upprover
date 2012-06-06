/*******************************************************************

 Module: Assertion checker that extracts and uses function 
 summaries

 Author: Ondrej Sery

\*******************************************************************/
#include <i2string.h>
#include "summarizing_checker.h"

void summarizing_checkert::initialize()
{
  // Prepare the summarization context
  summarization_context.analyze_functions(ns);

  // Load older summaries
  {
    const std::string& summary_file = options.get_option("load-summaries");
    if (!summary_file.empty()) {
      summarization_context.deserialize_infos(summary_file);
    }
  }

  // Prepare summary_info (encapsulated in omega), start with the lazy variant,
  // i.e., all summaries are initialized as HAVOC, except those on the way
  // to the target assertion, which are marked depending on initial mode.

  omega.initialize_summary_info (omega.get_summary_info(), goto_program);
  omega.process_goto_locations();
  init = get_init_mode(options.get_option("init-mode"));
  omega.setup_default_precision(init);
}

/*******************************************************************

 Function: summarizing_checkert::assertion_holds

 Inputs:

 Outputs:

 Purpose: Checks if the given assertion of the GP holds

\*******************************************************************/

bool summarizing_checkert::assertion_holds(const assertion_infot& assertion,
        bool store_summaries_with_assertion)
{
  fine_timet initial, final;
  initial=current_time();
  // Trivial case
  if(assertion.is_trivially_true())
  {
    status("ASSERTION IS TRIVIALLY TRUE");
    report_success();
    return true;
  }
  const bool no_slicing_option = options.get_bool_option("no-slicing");

  omega.set_initial_precision(assertion);
  const unsigned last_assertion_loc = omega.get_last_assertion_loc();
  const bool single_assertion_check = omega.is_single_assertion_check();

  partitioning_target_equationt equation(ns, summarization_context, false, store_summaries_with_assertion);

  summary_infot& summary_info = omega.get_summary_info();
  symex_assertion_sumt symex = symex_assertion_sumt(
            summarization_context, summary_info, ns, context,
            equation, message_handler, goto_program, last_assertion_loc,
            single_assertion_check, !no_slicing_option);

  setup_unwind(symex);

  refiner_assertion_sumt refiner = refiner_assertion_sumt(
              summarization_context, omega, equation,
              get_refine_mode(options.get_option("refine-mode")),
              message_handler, last_assertion_loc, true);

  prop_assertion_sumt prop = prop_assertion_sumt(summarization_context,
          equation, message_handler, max_memory_used);
  unsigned count = 0;
  bool end = false;
  if(&message_handler!=NULL){
	  std::cout <<"";
  }

  while (!end && count < 3)
  {
    count++;
    opensmt = new satcheck_opensmtt(
      options.get_int_option("verbose-solver"),
      options.get_bool_option("save-queries"));
    interpolator.reset(opensmt);
    bv_pointerst *deciderp = new bv_pointerst(ns, *opensmt);
    deciderp->unbounded_array = bv_pointerst::U_AUTO;
    decider.reset(deciderp);

    end = (count == 1) ? symex.prepare_SSA(assertion) : symex.refine_SSA (assertion, refiner.get_refined_functions());

    if (!end){

      end = prop.assertion_holds(assertion, ns, *decider, *interpolator);
      unsigned summaries_count = omega.get_summaries_count();
      if (end && interpolator->can_interpolate())
      {
        double red_timeout = compute_reduction_timeout((double)prop.get_solving_time());
        extract_interpolants(equation, red_timeout);
        if (summaries_count == 0)
        {
          status("ASSERTION(S) HOLD(S) AFTER INLINING.");
        } else {
          status(std::string("FUNCTION SUMMARIES (for ") + i2string(summaries_count) +
        		  std::string(" calls) WERE SUBSTITUTED SUCCESSFULLY."));
        }
        report_success();
      } else {
        if (summaries_count != 0 || init == ALL_HAVOCING) {
          if (init == ALL_HAVOCING){
            status("NONDETERMINISTIC ASSIGNMENTS FOR ALL FUNCTION CALLS ARE NOT SUITABLE FOR CHECKING ASSERTION.");
          } else {
            status(std::string("FUNCTION SUMMARIES (for ") +
            		i2string(summaries_count) + std::string(" calls) AREN'T SUITABLE FOR CHECKING ASSERTION"));
          }
          refiner.refine(*decider, omega.get_summary_info());

          if (refiner.get_refined_functions().size() == 0){
            status("A real bug found.");
            report_failure();
            break;
          } else {
            status("Counterexample is spurious");
            status("Go to next iteration");
          }
        } else {
          status("ASSERTION(S) DO(ES)N'T HOLD AFTER INLINING.");
          status("A real bug found");
          report_failure();
          break;
        }
      }
    }
  }
  final = current_time();

  status(std::string("\r\nTotal number of steps: ") + i2string(count));
  status(std::string("TOTAL TIME FOR CHECKING THIS CLAIM: ") + time2string(final - initial));
  return end;
}

double summarizing_checkert::compute_reduction_timeout(double solving_time)
{
  double red_timeout = 0;
  const char* red_timeout_str = options.get_option("reduce-proof").c_str();
  if (strlen(red_timeout_str)) {
    char* result;
    red_timeout = strtod(red_timeout_str, &result);

    if (result == red_timeout_str) {
            std::cerr << "WARNING: Invalid value of reduction time fraction \"" <<
                            red_timeout_str << "\". No reduction will be applied." << std::endl;
    } else {
      red_timeout = solving_time / 1000 * red_timeout;
    }
  }
  return red_timeout;
}

/*******************************************************************\

Function: summarizing_checkert::extract_interpolants

  Inputs:

 Outputs:

 Purpose: Extract and store the interpolation summaries

\*******************************************************************/

void summarizing_checkert::extract_interpolants (partitioning_target_equationt& equation, double red_timeout)
{
  summary_storet& summary_store = summarization_context.get_summary_store();
  interpolant_mapt itp_map;

  fine_timet before, after;
  before=current_time();
  equation.extract_interpolants(*interpolator, *decider, itp_map, red_timeout);
  after=current_time();
  status(std::string("INTERPOLATION TIME: ") + time2string(after-before));

  for (interpolant_mapt::iterator it = itp_map.begin();
                  it != itp_map.end(); ++it) {
    summary_infot& summary_info = it->first->summary_info;

    function_infot& function_info =
            summarization_context.get_function_info(
            summary_info.get_function_id());

    function_info.add_summary(summary_store, it->second,
            !options.get_bool_option("no-summary-optimization"));
    
    summary_info.add_used_summary(it->second);
    summary_info.set_summary();           // helpful flag for omega's (de)serialization
  }
  // Store the summaries
  const std::string& summary_file = options.get_option("save-summaries");
  if (!summary_file.empty()) {
    summarization_context.serialize_infos(summary_file, 
            omega.get_summary_info());
  }
}
/*******************************************************************\

Function: summarizing_checkert::setup_unwind

  Inputs:

 Outputs:

 Purpose: Setup the unwind bounds.

\*******************************************************************/

void summarizing_checkert::setup_unwind(symex_assertion_sumt& symex)
{
  const std::string &set=options.get_option("unwindset");
  unsigned int length=set.length();

  for(unsigned int idx=0; idx<length; idx++)
  {
    std::string::size_type next=set.find(",", idx);
    std::string val=set.substr(idx, next-idx);

    if(val.rfind(":")!=std::string::npos)
    {
      std::string id=val.substr(0, val.rfind(":"));
      unsigned long uw=atol(val.substr(val.rfind(":")+1).c_str());
      symex.unwind_set[id]=uw;
    }
    
    if(next==std::string::npos) break;
    idx=next;
  }

  symex.max_unwind=options.get_int_option("unwind");
}


/*******************************************************************\

Function: get_refine_mode

  Inputs:

 Outputs:

 Purpose: Determining the refinement mode from a string.

\*******************************************************************/

refinement_modet get_refine_mode(const std::string& str)
{
  if (str == "force-inlining" || str == "0"){
    return FORCE_INLINING;
  } else if (str == "random-substitution" || str == "1"){
    return RANDOM_SUBSTITUTION;
  } else if (str == "slicing-result" || str == "2"){
    return SLICING_RESULT;
  } else {
    // by default
    return SLICING_RESULT;
  }
};

/*******************************************************************\

Function: get_initial_mode

  Inputs:

 Outputs:

 Purpose: Determining the initial mode from a string.

\*******************************************************************/

init_modet get_init_mode(const std::string& str)
{
  if (str == "havoc-all" || str == "0"){
    return ALL_HAVOCING;
  } else if (str == "use-summaries" || str == "1"){
    return ALL_SUBSTITUTING;
  } else {
    // by default
    return ALL_SUBSTITUTING;
  }
};

/*******************************************************************\

Function: summarizing_checkert::report_success

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizing_checkert::report_success()
{
  //status("VERIFICATION SUCCESSFUL");

  switch(message_handler.get_ui())
  {
  case ui_message_handlert::OLD_GUI:
    std::cout << "SUCCESS" << std::endl
              << "Verification successful" << std::endl
              << ""     << std::endl
              << ""     << std::endl
              << ""     << std::endl
              << ""     << std::endl;
    break;

  case ui_message_handlert::PLAIN:
    break;

  case ui_message_handlert::XML_UI:
    {
      xmlt xml("cprover-status");
      xml.data="SUCCESS";
      std::cout << xml;
      std::cout << std::endl;
    }
    break;

  default:
    assert(false);
  }
}

/*******************************************************************\

Function: summarizing_checkert::report_failure

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void summarizing_checkert::report_failure()
{
  //status("VERIFICATION FAILED");

  switch(message_handler.get_ui())
  {
  case ui_message_handlert::OLD_GUI:
    break;

  case ui_message_handlert::PLAIN:
    break;

  case ui_message_handlert::XML_UI:
    {
      xmlt xml("cprover-status");
      xml.data="FAILURE";
      std::cout << xml;
      std::cout << std::endl;
    }
    break;

  default:
    assert(false);
  }
}