/*******************************************************************

 Module: Diff utility for 2 goto-binaries

 Author: Grigory Fedyukovich

\*******************************************************************/
#include <time_stopping.h>
#include <goto-programs/read_goto_binary.h>

#include <ui_message.h>
#include "base_type.h"
#include "arith_tools.h"
#include "fixedbv.h"
#include "ieee_float.h"
#include <cstring>
#include <iostream>
#include <fstream>

class difft : public messaget{
public:
  difft(
    message_handlert &_message_handler,
    goto_functionst &_goto_functions_1,
    goto_functionst &_goto_functions_2,
    const char* _output) :
      message_handler (_message_handler),
      goto_functions_1(_goto_functions_1),
      goto_functions_2(_goto_functions_2),
      output(_output),
      do_write(true),
      old_summs(0),
      new_summs(0)
  {set_message_handler(_message_handler);};

  difft(
	message_handlert &_message_handler,
    goto_functionst &_goto_functions_1,
    goto_functionst &_goto_functions_2) :
      message_handler (_message_handler),
      goto_functions_1(_goto_functions_1),
      goto_functions_2(_goto_functions_2),
      output("__omega"),
      do_write(false),
      old_summs(0),
      new_summs(0)
  {set_message_handler(_message_handler);};

  bool do_diff();

  void set_output(const char* _output){
    // FIXME:
    //_output = "__omega";
    do_write = true;
  };

protected:
  message_handlert &message_handler;

  
private:
  std::vector<std::pair<const irep_idt*, bool> > functions_old;

  std::vector<std::pair<const irep_idt*, bool> > functions_new;

  goto_functionst &goto_functions_1;

  goto_functionst &goto_functions_2;

  const char* output;

  bool do_write;

  std::vector<std::string > old_summs;

  std::vector<std::string > new_summs;

  std::map<unsigned,std::vector<unsigned> > calltree_old;

  std::map<unsigned,std::vector<unsigned> > calltree_new;

  void stub_new_summs(unsigned loc);

  bool is_untouched(const irep_idt &name);

  bool unroll_goto(goto_functionst &goto_functions, const irep_idt &name,
        std::vector<std::pair<std::string, unsigned> > &goto_unrolled,
        std::map<unsigned,std::vector<unsigned> > &calltree, unsigned init, bool inherit_change);

  void do_proper_diff(std::vector<std::pair<std::string, unsigned> > &goto_unrolled_1,
               std::vector<std::pair<std::string, unsigned> > &goto_unrolled_2,
               std::vector<std::pair<std::string, unsigned> > &goto_common);

};

