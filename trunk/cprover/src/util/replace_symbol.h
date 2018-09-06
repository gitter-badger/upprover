/*******************************************************************\

Module:

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/


#ifndef CPROVER_UTIL_REPLACE_SYMBOL_H
#define CPROVER_UTIL_REPLACE_SYMBOL_H

//
// true: did nothing
// false: replaced something
//

#include "expr.h"

#include <unordered_map>

/// Replace expression or type symbols by an expression or type, respectively.
class replace_symbolt
{
public:
  typedef std::unordered_map<irep_idt, exprt> expr_mapt;
  typedef std::unordered_map<irep_idt, typet> type_mapt;

  void insert(const irep_idt &identifier,
                     const exprt &expr)
  {
    expr_map.insert(std::pair<irep_idt, exprt>(identifier, expr));
  }

  void insert(const class symbol_exprt &old_expr,
              const exprt &new_expr);

  void insert(const irep_idt &identifier,
                     const typet &type)
  {
    type_map.insert(std::pair<irep_idt, typet>(identifier, type));
  }

  /// \brief Replaces a symbol with a constant
  /// If you are replacing symbols with constants in an l-value, you can
  /// create something that is not an l-value.   For example if your
  /// l-value is "a[i]" then substituting i for a constant results in an
  /// l-value but substituting a for a constant (array) wouldn't.  This
  /// only applies to the "top level" of the expression, for example, it
  /// is OK to replace b with a constant array in a[b[0]].
  ///
  /// \param dest The expression in which to do the replacement
  /// \param replace_with_const If false then it disables the rewrites that
  /// could result in something that is not an l-value.
  ///
  virtual bool replace(
    exprt &dest,
    const bool replace_with_const=true) const;

  virtual bool replace(typet &dest) const;

  void operator()(exprt &dest) const
  {
    replace(dest);
  }

  void operator()(typet &dest) const
  {
    replace(dest);
  }

  void clear()
  {
    expr_map.clear();
    type_map.clear();
  }

  bool empty() const
  {
    return expr_map.empty() && type_map.empty();
  }

  std::size_t erase(const irep_idt &id)
  {
    return expr_map.erase(id) + type_map.erase(id);
  }

  bool replaces_symbol(const irep_idt &id) const
  {
    return expr_map.find(id) != expr_map.end() ||
           type_map.find(id) != type_map.end();
  }

  replace_symbolt();
  virtual ~replace_symbolt();

  const expr_mapt &get_expr_map() const
  {
    return expr_map;
  }

  expr_mapt &get_expr_map()
  {
    return expr_map;
  }

protected:
  expr_mapt expr_map;
  type_mapt type_map;

protected:
  bool have_to_replace(const exprt &dest) const;
  bool have_to_replace(const typet &type) const;
};

#endif // CPROVER_UTIL_REPLACE_SYMBOL_H
