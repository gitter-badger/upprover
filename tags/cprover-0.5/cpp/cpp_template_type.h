/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@cs.cmu.edu

\*******************************************************************/

#ifndef CPROVER_CPP_TEMPLATE_TYPE_H
#define CPROVER_CPP_TEMPLATE_TYPE_H

#include <type.h>

class template_typet:public typet
{
public:
  template_typet()
  {
    id(ID_template);
  }

  irept &arguments()
  {
    return add(ID_arguments);
  }

  const irept &arguments() const
  {
    return find(ID_arguments);
  }
};

inline template_typet &to_template_type(typet &type)
{
  assert(type.id()==ID_template);
  return static_cast<template_typet &>(type);
}

inline const template_typet& to_template_type(const typet &type)
{
  assert(type.id()==ID_template);
  return static_cast<const template_typet &>(type);
}

inline const typet &template_subtype(const typet &type)
{
  if(type.id()==ID_template)
    return type.subtype();

  return type;
}

inline typet &template_subtype(typet &type)
{
  if(type.id()==ID_template)
    return type.subtype();

  return type;
}

#endif
