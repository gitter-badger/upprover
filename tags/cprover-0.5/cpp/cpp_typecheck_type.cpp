/*******************************************************************\

Module: C++ Language Type Checking

Author: Daniel Kroening, kroening@cs.cmu.edu

\*******************************************************************/

#include <location.h>
#include <simplify_expr_class.h>
#include <ansi-c/c_qualifiers.h>

#include "cpp_typecheck.h"
#include "cpp_convert_type.h"
#include "expr2cpp.h"

/*******************************************************************\

Function: cpp_typecheckt::typecheck_type

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void cpp_typecheckt::typecheck_type(typet &type)
{
  assert(type.id()!=irep_idt());
  assert(type.is_not_nil());

  try
  {
    cpp_convert_plain_type(type);
  }

  catch(const char *error)
  {
    err_location(type);
    str << error;
    throw 0;
  }

  catch(const std::string &error)
  {
    err_location(type);
    str << error;
    throw 0;
  }

  if(type.id()=="cpp-name")
  {
    c_qualifierst qualifiers(type);
    cpp_namet cpp_name;
    cpp_name.swap(type);
    exprt symbol_expr;
    resolve(
      cpp_name,
      cpp_typecheck_resolvet::TYPE,
      cpp_typecheck_fargst(),
      symbol_expr);

    if(symbol_expr.id()!=ID_type)
    {
      err_location(type);
      str << "error: expected type";
      throw 0;
    }
    type = symbol_expr.type();

    if(type.get_bool(ID_C_constant))
      qualifiers.is_constant = true;

     qualifiers.write(type);
  }
  else if(type.id()==ID_struct ||
          type.id()==ID_union)
  {
    typecheck_compound_type(type);
  }
  else if(type.id()==ID_pointer)
  {
    // the pointer might have a qualifier, but do subtype first
    typecheck_type(type.subtype());

    // Check if it is a pointer-to-member
    if(type.find("to-member").is_not_nil())
    {
      // Must point to a method
      if(type.subtype().id()!=ID_code)
      {
        err_location(type);
        str << "pointer-to-member musst point to a method: "
            << type.subtype().pretty();
        throw 0;
      }

      typet &member=static_cast<typet &>(type.add("to-member"));

      if(member.id()=="cpp-name")
      {
        assert(member.get_sub().back().id()=="::");
        member.get_sub().pop_back();
      }

      typecheck_type(member);

      irept::subt &args=type.subtype().add(ID_arguments).get_sub();

      if(args.empty() ||
         args.front().get(ID_C_base_name)!=ID_this)
      {
        // Add 'this' to the arguments
        exprt a0(ID_argument);
        a0.set(ID_C_base_name, ID_this);
        a0.type().id(ID_pointer);
        a0.type().subtype() = member;
        args.insert(args.begin(),a0);
      }
    }

    // now do qualifier
    if(type.find("#qualifier").is_not_nil())
    {
      typet &t=(typet &)type.add("#qualifier");
      cpp_convert_plain_type(t);
      c_qualifierst q(t);
      q.write(type);
    }

    type.remove("#qualifier");
  }
  else if(type.id()==ID_array)
  {
    exprt &size_expr=static_cast<exprt &>(type.add(ID_size));

    if(size_expr.is_nil())
      type.id(ID_incomplete_array);
    else
    {
      typecheck_expr(size_expr);

      simplify_exprt expr_simplifier(*this);
      expr_simplifier.simplify(size_expr);

      if(size_expr.id()!=ID_constant &&
         size_expr.id()!=ID_infinity)
      {
        err_location(type);
        str << "failed to determine size of array: " <<
          expr2cpp(size_expr,context);
        throw 0;
      }
    }

    typecheck_type(type.subtype());

    if(type.subtype().get_bool(ID_C_constant))
      type.set(ID_C_constant, true);

    if(type.subtype().get_bool(ID_C_volatile))
      type.set(ID_C_volatile, true);
  }
  else if(type.id()==ID_code)
  {
    code_typet &code_type=to_code_type(type);
    typecheck_type(code_type.return_type());

    code_typet::argumentst &arguments=code_type.arguments();

    for(code_typet::argumentst::iterator it=arguments.begin();
        it!=arguments.end();
        it++)
    {
      typecheck_type(it->type());

      // see if there is a default value
      if(it->has_default_value())
      {
        typecheck_expr(it->default_value());
        implicit_typecast(it->default_value(), it->type());
      }
    }
  }
  else if(type.id()==ID_template)
  {
    typecheck_type(type.subtype());
  }
  else if(type.id()==ID_c_enum)
  {
    typecheck_enum_type(type);
  }
  else if(type.id()==ID_unsignedbv ||
          type.id()==ID_signedbv ||
          type.id()==ID_bool ||
          type.id()==ID_floatbv ||
          type.id()==ID_fixedbv ||
          type.id()==ID_empty)
  {
  }
  else if(type.id()==ID_symbol)
  {
  }
  else if(type.id()==ID_constructor ||
          type.id()==ID_destructor)
  {
  }
  else if(type.id()=="cpp-cast-operator")
  {
  }
  else if(type.id()=="cpp-template-type")
  {
  }
  else if(type.id()=="cpp-typeof")
  {
    exprt e = static_cast<const exprt&>(type.find("expr"));
    typecheck_expr(e);
    type = e.type();
  }
  #ifdef CPP_SYSTEMC_EXTENSION
  else if(type.id() == ID_verilogbv)
  {
  }
  #endif
  else
  {
    err_location(type);
    str << "unexpected type: " << type.pretty();
    throw 0;
  }
}