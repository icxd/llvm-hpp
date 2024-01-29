#include <iostream>
#include <sstream>
#include "llvm.hpp"
using namespace LLVM;

int main(void) {
    std::stringstream ss;

    GlobalVariable my_variable{
        .global_var_name = "my_var",
        .type = I32,
        .initializer_constant = Constant{
            .type = Constant::Type::Integer,
            .int_value = 123,
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
        .parameters = {
            FunctionParameter{ I32, std::make_optional("argc") },
            FunctionParameter{ Type{
                .kind = Type::Kind::Pointer,
                .inner = new Type{
                    .kind = Type::Kind::Pointer,
                    .inner = new Type{
                        .kind = Type::Kind::Integer,
                        .size = 32,
                    }
                }
            }, std::make_optional("argv") }
        },
        .body = {
            BasicBlock{
                .name = "entry",
                .instructions = {
                    Instruction{
                        .type = Instruction::Type::Alloca,
                        .name = "ptr",
                        .var = new Alloca{
                            .type = I32,
                        }
                    },
                    Instruction{
                        .type = Instruction::Type::Store,
                        .var = new Store{
                            .value_type = I32,
                            .value = Constant{
                                .type = Constant::Type::Integer,
                                .int_value = 123,
                            },
                            .point_type = Type{
                                .kind = Type::Kind::Pointer,
                                .inner = new Type{
                                    .kind = Type::Kind::Integer,
                                    .size = 32,
                                }
                            },
                            .point = Constant{
                                .type = Constant::Type::LocalVariable,
                                .variable_name = "ptr"
                            }
                        },
                    },
                    Instruction{
                        .type = Instruction::Type::Load,
                        .name = "val",
                        .var = new Load{
                            .value_type = I32,
                            .point_type = Type{
                                .kind = Type::Kind::Pointer,
                                .inner = new Type{
                                    .kind = Type::Kind::Integer,
                                    .size = 32,
                                }
                            },
                            .point = Constant{
                                .type = Constant::Type::LocalVariable,
                                .variable_name = "ptr"
                            }
                        },
                    },
                    Instruction{
                        .type = Instruction::Type::Ret,
                        .var = new Ret{ I32, std::make_optional(Constant{
                            .type = Constant::Type::LocalVariable,
                            .variable_name = "val",
                        })}
                    }
                }
            },
        },
    };

    ss << generate<GlobalVariable>(my_variable) << "\n";
    ss << generate<ExternalFunction>(puts) << "\n";
    ss << generate<Function>(main_function) << "\n";

    std::cout << ss.str() << "\n";

    return 0;
}
