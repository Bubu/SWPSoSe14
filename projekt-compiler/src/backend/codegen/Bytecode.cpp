#include <backend/codegen/Bytecode.h>


codegen::Bytecode::CODE_FUNC_MAPPING
codegen::Bytecode::func_map= {
  {Command::Type::OUTPUT, &output_ByteCode},
  {Command::Type::PUSH_CONST, &push_ByteCode},
  {Command::Type::ADD, &add_ByteCode},
  {Command::Type::SUB, &sub_ByteCode},
  {Command::Type::MULT, &mult_ByteCode},
  {Command::Type::DIV, &div_ByteCode},
  {Command::Type::MOD, &mod_ByteCode},
  {Command::Type::CUT, &cut_ByteCode},
  {Command::Type::APPEND, &append_ByteCode},
  {Command::Type::SIZE, &size_ByteCode},
  {Command::Type::CALL , &call_ByteCode},
  {Command::Type::NIL, &null_ByteCode},
  {Command::Type::LIST_CONS, &list_push_ByteCode},
  {Command::Type::LIST_BREAKUP, &list_pop_ByteCode},
  {Command::Type::FALSE, &false_ByteCode},
  {Command::Type::GREATER, &greater_ByteCode},
  {Command::Type::EQUAL, &equal_ByteCode},
  {Command::Type::TRUE, &true_ByteCode},
  //Command::Type::REFLECTOR
  //Command::Type::START
  //Command::Type::FINISH
  //Command::Type::LAMBDA
  {Command::Type::BOOM, &boom_ByteCode},
  {Command::Type::EOF_CHECK, &eof_ByteCode},
  {Command::Type::INPUT, &input_ByteCode},
  {Command::Type::UNDERFLOW_CHECK, &underflow_ByteCode},
  {Command::Type::TYPE_CHECK, &type_ByteCode},
  {Command::Type::EASTJUNC, &if_or_while_ByteCode},
  {Command::Type::WESTJUNC, &if_or_while_ByteCode},
  {Command::Type::NORTHJUNC, &if_or_while_ByteCode},
  {Command::Type::SOUTHJUNC, &if_or_while_ByteCode},
  {Command::Type::VAR_POP, &pop_Variable},
  {Command::Type::VAR_PUSH, &push_Variable}
};

codegen::Bytecode::Bytecode(ConstantPool& p) : pool(p) {}

codegen::Bytecode::~Bytecode() {}


codegen::Bytecode* codegen::Bytecode::build(Graphs::Graph_ptr graph)
{
  return this;
}

//================================================================================
//==================================GETTER========================================
//================================================================================

size_t codegen::Bytecode::length()
{
  return bytecode.size();
}

int codegen::Bytecode::get_local_count()
{
  return local_count;
}

ConstantPool& codegen::Bytecode::get_constant_pool()
{
  return pool;
}

codegen::Bytecode::Code& codegen::Bytecode::get_bytecode()
{
  return bytecode;
}


//================================================================================
//==================================INDICIES======================================
//================================================================================
uint16_t codegen::Bytecode::get_class_idx(const std::string& class_name)
{
  return pool.addClassRef(pool.addString(class_name));
}

uint16_t codegen::Bytecode::get_name_type_idx(const std::string& name, const std::string& type) {
  uint16_t name_idx = pool.addString(name);
  uint16_t type_idx = pool.addString(type);
  return pool.addNameAndType(name_idx, type_idx);
}

uint16_t codegen::Bytecode::get_method_idx(const std::string& class_name, const std::string& member_name, const std::string& descriptor)
{
  return pool.addMethRef(get_class_idx(class_name), get_name_type_idx(member_name, descriptor));
}

uint16_t codegen::Bytecode::get_field_idx(const std::string& class_name, const std::string& member_name, const std::string& descriptor)
{
  return pool.addFieldRef(get_class_idx(class_name), get_name_type_idx(member_name, descriptor));
}


uint16_t  codegen::Bytecode::get_stack_method_idx(const std::string& method, const std::string& descriptor)
{
  return get_method_idx("java/util/ArrayDeque", method, descriptor);
}

uint16_t codegen::Bytecode::get_stack_field_idx()
{
  return get_field_idx(Env::getDstClassName(), "stack", "Ljava/util/ArrayDeque;");
}

//================================================================================
//==================================FUNCTORS======================================
//================================================================================

void codegen::output_ByteCode(Bytecode::Current_state state) { }
void codegen::push_ByteCode(Bytecode::Current_state state) { }
//ARITMETIC OPERATIONS
void codegen::add_ByteCode(Bytecode::Current_state state) { }
void codegen::sub_ByteCode(Bytecode::Current_state state) { }
void codegen::mult_ByteCode(Bytecode::Current_state state) { }
void codegen::div_ByteCode(Bytecode::Current_state state) { }
void codegen::mod_ByteCode(Bytecode::Current_state state) { }
//STRING OPERATIONS
void codegen::cut_ByteCode(Bytecode::Current_state state) { }
void codegen::append_ByteCode(Bytecode::Current_state state) { }
void codegen::size_ByteCode(Bytecode::Current_state state) { }
//CALL
void codegen::call_ByteCode(Bytecode::Current_state state) { }
// LIST OPERATIONS
void codegen::null_ByteCode(Bytecode::Current_state state) { }
void codegen::list_push_ByteCode(Bytecode::Current_state state) { }
void codegen::list_pop_ByteCode(Bytecode::Current_state state) { }
//BOOLEAN ARITHMETIC
void codegen::false_ByteCode(Bytecode::Current_state state) { }
void codegen::greater_ByteCode(Bytecode::Current_state state) { }
void codegen::equal_ByteCode(Bytecode::Current_state state) { }
void codegen::true_ByteCode(Bytecode::Current_state state) { }
//IO OPERATIONS
void codegen::boom_ByteCode(Bytecode::Current_state state) { }
void codegen::eof_ByteCode(Bytecode::Current_state state) { }
void codegen::input_ByteCode(Bytecode::Current_state state) { }
void codegen::underflow_ByteCode(Bytecode::Current_state state) { }
void codegen::type_ByteCode(Bytecode::Current_state state) { }
//CONTROL STRUCTURE
void codegen::if_or_while_ByteCode(Bytecode::Current_state state) { }
//VARIABLES
void codegen::pop_Variable(Bytecode::Current_state state) { }
void codegen::push_Variable(Bytecode::Current_state state) { }
