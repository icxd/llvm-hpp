#include <iostream>
#include <sstream>
#include <cstdio>
#include "llvm.hpp"
using namespace LLVM;

int main() {
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
            .size = 13,
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
                        .type = Instruction::Type::GetElementPtr,
                        .name = "msg_ptr",
                        .var = new GetElementPtr{
                            .type = Type{
                                .kind = Type::Kind::Array,
                                .inner = new Type{
                                    .kind = Type::Kind::Integer,
                                    .size = 8,
                                },
                                .size = 13,
                            },
                            .ptr_type = Type {
                                .kind = Type::Kind::Pointer,
                                .inner = new Type{
                                    .kind = Type::Kind::Array,
                                    .inner = new Type{
                                        .kind = Type::Kind::Integer,
                                        .size = 8,
                                    },
                                    .size = 13,
                                },
                            },
                            .ptr_value = Constant{
                                .type = Constant::Type::GlobalVariable,
                                .variable_name = "msg",
                            }
                        }
                    },
                    Instruction{
                        .type = Instruction::Type::Call,
                        .var = new Call{
                            .return_type = I32,
                            .name = "puts",
                            .arguments = {
                                {Type{
                                    .kind = Type::Kind::Pointer,
                                    .inner = new Type{
                                        .kind = Type::Kind::Integer,
                                        .size = 8,
                                    }
                                }, Constant{
                                    .type = Constant::Type::LocalVariable,
                                    .variable_name = "msg_ptr"
                                }}
                            }
                        }
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

    FILE *fp = fopen("hello.ll", "w");
    fprintf(fp, "%s", ss.str().c_str());
    fclose(fp);

    return 0;
}
