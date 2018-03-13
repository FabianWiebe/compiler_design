#include "labTrees.hh"

std::pair<std::string, Type> Command::convert(Environment& e, BBlock *out, bool is_expression) {
  std::list<std::string> values;
  std::list<Type> types;
  for (Expression* par : parameters) {
    std::string value;
    Type type;
    std::tie(value, type) = par->convert(e, out);
    values.push_back(value);
    types.push_back(type);
  }
  std::string first = "", second = "";
  Type first_type = Type::UNDEFINED, second_type = Type::UNDEFINED;
  auto itr = values.begin();
  auto t_itr = types.begin();
  if (itr != values.end()) {
    first = *itr;
    first_type = *t_itr;
    if (++itr != values.end()) {
      second = *itr;
      second_type = *++t_itr;
    }
  }
  Type return_type = Type::LONG;
  if (name != "io.read" && name != "io.write" && name != "print") {
    return_type = e.get_function(name)->dump_function(e, types);
  }
  if (is_expression) {
    // use return value
    second_type = return_type;
    second = Expression::makeNames(e, second_type);
  }
  //std::cout << "---<<<< " << name << "  " << first << " "<< type_as_string(first_type) << " " << second << " " << type_as_string(second_type) << std::endl;
  out->instructions.emplace_back(second, std::string("call") + (is_expression ? "E" : "S"), name, first, first_type, second_type);
  return {second, second_type};
}

void output_start_of_asm(std::ostream& stream) {
  stream << "  asm(" << std::endl;
}

std::string format_type(Type type) {
  if (type == Type::DOUBLE) {
    return "x";
  }
  return "g";
}

void output_end_of_asm(std::ostream& stream, const std::list<std::pair<std::string, Type>>& var_names) {
  stream << ":";
  if (!var_names.empty()) {
    auto itr = var_names.begin();
    stream << " [" << itr->first << "] \"+" << format_type(itr->second) << "\" (" << itr->first << ")";
    for (++itr; itr != var_names.end(); ++itr) {
      stream << "," << std::endl << "  [" << itr->first << "] \"+" << format_type(itr->second) << "\" (" << itr->first << ")";
    }
  }
  stream << R"(
:
: "rax", "rbx", "rdx", "cc", "xmm0", "xmm1"
  );
)";
}

std::string get_print_parm(Type type) {
    switch(type) {
      case Type::LONG: {
        return "%ld";
      }
      case Type::DOUBLE: {
        return "%g"; //f
      }
      case Type::BOOL: {
        return "%d";
      }
      case Type::STRING: {
        return "%s";
      }
    }
    return "";
}

std::list<Type> types{Type::LONG, Type::DOUBLE, Type::BOOL, Type::STRING};

std::string type_to_string(Type type, std::string name) {
  if (type == Type::ARRAY) {
      return std::string("double ") + name + "[]";
    } else if (type == Type::STRING) {
      return std::string("char ") + name + "[]";
    } else {
      return type_as_string(type) + " " + name;
    }
}

std::string escape_new_lines(const std::string& str) {
  std::string result = str;
  size_t pos = 0;
  while ((pos = result.find("\\n", pos)) != std::string::npos) {
      result.insert(pos, "\\");
      pos += 3;
  }
  return result;
}

void define_vars(std::ostream& stream, Environment& e, bool esc_str) {
  for (Type type : types) {
    auto var_names = e.get_all_of_type(type);
    if (!var_names.empty()) {
      auto itr = var_names.begin();
      stream << "" << type_as_string(type) << " " << *itr;
      for (++itr; itr != var_names.end(); ++itr) {
        stream << ", " << *itr;
      }
      stream << ";" << std::endl;
    }
  }
  for (auto& pair : e.get_const_values()) {
    Type type = pair.second.type;
    stream << type_to_string(type, pair.first) << " = ";
    if (type == Type::STRING) {
      if (esc_str) {
        stream << "\\\"" << escape_new_lines(pair.second.as_string()) << "\\\"";
      } else {
        stream << "\"" << pair.second << "\"";
      }
    } else {
      stream << pair.second;
    }
    stream << ";" << std::endl;
  }
}

void output_vars(std::ostream& stream, Environment& e) {
  for (Type type : types) {
    auto var_names = e.get_all_of_type(type);
    for (const std::string& var_name : var_names) {
      stream << "  printf(\"" << var_name << " = " << get_print_parm(type) << "\\n\", " << var_name << ");" << std::endl;
    }
  }
}


void output_vars(std::ostream& stream, std::list<std::pair<std::string, Type>>& vars) {
  for (auto & pair : vars) {
    stream << "  printf(\"" << pair.first << " = " << get_print_parm(pair.second) << "\\n\", " << pair.first << ");" << std::endl;
  }
}

bool is_digits(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

std::ostream& indent(std::ostream& stream, int depth) {
  for (int i = 0; i < depth; ++i) {
    stream << " ";
  }
  return stream;
}


int BBlock::nCounter = 0;
int Expression::tmp_counter = 0;


/* Test casesType:: */
Statement *test = new Seq({
                          new Assignment(
                                  new Var("x"),
                                  new Math("+",
                                          new Var("x"),
                                          new Constant(1l)
                                  )
                          ),new If(
                                  new Comp("==",
                                          new Var("x"),
                                          new Constant(10l)
                                  ),new Assignment(
                                          new Var("y"),
                                          new Math("+",
                                                  new Var("x"),
                                                  new Constant(1l)
                                          )
                                  ), new Assignment(
                                          new Var("y"),
                                          new Math("*",
                                                  new Var("x"),
                                                  new Constant(2l)
                                          )
                                  )
                          ), new Assignment(
                                  new Var("x"),
                                  new Math("+",
                                          new Var("x"),
                                          new Constant(1l)
                                  )
                          )
});

Statement *test2 = new Seq({
                          new Assignment(
                                  new Var("x"),
                                  new Constant(0l)
                          ),new Assignment(
                                  new Var("y"),
                                  new Constant(0l)
                          ),new Assignment(
                                  new Var("x"),
                                  new Math("+",
                                          new Var("x"),
                                          new Constant(1l)
                                  )
                          ),new Assignment(
                                  new Var("y"),
                                  new Math("+",
                                          new Var("y"),
                                          new Constant(1l)
                                  )
                          ),new If(
                                  new Comp("==",
                                          new Var("x"),
                                          new Constant(0l)
                                  ),new If(
                                          new Comp("==",
                                                  new Var("y"),
                                                  new Constant(0l)
                                          ),new Assignment(
                                                  new Var("x"),
                                                  new Constant(1l)
                                          ), new Assignment(
                                                  new Var("y"),
                                                  new Constant(2l)
                                          )
                                  ), new Assignment(
                                          new Var("y"),
                                          new Constant(3l)
                                  )
                          )
});

Statement *test3 = new Seq({
                          new Assignment(
                                  new Var("x"),
                                  new Constant(1l)
                          ),new Assignment(
                                  new Var("y"),
                                  new Constant(10l)
                          ),new Loop(
                                  new Comp("!=",
                                          new Var("x"),
                                          new Var("y")
                                  ),
                                  new Assignment(
                                          new Var("x"),
                                          new Math("+",
                                                  new Var("x"),
                                                  new Constant(1l)
                                          )
                                  )
                          ),new CommandS("io.write",
                          {new Constant(std::string("x = ")),
                          new Var("x")}),
                          new CommandS("print",
                          {new Constant(std::string("\\ny =")),
                          new Var("y")})
});

void dump_blocks(BBlock *start, std::ostream& stream, void (BBlock::*func)(std::ostream&)) {
  std::set<BBlock *> done, todo;
  todo.insert(start);
  while(todo.size()>0)
  {
          // Pop an arbitrary element from todo set
          auto first = todo.begin();
          BBlock *next = *first;
          todo.erase(first);
          (next->*func)(stream);
          stream << std::endl;
          done.insert(next);
          if(next->tExit!=NULL && done.find(next->tExit)==done.end())
                  todo.insert(next->tExit);
          if(next->fExit!=NULL && done.find(next->fExit)==done.end())
                  todo.insert(next->fExit);
  }
}

/*
 * Iterate over each basic block that can be reached from the entry point
 * exactly once, so that we can dump out the entire graph.
 * This is a concrete example of the graph-walk described in lecture 7.
 */
void dumpASM(Environment& e, BBlock *start, std::ostream& stream)
{
        stream << R"(#include "stdio.h"
#include "math.h"
#include "stdlib.h"

)";

        define_vars(stream, e);
        for (auto & pair : e.get_functions()) {
          Function* func = pair.second;
          if (func->first_block) {
            stream << std::endl;
            stream << type_as_string(func->return_type) << " " << func->name << "(";
            if (!func->parameter_types.empty()) {
              auto type_itr = func->parameter_types.begin();
              auto name_itr = func->parameter_names.begin();
              stream << type_to_string(*type_itr, *name_itr);
              for(++type_itr, ++name_itr; type_itr != func->parameter_types.end(); ++type_itr, ++name_itr) {
                stream << ", " << type_to_string(*type_itr, *name_itr);
              }
              stream << ") {" << std::endl;
                dump_blocks(func->first_block, stream, &BBlock::dump);
              stream << "}" << std::endl;
            }
          }
        }

        stream << R"(
int main(int argc, char **argv)
{
)";
        //output_start_of_asm(stream);

        dump_blocks(start, stream, &BBlock::dump);        

        //output_end_of_asm(stream, vars);
        //output_vars(stream, e);
        stream << "}" << std::endl;
}

/*
 * Iterate over each basic block that can be reached from the entry point
 * exactly once, so that we can dump out the entire graph.
 * This is a concrete example of the graph-walk described in lecture 7.
 */
void dumpCFG(Environment& e, BBlock *start, std::ostream& stream)
{
        stream << "digraph {" << std::endl;

        stream << "declaration_block [shape=box, label=\"";
        define_vars(stream, e, true);
        stream << "\"];" << std::endl;
        stream << "declaration_block -> " << start->name << ";" << std::endl;

        dump_blocks(start, stream, &BBlock::dumpCFG);

        for (auto & pair : e.get_functions()) {
          Function* func = pair.second;
          if (func->first_block) {
            stream << func->name << "_block [label=\"";
            stream << type_as_string(func->return_type) << " " << func->name << "(";
            if (!func->parameter_types.empty()) {
              auto type_itr = func->parameter_types.begin();
              auto name_itr = func->parameter_names.begin();
              stream << type_to_string(*type_itr, *name_itr);
              for(++type_itr, ++name_itr; type_itr != func->parameter_types.end(); ++type_itr, ++name_itr) {
                stream << ", " << type_to_string(*type_itr, *name_itr);
              }
              stream << ")\"]" << std::endl;
              stream << func->name << "_block -> " << func->first_block->name << ";" << std::endl;
              dump_blocks(func->first_block, stream, &BBlock::dumpCFG);
            }
          }
        }

        stream << "}" << std::endl;
}
