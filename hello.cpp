#include <iostream>
#include <sstream>
#include <cstdio>
#include "llvm.hpp"
using namespace LLVM;

int main() {
    std::stringstream ss;

    auto my_variable = GlobalVariable::create("msg",
            *Type::Array(Type::Integer(8), 13),
            Constant::String("Hello World!\\00"))
                    .set_linkage(Linkage::Internal);
    auto puts = ExternalFunction::create("puts", *Type::Integer())
            .add_parameter(FunctionParameter{*Type::Pointer(Type::Integer(8))});
    auto main_function = Function::create("main", *Type::Integer())
            .add_basic_block(BasicBlock::create("entry")
                    .add_instruction(Instruction::from(Instruction::GetElementPtr(*Type::Array(Type::Integer(8), 13),
                                                                                  *Type::Pointer(Type::Array(Type::Integer(8), 13)),
                                                                                  Constant::GlobalVariable("msg")))
                            .set_name("msg_ptr"))
                    .add_instruction(Instruction::from(Instruction::Call(*Type::Integer(), "puts")
                            ->add_argument({ *Type::Pointer(Type::Integer(8)), Constant::LocalVariable("msg_ptr") })))
                    .add_instruction(Instruction::from(Instruction::Ret(*Type::Integer(), Constant::Integer(0)))));

    ss << generate<GlobalVariable>(my_variable) << "\n";
    ss << generate<Function>(main_function) << "\n";
    ss << generate<ExternalFunction>(puts) << "\n";

    std::cout << ss.str() << "\n";

    FILE *fp = fopen("hello.ll", "w");
    fprintf(fp, "%s", ss.str().c_str());
    fclose(fp);

    return 0;
}
