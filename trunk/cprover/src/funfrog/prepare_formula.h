
#ifndef SMT_ASSERTION_SUM_H
#define SMT_ASSERTION_SUM_H

#include <util/message.h>
#include <util/ui_message.h>
#include <funfrog/interface/convertor.h>

class assertion_infot;
class namespacet;
class partitioning_target_equationt;
class check_opensmt2t;
class interpolating_solvert;
class solvert;

class prepare_formulat
{
public:
    prepare_formulat(
            partitioning_target_equationt &_target,
            ui_message_handlert &_message_handler
            )
        : message{_message_handler},
          equation(_target) {};
    
    void convert_to_formula(convertort &convertor, interpolating_solvert &interpolator);

    void error_trace(solvert &solver, const namespacet &ns, std::map<irep_idt, std::string> &guard_expln);

    bool is_satisfiable(solvert & decider);
private:

    // Store for the symex result
    messaget message;
    partitioning_target_equationt &equation;

};

#endif /* SMT_ASSERTION_SUM_H */

