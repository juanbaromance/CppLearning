﻿// tagged_sqlite.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include <array>
#include <assert.h>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sqlite3.h>
#include <string>
#include <type_traits>
#include <vector>

#include "..//simple_type_name/simple_type_name.h"
#include "..//tagged_tuple/tagged_tuple.h"

template <typename... Tables> struct define_database : Tables... {};

template <typename Tag, typename... Columns> struct define_table : Columns... {
  using table_tag_type = Tag;
};

template <typename Tag, typename Type> struct define_column {
  using tag_type = Tag;
  using value_type = Type;
};

namespace detail {
template <typename T> struct t2t { using type = T; };

template <typename Tag, typename... Columns>
auto get_table_type(const define_table<Tag, Columns...> &t)
    -> t2t<define_table<Tag, Columns...>>;
template <typename Tag> auto get_table_type(...) -> t2t<void>;

template <typename Tag, typename Type>
auto get_column_type(const define_column<Tag, Type> &t)
    -> t2t<define_column<Tag, Type>>;

template <typename Tag, typename TableTag, typename... Columns>
auto get_column_type_with_table(const define_table<TableTag, Columns...> &t)
    -> decltype(get_column_type<Tag>(t));

template <typename Tag> auto get_column_type(...) -> t2t<void>;

template <typename Database, typename Tag>
using table_type = typename decltype(get_table_type<Tag>(Database{}))::type;

template <typename Database, typename TableTag, typename ColumnTag>
struct table_column_type_helper {
  using type = typename decltype(get_column_type<ColumnTag>(
      table_type<Database, TableTag>{}))::type::value_type;
};

template <typename Database, typename ColumnTag>
struct table_column_type_helper<Database, void, ColumnTag> {
  using type = typename decltype(
      get_column_type<ColumnTag>(Database{}))::type::value_type;
};

template <typename Database, typename TableTag, typename ColumnTag>
using table_column_type =
    typename table_column_type_helper<Database, TableTag, ColumnTag>::type;

template <typename Database, typename ColumnTag>
using column_type =
    typename decltype(get_column_type<ColumnTag>(Database{}))::type::value_type;

template <typename Database, typename Tag>
inline constexpr bool has_table =
    !std::is_same_v<decltype(get_table_type<Tag>(Database{})), t2t<void>>;

template <typename Database, typename TableTag, typename Tag>
inline constexpr bool has_column = !std::is_same_v<
    t2t<void>, decltype(get_column_type_with_table<Tag, TableTag>(Database{}))>;
//    decltype(test_has_column(table_type<Database, TableTag>{}))::value;

template <typename Tag, typename DbTag, typename... Tables>
constexpr bool has_unique_column_helper(define_database<DbTag, Tables...> db) {
  return (has_column<decltype(db), typename Tables::table_tag_type, Tag> ^ ...);
}

template <typename Database, typename Tag>
constexpr bool has_unique_column = has_unique_column_helper<Tag>(Database{});

} // namespace detail

template <typename Alias, typename ColumnName, typename TableName>
struct column_alias_ref {};

template <typename E> struct expression {
  E e_;

  using underlying_type = E;

  template <typename Alias> constexpr auto as() const {
    return e_.template as<Alias>();
  }
};

template <typename E>
using expression_underlying_type = typename E::underlying_type;

template <typename ColumnName, typename TableName = void> struct column_ref {
  template <typename Alias>
  constexpr column_alias_ref<Alias, ColumnName, TableName> as() const {
    return {};
  }
  expression<column_ref> operator()() const { return {*this}; }
};

template <typename N1, typename N2> struct column_ref_definer {
  using type = column_ref<N2, N1>;
};

template <typename Name, typename T> struct parameter_ref {};

template <typename Name, typename T> struct parameter_value { T t_; };

enum class binary_ops {
  equal_ = 0,
  not_equal_,
  less_,
  less_equal_,
  greater_,
  greater_equal_,
  and_,
  or_,
  add_,
  sub_,
  mul_,
  div_,
  mod_,
  size_
};

template <typename T1, typename T2, binary_ops op> auto get_binary_op_type() {
  if constexpr (op < binary_ops::add_) {
    return true;
  } else {
    return typename T1::type();
  }
}

template <typename T1, typename T2, binary_ops op>
using binary_op_type = decltype(get_binary_op_type<T1, T2, op>());

template <typename Enum> constexpr auto to_underlying(Enum e) {
  return static_cast<std::underlying_type_t<Enum>>(e);
}

inline constexpr std::array<std::string_view, to_underlying(binary_ops::size_)>
    binary_ops_to_string{
        " = ",  " != ", " < ", " <= ", " > ", " >= ", " and ",
        " or ", " + ",  " - ", " * ",  " / ", " % ",
    };

template <binary_ops bo, typename E1, typename E2> struct binary_expression {
  E1 e1_;
  E2 e2_;
};

template <typename E> auto make_expression(E e) -> decltype(expression<E>{e}) {
  return expression<E>{std::move(e)};
}

template <typename E> auto make_expression(expression<E> e) -> expression<E> {
  return e;
}

template <binary_ops bo, typename E1, typename E2>
auto make_binary_expression(E1 e1, E2 e2) {
  return binary_expression<bo, E1, E2>{std::move(e1), std::move(e2)};
}

template <typename T> struct val_holder { T e_; };

template <typename T> auto make_holder(T t) {
  return val_holder<T>{std::move(t)};
}

template <> struct expression<val_holder<std::string>> {
  using E = val_holder<std::string>;
  expression(std::string e) : e_(make_holder(std::move(e))) {}
  expression(std::string_view e) : e_(make_holder(std::string(e))) {}
  expression(const char *e) : e_(make_holder(std::string(e))) {}
  expression(E e) : e_{std::move(e)} {}
  E e_;

  using underlying_type = E;
};

template <> struct expression<val_holder<double>> {
  using E = val_holder<double>;
  expression(double e) : e_(make_holder(std::move(e))) {}
  expression(E e) : e_{std::move(e)} {}
  E e_;

  using underlying_type = E;
};
template <> struct expression<val_holder<std::int64_t>> {
  using E = val_holder<std::int64_t>;
  expression(std::int64_t e) : e_(make_holder(std::move(e))) {}
  expression(E e) : e_{std::move(e)} {}
  E e_;

  using underlying_type = E;
};

expression(std::string)->expression<val_holder<std::string>>;
expression(std::string_view)->expression<val_holder<std::string>>;
expression(const char *)->expression<val_holder<std::string>>;
expression(double)->expression<val_holder<double>>;
expression(std::int64_t)->expression<val_holder<std::int64_t>>;
expression(int)->expression<val_holder<std::int64_t>>;

std::true_type convertible_to_expression(std::string_view);
std::true_type convertible_to_expression(double);
std::true_type convertible_to_expression(std::int64_t);
std::true_type convertible_to_expression(int);
template <typename E> std::true_type convertible_to_expression(expression<E>);
std::false_type convertible_to_expression(...);

template <typename E> std::true_type is_expression(expression<E> *);
std::false_type is_expression(...);

template <typename... E>
inline constexpr bool valid_op_parameters_v = std::conjunction_v<decltype(
    convertible_to_expression(std::declval<E>()))...>
    &&std::disjunction_v<decltype(
        is_expression(std::declval<std::add_pointer_t<E>>()))...>;

template <typename... E>
using enable_op = std::enable_if_t<valid_op_parameters_v<E...>>;

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator==(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::equal_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator!=(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::not_equal_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator<(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::less_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator<=(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::less_equal_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator>(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::greater_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator>=(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::greater_equal_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2>
auto operator&&(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::and_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator||(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::or_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator+(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::add_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator-(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::sub_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator*(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::mul_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator/(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::div_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <typename E1, typename E2, typename = enable_op<E1, E2>>
auto operator%(E1 e1, E2 e2) {
  return make_expression(make_binary_expression<binary_ops::mod_>(
      expression{std::move(e1)}, expression{std::move(e2)}));
}

template <binary_ops bo, typename E1, typename E2>
std::string
expression_to_string(const expression<binary_expression<bo, E1, E2>> &e) {
  return expression_to_string(e.e_.e1_) +
         std::string(binary_ops_to_string[to_underlying(bo)]) +
         expression_to_string(e.e_.e2_);
}

namespace expression_parts {
struct expression_string;
struct column_refs;
struct parameters_ref;
struct arguments;
struct type;

} // namespace expression_parts

template <typename Tag, typename TT> auto add_tag_if_not_present(TT t) {
  if constexpr (tagged_tuple::has_tag<Tag, TT>) {
    return std::move(t);
  } else {
    return tagged_tuple::append(
        t, tagged_tuple::make_member<Tag>(tagged_tuple::make_ttuple()));
  }
}

template <typename T> struct type_ref { using type = T; };

template <typename TypeRef> using remove_type_ref_t = typename TypeRef::type;

template <typename T>
std::ostream &operator<<(std::ostream &os, const type_ref<T> &) {
  os << "type_ref<" << simple_type_name::short_name<T> << ">";
  return os;
}

template <typename Column, typename Table>
std::ostream &operator<<(std::ostream &os, const column_ref<Column, Table> &) {
  os << simple_type_name::short_name<column_ref<Column, Table>>;
  return os;
}

template <typename Name, typename T>
std::ostream &operator<<(std::ostream &os, const parameter_ref<Name, T> &) {
  os << "parameter_ref<" << simple_type_name::short_name<Name> << ","
     << simple_type_name::short_name<T> << ">";
  return os;
}

template <typename T> using ptr = T *;

template <typename Database, typename Name, typename T, typename TT>
auto process(const parameter_ref<Name, T> &e, TT raw_t) {
  auto tt = add_tag_if_not_present<expression_parts::arguments>(
      add_tag_if_not_present<expression_parts::parameters_ref>(
          std::move(raw_t)));
  auto pr = tagged_tuple::get<expression_parts::parameters_ref>(tt);
  using C = std::integral_constant<int, tagged_tuple::tuple_size(pr)>;
  auto pr_appended = tagged_tuple::append(
      pr, tagged_tuple::make_member<C>(parameter_ref<Name, T>()));

  auto tt_with_parameters_ref = tagged_tuple::merge(
      std::move(tt),
      tagged_tuple::make_ttuple(
          tagged_tuple::make_member<expression_parts::parameters_ref>(
              std::move(pr_appended)),
          tagged_tuple::make_member<expression_parts::type>(type_ref<T>())));
  return tt_with_parameters_ref;
}

template <typename Database, typename T, typename TT>
auto process(const val_holder<T> &e, TT raw_t) {
  auto tt = add_tag_if_not_present<expression_parts::arguments>(
      add_tag_if_not_present<expression_parts::parameters_ref>(
          std::move(raw_t)));
  auto pr = tagged_tuple::get<expression_parts::parameters_ref>(tt);
  using SizePR = std::integral_constant<int, tagged_tuple::tuple_size(pr)>;
  auto arg = tagged_tuple::get<expression_parts::arguments>(tt);
  using SizeArguments =
      std::integral_constant<int, tagged_tuple::tuple_size(arg)>;
  auto pr_appended = tagged_tuple::append(
      pr, tagged_tuple::make_member<SizePR>(type_ref<SizeArguments>()));

  auto tt_with_parameters_ref = tagged_tuple::merge(
      tt, tagged_tuple::make_ttuple(
              tagged_tuple::make_member<expression_parts::parameters_ref>(
                  std::move(pr_appended))));

  auto arg_appended =
      tagged_tuple::append(arg, tagged_tuple::make_member<SizeArguments>(e.e_));

  auto tt_with_arg_type = tagged_tuple::make_ttuple(
      tagged_tuple::make_member<expression_parts::arguments>(
          std::move(arg_appended)),
      tagged_tuple::make_member<expression_parts::type>(type_ref<T>()));
  auto ret =
      tagged_tuple::merge(tt_with_parameters_ref, std::move(tt_with_arg_type));
  return ret;
}

template <typename Database, binary_ops bo, typename E1, typename E2, class TT>
auto process(const binary_expression<bo, E1, E2> &e, TT tt) {
  using tagged_tuple::get;
  auto tt_left = process<Database>(e.e1_, std::move(tt));
  using left_type =
      std::decay_t<decltype(get<expression_parts::type>(tt_left))>;
  auto tt_right = process<Database>(e.e2_, std::move(tt_left));
  using right_type =
      std::decay_t<decltype(get<expression_parts::type>(tt_right))>;
  left_type *p = static_cast<right_type *>(nullptr);
  static_assert(std::is_same_v<left_type, right_type>);
  return tagged_tuple::merge(
      tt_right,
      tagged_tuple::make_ttuple(
          tagged_tuple::make_member<expression_parts::type>(
              type_ref<binary_op_type<left_type, right_type, bo>>())));
}

template <typename Database, typename E, typename TT>
auto process(const expression<E> &e, TT tt) {
  return process<Database>(e.e_, std::move(tt));
}

template <typename T>
std::string expression_to_string(const expression<val_holder<T>> &c) {
  return "?";
}

template <typename T> val_holder<T> make_val(T t) { return {std::move(t)}; }

inline auto val(std::string s) {
  return make_expression(make_val(std::move(s)));
}

inline auto val(int i) { return make_expression(make_val(std::int64_t{i})); }
inline auto val(std::int64_t i) { return make_expression(make_val(i)); }

inline auto val(double d) { return make_expression(make_val(d)); }

template <typename Column, typename Table>
std::string
expression_to_string(const expression<column_ref<Column, Table>> &) {
  if constexpr (std::is_same_v<Table, void>) {
    return std::string(simple_type_name::short_name<Column>);
  } else {
    return std::string(simple_type_name::short_name<Table>) + "." +
           std::string(simple_type_name::short_name<Column>);
  }
}

template <typename Database, typename Column, typename Table>
auto process_expression(const expression<column_ref<Column, Table>> &) {
  if constexpr (std::is_same_v<Table, void>) {
    return std::string(simple_type_name::short_name<Column>);
  } else {
    return std::string(simple_type_name::short_name<Table>) + "." +
           std::string(simple_type_name::short_name<Column>);
  }
}

template <typename Name, typename T>
std::string expression_to_string(expression<parameter_ref<Name, T>>) {
  return " ? ";
}

std::string expression_to_string(expression<std::int64_t> i) {
  return std::to_string(i.e_);
}

template <typename Name, typename T> struct parameter_object {
  expression<parameter_ref<Name, T>> operator()() const { return {}; }

  parameter_value<Name, T> operator()(T t) const { return {std::move(t)}; }
};

template <typename Name, typename T>
inline constexpr auto parameter = parameter_object<Name, T>{};
template <typename N1> struct column_ref_definer<N1, void> {
  using type = column_ref<N1, void>;
};

template <typename... ColumnRefs> struct column_ref_holder {};

template <typename ColumnRef, typename NewName> struct as_ref {};

template <typename N1, typename N2 = void>
inline constexpr auto column =
    expression<typename column_ref_definer<N1, N2>::type>{};

enum class join_type { inner, left, right, full };

template <typename Table1, typename Table2, typename Expression, join_type type>
struct join_t {
  Table1 t1;
  Table2 t2;
  Expression e_;
};

template <typename Table1, typename Table2, join_type type>
struct join_table_info {
  Table1 t1;
  Table2 t2;

  template <typename E> auto on(E e) {
    return join_t<Table1, Table2, E, type>{std::move(t1), std::move(t2),
                                           std::move(e)};
  }
};



template <typename Table> struct table_ref { using type = Table;
  template <typename Table2> auto join(Table2 table2) const {
    return join_table_info<table_ref<Table>,Table2,join_type::inner>{*this,std::move(table2)};
  }
 };

template <typename Table> inline constexpr auto table = table_ref<Table>{};

template <typename Expression> struct from_type { Expression e; };

template <typename... ColumnRefs> struct select_type {};

template <typename T1, typename T2> struct catter_imp;

template <typename... M1, typename... M2>
struct catter_imp<tagged_tuple::ttuple<M1...>, tagged_tuple::ttuple<M2...>> {
  using type = tagged_tuple::ttuple<M1..., M2...>;
};

template <typename T1, typename T2>
using cat_t = typename catter_imp<T1, T2>::type;

struct empty_type {};

class from_tag;
class select_tag;
class where_tag;
class from_tables;
class referenced_tables;
class aliases;
class selected_columns;
class potential_selected_columns;


template <typename Database, typename Table, typename TT>
auto process(const table_ref<Table>, TT tt) {
  static_assert(detail::has_table<Database, Table>, "Missing table");
  auto tt2 = add_tag_if_not_present<referenced_tables>(std::move(tt));
  if constexpr (!tagged_tuple::has_tag<
                    Table, std::decay_t<decltype(
                               tagged_tuple::get<referenced_tables>(tt2))>>) {
    return tagged_tuple::merge(
        std::move(tt2),
        tagged_tuple::make_ttuple(tagged_tuple::make_member<referenced_tables>(
            tagged_tuple::make_ttuple(
                tagged_tuple::make_member<Table>(empty_type{})))));
  } else
    return std::move(tt);
}

template <typename Database, typename Alias, typename Table, typename Column,
          typename TT>
auto process(const column_alias_ref<Alias, Column, Table> &, TT tt) {
  static_assert(detail::has_table<Database, Table>, "Missing table");
  static_assert(detail::has_column<Database, Table, Column>, "Missing column");
  auto tt2 = add_tag_if_not_present<aliases>(std::move(tt));
  static_assert(
      !tagged_tuple::has_tag<
          Alias, std::decay_t<decltype(tagged_tuple::get<aliases>(tt2))>>,
      "Aliases already defined");
  return tagged_tuple::merge(
      std::move(tt2),
      tagged_tuple::make_ttuple(
          tagged_tuple::make_member<aliases>(tagged_tuple::make_ttuple(
              tagged_tuple::make_member<Alias>(column_ref<Table, Column>()))),
          tagged_tuple::make_ttuple(
              tagged_tuple::make_member<potential_selected_columns>(
                  tagged_tuple::make_ttuple(tagged_tuple::make_member<Column>(
                      type_ref<detail::table_column_type<Database, Table,
                                                         Column>>()))))));
}

template <typename Database, typename Table, typename Column, typename TT>
auto process(const column_ref<Column, Table> &, TT tt) {
  constexpr bool has_table = std::is_same_v<Table, void>;
  if constexpr (has_table) {
    static_assert(detail::has_unique_column<Database, Column>,
                  "Non-unique column without table name");
  } else {
    static_assert(detail::has_table<Database, Table>, "Missing table");
    static_assert(detail::has_column<Database, Table, Column>,
                  "Missing column");
  }
  auto tt2 = add_tag_if_not_present<potential_selected_columns>(std::move(tt));
  static_assert(
      !tagged_tuple::has_tag<
          Table, std::decay_t<decltype(
                     tagged_tuple::get<potential_selected_columns>(tt2))>>,
      "Column already selected");
  return tagged_tuple::merge(
      std::move(tt2),
      tagged_tuple::make_ttuple(
          tagged_tuple::make_member<potential_selected_columns>(
              tagged_tuple::make_ttuple(
                  tagged_tuple::make_member<column_ref<Column, Table>>(
                      type_ref<detail::table_column_type<Database, Table,
                                                         Column>>()))),
          tagged_tuple::make_member<expression_parts::type>(
              type_ref<detail::table_column_type<Database, Table, Column>>())));
}

template <typename Database, typename Table1, typename Table2,
          typename Expression, join_type type, typename TT>
auto process(const join_t<Table1, Table2, Expression, type> &j, TT tt) {
  auto tt1 = process<Database>(j.t1, std::move(tt));
  auto tt2 = process<Database>(j.t2, std::move(tt1));
  return process<Database>(j.e_, std::move(tt2));
}

template <typename Database, typename TT> auto process_helper(TT tt) {
  return std::move(tt);
}

template <typename Database, typename TT, typename T, typename... Rest>
auto process_helper(TT tt, T &&t, Rest &&... rest) {
  return process_helper<Database>(
      process<Database>(std::forward<T>(t), std::move(tt)),
      std::forward<Rest>(rest)...);
}

template <typename Database, typename... Columns, typename TT>
auto process(const select_type<Columns...> &s, TT tt) {
  return process_helper<Database>(std::move(tt), Columns()...);
}

template <typename Database, typename E, typename TT>
auto process(const from_type<E> &f, TT tt) {
  return process(f.e, std::move(tt));
}

template <typename Table1, typename Table2, typename Expression>
auto join(table_ref<Table1> t1, table_ref<Table2> t2, Expression e) {
  return join_t<table_ref<Table1>, table_ref<Table2>, Expression,
                join_type::inner>{std::move(t1), std::move(t2), std::move(e)};
}

template <typename Database, typename TTuple = tagged_tuple::ttuple<>>
struct query_builder {
  template <typename NewDatabase, typename NewTTuple>
  static auto make_query_builder(NewTTuple t) {
    return query_builder<NewDatabase, NewTTuple>{std::move(t)};
  }

  using ttuple_type = TTuple;

  TTuple t_;

  template <typename Table>
  // query_builder<
  //     Database, cat_t<TTuple, >>
  auto from(table_ref<Table> e) && {
    static_assert((detail::has_table<Database, Table>),
                  "All tables not found in database");
    return make_query_builder<Database>(
        std::move(t_) |
        tagged_tuple::make_ttuple(tagged_tuple::make_member<from_tag>(
            from_type<Table>{std::move(e)})));
  }

  template <typename Table1, typename Table2, typename Expression,
            join_type type>
  auto from(join_t<Table1, Table2, Expression, type> j) && {
    static_assert((detail::has_table<Database, typename Table1::type> &&
                   detail::has_table<Database, typename Table2::type>),
                  "All tables not found in database");
    return make_query_builder<Database>(
        std::move(t_) | tagged_tuple::make_ttuple(
                            tagged_tuple::make_member<from_tag>(std::move(j))));
  }

  template <typename... Columns> auto select(Columns...) && {
    return make_query_builder<Database>(
        std::move(t_) |
        tagged_tuple::make_ttuple(
            tagged_tuple::make_member<select_tag>(select_type<Columns...>{})));
  }

  template <typename Expression> auto where(Expression e) && {
    return make_query_builder<Database>(
        std::move(t_) |
        tagged_tuple::make_ttuple(tagged_tuple::make_member<where_tag>(e)));
  }

  auto build() && {
    if constexpr (tagged_tuple::has_tag<select_tag, TTuple>) {
      auto tt_select_raw = process<Database>(tagged_tuple::get<select_tag>(t_),
                                             tagged_tuple::make_ttuple());
      auto tt_select = tagged_tuple::append(
          tt_select_raw,
          tagged_tuple::make_member<selected_columns>(
              tagged_tuple::get<potential_selected_columns>(tt_select_raw)));

      auto tt_from = process<Database>(tagged_tuple::get<from_tag>(t_),
                                       std::move(tt_select));

      if constexpr (tagged_tuple::has_tag<where_tag, TTuple>) {
        auto tt_where = process<Database>(tagged_tuple::get<where_tag>(t_),
                                          std::move(tt_from));

        return tagged_tuple::merge(std::move(t_), std::move(tt_where));
      } else {
        return tagged_tuple::merge(std::move(t_), std::move(tt_select));
      }
    }
  }
};

template <typename Builder> struct query_t {
  query_t(Builder b) : t_(std::move(b).build()) {}

  decltype(std::declval<Builder &&>().build()) t_;
};

template <typename Column, typename Table>
std::string to_column_string(expression<column_ref<Column, Table>>) {
  if constexpr (std::is_same_v<Table, void>) {
    return std::string(simple_type_name::short_name<Column>);
  } else {
    return std::string(simple_type_name::short_name<Table>) + "." +
           std::string(simple_type_name::short_name<Column>);
  }
}

template <typename Alias, typename Column, typename Table>
std::string to_column_string(column_alias_ref<Alias, Column, Table>) {
  return to_column_string(expression<column_ref<Column, Table>>{}) + " AS " +
         std::string(simple_type_name::short_name<Alias>);
}

inline std::string join_vector(const std::vector<std::string> &v) {
  std::string ret;
  for (auto &s : v) {
    ret += s + ", ";
  }
  if (ret.back() == ' ')
    ret.resize(ret.size() - 2);

  return ret;
}
template <typename Table> std::string to_statement(const table_ref<Table> &) {
  return std::string(simple_type_name::short_name<Table>);
}
template <typename Table1, typename Table2, typename Expression>
std::string
to_statement(const join_t<Table1, Table2, Expression, join_type::inner> &j) {
  return std::string("\nFROM ") + to_statement(j.t1) + " JOIN " +
         to_statement(j.t2) + " ON " + expression_to_string(j.e_);
}

template <typename... Members>
std::string to_statement(tagged_tuple::ttuple<Members...> t) {
  using T = decltype(t);
  auto statement = to_statement(tagged_tuple::get<select_tag>(t)) +
                   to_statement(tagged_tuple::get<from_tag>(t));
  if constexpr (tagged_tuple::has_tag<where_tag, T>) {
    statement = statement + "\nWHERE " +
                expression_to_string(tagged_tuple::get<where_tag>(t));
  }
  return statement;
}

template <typename... Columns>
std::string to_statement(select_type<Columns...>) {
  std::vector<std::string> v{to_column_string(Columns{})...};

  return std::string("SELECT ") + join_vector(v);
}

template <typename... Tables> std::string to_statement(from_type<Tables...>) {
  std::vector<std::string> v{
      std::string(simple_type_name::short_name<Tables>)...};
  return std::string("\nFROM ") + join_vector(v);
}

template <typename Database, typename TTuple>
std::string to_statement(const query_builder<Database, TTuple> &t) {
  using tagged_tuple::get;
  std::string ret;
  if constexpr (tagged_tuple::has_tag<select_tag, TTuple>) {
    ret += to_statement(get<select_tag>(t.t_));
  }

  if constexpr (tagged_tuple::has_tag<from_tag, TTuple>) {
    ret += to_statement(get<from_tag>(t.t_));
  }
  return ret;
}

template <typename Member>
using member_value_type_t = typename Member::value_type;
template <typename Member> using member_tag_type_t = typename Member::tag_type;

template <typename T> struct result_type { using type = std::optional<T>; };

template <> struct result_type<std::string> {
  using type = std::optional<std::string_view>;
};
template <typename T> using result_type_t = typename result_type<T>::type;

template <typename SelectedColumns> struct row_type_helper;

template <typename... Columns>
struct row_type_helper<tagged_tuple::ttuple<Columns...>> {
  using type = tagged_tuple::ttuple<tagged_tuple::member<
      member_tag_type_t<Columns>,
      result_type_t<remove_type_ref_t<member_value_type_t<Columns>>>>...>;
};

template <typename Query>
using row_type_t = typename row_type_helper<
    tagged_tuple::element_type_t<selected_columns, Query>>::type;

template <typename Column, typename Table, typename Value>
const Value &
field_helper(const tagged_tuple::member<column_ref<Column, Table>, Value> &m) {
  return m.value;
}

template <typename Column, typename RowTuple>
decltype(auto) field(const RowTuple &r) {
  return field_helper<Column>(r);
}

template <typename Table, typename Column, typename RowTuple>
decltype(auto) field(const RowTuple &r) {
  return field_helper<Column, Table>(r);
}

void check_sqlite_return(int r, int good = SQLITE_OK) {
  assert(r == good);
  if (r != good) {
    std::cerr << "SQLITE ERROR " << r;
    throw std::runtime_error("sqlite error");
  }
}

inline bool read_row_into(sqlite3_stmt *stmt, int index,
                          std::optional<std::int64_t> &v) {
  auto type = sqlite3_column_type(stmt, index);
  if (type == SQLITE_INTEGER) {
    v = sqlite3_column_int64(stmt, index);
    return true;
  } else if (type == SQLITE_NULL) {
    v = std::nullopt;
    return true;
  } else {
    return false;
  }
}

inline bool read_row_into(sqlite3_stmt *stmt, int index,
                          std::optional<double> &v) {
  auto type = sqlite3_column_type(stmt, index);
  if (type == SQLITE_FLOAT) {
    v = sqlite3_column_double(stmt, index);
    return true;
  } else if (type == SQLITE_NULL) {
    v = std::nullopt;
    return true;
  } else {
    return false;
  }
}
inline bool read_row_into(sqlite3_stmt *stmt, int index,
                          std::optional<std::string_view> &v) {
  auto type = sqlite3_column_type(stmt, index);
  if (type == SQLITE_TEXT) {
    const char *ptr =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, index));
    auto size = sqlite3_column_bytes(stmt, index);

    v = std::string_view(ptr, ptr ? size : 0);
    return true;
  } else if (type == SQLITE_NULL) {
    v = std::nullopt;
    return true;
  } else {
    return false;
  }
}

template <typename TTuple>
auto read_row(const TTuple &query, sqlite3_stmt *stmt) {
  using row_type = row_type_t<TTuple>;
  row_type row;
  int count = sqlite3_column_count(stmt);
  if (count != tagged_tuple::tuple_size(row)) {
    return row;
  }
  int index = 0;
  tagged_tuple::for_each(row, [&](auto &m) mutable {
    read_row_into(stmt, index, m.value);
    ++index;
  });
  return row;
}

template <typename Query> struct row_range {

  using row_type = row_type_t<Query>;
  row_type row;
  Query query;
  int last_result = 0;
  sqlite3_stmt *stmt;

  row_range(Query query, sqlite3_stmt *stmt) : query(query), stmt(stmt) {}

  struct end_type {};

  end_type end() { return {}; }

  void next() { last_result = sqlite3_step(stmt); }

  struct row_iterator {
    row_range *p;

    row_iterator &operator++() {
      p->next();
      return *this;
    }

    bool operator!=(end_type) { return p->last_result == SQLITE_ROW; }

    row_type &operator*() {
      p->row = read_row(p->query, p->stmt);
      return p->row;
    }
  };

  row_iterator begin() {
    next();
    return {this};
  }
};

inline bool bind(sqlite3_stmt *stmt, int index, double v) {
  auto r = sqlite3_bind_double(stmt, index, v);
  return r == SQLITE_OK;
}

inline bool bind(sqlite3_stmt *stmt, int index, std::int64_t v) {
  auto r = sqlite3_bind_int64(stmt, index, v);
  return r == SQLITE_OK;
}

inline bool bind(sqlite3_stmt *stmt, int index, const std::string_view v) {
  auto r = sqlite3_bind_text(stmt, index, v.data(), v.size(), SQLITE_TRANSIENT);
  return r == SQLITE_OK;
}

template <typename Query>
void bind_query(const Query &query, sqlite3_stmt *stmt) {
  if constexpr (!tagged_tuple::has_tag<expression_parts::arguments, Query>) {
    return;
  } else {
    auto &args = tagged_tuple::get<expression_parts::arguments>(query);

    int count = sqlite3_bind_parameter_count(stmt);
    if (count != tagged_tuple::tuple_size(args)) {
      return;
    }
    int index = 1;
    tagged_tuple::for_each(args, [&](auto &m) mutable {
      bind(stmt, index, m.value);
      ++index;
    });
  }
}

template <typename Query>
auto execute_query(const Query &query, sqlite3 *sqldb) {
  sqlite3_stmt *stmt;
  auto query_string = to_statement(query.t_);
  auto rc = sqlite3_prepare_v2(sqldb, query_string.c_str(), query_string.size(),
                               &stmt, 0);
  check_sqlite_return(rc);
  bind_query(query.t_, stmt);
  return row_range(query.t_, stmt);
}

template <typename DB, typename... Args> auto select(Args &&... args) {
  return query_builder<DB>().select(std::forward<Args>(args)...);
}

// TODO: Reference additional headers your program requires here.
