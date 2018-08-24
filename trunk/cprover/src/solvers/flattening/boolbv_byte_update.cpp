/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include "boolbv.h"

#include <iostream>
#include <cassert>

#include <util/arith_tools.h>
#include <util/byte_operators.h>

#include "bv_endianness_map.h"

bvt boolbvt::convert_byte_update(const byte_update_exprt &expr)
{
  if(expr.operands().size()!=3)
    throw "byte_update takes three operands";

  const exprt &op=expr.op0();
  const exprt &offset_expr=expr.offset();
  const exprt &value=expr.value();

  PRECONDITION(
    expr.id() == ID_byte_update_little_endian ||
    expr.id() == ID_byte_update_big_endian);
  const bool little_endian = expr.id() == ID_byte_update_little_endian;

  bvt bv=convert_bv(op);

  const bvt &value_bv=convert_bv(value);
  std::size_t update_width=value_bv.size();
  std::size_t byte_width=8;

  if(update_width>bv.size())
    update_width=bv.size();

  // see if the byte number is constant

  mp_integer index;
  if(!to_integer(offset_expr, index))
  {
    // yes!
    mp_integer offset=index*8;

    if(offset+update_width>mp_integer(bv.size()) || offset<0)
    {
      // out of bounds
    }
    else
    {
      if(little_endian)
      {
        for(std::size_t i=0; i<update_width; i++)
          bv[integer2size_t(offset+i)]=value_bv[i];
      }
      else
      {
        bv_endianness_mapt map_op(op.type(), false, ns, boolbv_width);
        bv_endianness_mapt map_value(value.type(), false, ns, boolbv_width);

        std::size_t offset_i=integer2unsigned(offset);

        for(std::size_t i=0; i<update_width; i++)
        {
          size_t index_op=map_op.map_bit(offset_i+i);
          size_t index_value=map_value.map_bit(i);

          assert(index_op<bv.size());
          assert(index_value<value_bv.size());

          bv[index_op]=value_bv[index_value];
        }
      }
    }

    return bv;
  }

  // byte_update with variable index
  for(std::size_t offset=0; offset<bv.size(); offset+=byte_width)
  {
    // index condition
    equal_exprt equality;
    equality.lhs()=offset_expr;
    equality.rhs()=from_integer(offset/byte_width, offset_expr.type());
    literalt equal=convert(equality);

    bv_endianness_mapt map_op(op.type(), little_endian, ns, boolbv_width);
    bv_endianness_mapt map_value(value.type(), little_endian, ns, boolbv_width);

    for(std::size_t bit=0; bit<update_width; bit++)
      if(offset+bit<bv.size())
      {
        std::size_t bv_o=map_op.map_bit(offset+bit);
        std::size_t value_bv_o=map_value.map_bit(bit);

        bv[bv_o]=prop.lselect(equal, value_bv[value_bv_o], bv[bv_o]);
      }
  }

  return bv;
}
