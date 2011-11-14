/*******************************************************************

 Module: Diff utility for 2 goto-binaries

 Author: Grigory Fedyukovich

\*******************************************************************/
#include <time_stopping.h>
#include <goto-programs/read_goto_binary.h>

#include "arith_tools.h"
#include "fixedbv.h"
#include "ieee_float.h"
#include <cstring>
#include <iostream>
#include <fstream>

class difft {
public:
  difft(
    goto_functionst &_goto_functions_1,
    goto_functionst &_goto_functions_2,
    const char* _output) :
      goto_functions_1(_goto_functions_1),
      goto_functions_2(_goto_functions_2),
      output(_output),
      do_write(true),
      summs(0)
  {};

  difft(
    goto_functionst &_goto_functions_1,
    goto_functionst &_goto_functions_2) :
      goto_functions_1(_goto_functions_1),
      goto_functions_2(_goto_functions_2),
      output("__omega"),
      do_write(false),
      summs(0)
  {};

  bool do_diff();

  void set_output(const char* _output){
    // FIXME:
    //_output = "__omega";
    do_write = true;
  };

  
private:
  std::vector<std::pair<const irep_idt*, bool> > functions;

  goto_functionst &goto_functions_1;

  goto_functionst &goto_functions_2;

  const char* output;

  bool do_write;

  std::vector<std::string > summs;

  bool is_untouched(const irep_idt &name);

  bool unroll_goto(goto_functionst &goto_functions, const irep_idt &name,
        std::vector<std::pair<std::string, unsigned> > &goto_unrolled, unsigned init, bool inherit_change);

  void do_proper_diff(std::vector<std::pair<std::string, unsigned> > &goto_unrolled_1,
               std::vector<std::pair<std::string, unsigned> > &goto_unrolled_2,
               std::vector<std::pair<std::string, unsigned> > &goto_common);

};

