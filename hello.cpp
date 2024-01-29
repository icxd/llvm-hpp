#include <iostream>
#include <sstream>
#include "llvm.hpp"
using namespace LLVM;

int main(void) {
    std::stringstream ss;

    GlobalVariable my_variable{
        .global_var_name = "msg",
        .linkage = Linkage::Internal,
        .type = Type{
            .kind = Type::Kind::Array,
            .inner = new Type{
                .kind = Type::Kind::Integer,
                .size = 8,
            },
            .size = 14,
        },
        .initializer_constant = Constant{
            .type = Constant::Type::String,
            .string_value = "Hello World!\\00",
        },
    };
    ExternalFunction puts{
        .return_type = I32,
        .function_name = "puts",
        .parameters = {
            FunctionParameter{ Type{
                .kind = Type::Kind::Pointer,
                .inner = new Type{
                    .kind = Type::Kind::Integer,
                    .size = 8,
                }
            }}
        }
    };
    Function main_function{
        .return_type = I32,
        .function_name = "main",
        .parameters = {},
        .body = {
            BasicBlock{
                .name = "entry",
                .instructions = {
                    Instruction{
                        .type = Instruction::Type::Call,
                    },
                    Instruction{
                        .type = Instruction::Type::Ret,
                        .var = new Ret{
                            .type = I32,
                            .value = std::make_optional(Constant{
                                .type = Constant::Type::Integer,
                                .int_value = 0,
                            })
                        }
                    }
                }
            },
        },
    };

    ss << generate<GlobalVariable>(my_variable) << "\n";
    ss << generate<Function>(main_function) << "\n";
    ss << generate<ExternalFunction>(puts) << "\n";

    std::cout << ss.str() << "\n";

    return 0;
}
