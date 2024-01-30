#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include "llvm.hpp"

namespace LLVM {

[[noreturn]] void panic(usz line, const char *file, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "PANIC: %s:%lu: ", file, line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

template <typename T> std::string generate([[maybe_unused]] T t) {
    PANIC("%s cannot be called for an arbitrary type", __PRETTY_FUNCTION__);
}

template <> std::string generate<Linkage>(Linkage linkage) {
    switch (linkage) {
        case Linkage::Private: return "private";
        case Linkage::Internal: return "internal";
        case Linkage::AvailableExternally: return "available_externally";
        case Linkage::Linkonce: return "linkonce";
        case Linkage::Weak: return "weak";
        case Linkage::Common: return "common";
        case Linkage::Appending: return "appending";
        case Linkage::ExternWeak: return "extern_weak";
        case Linkage::LinkonceODR: return "linkonce_odr";
        case Linkage::WeakODR: return "weak_odr";
        case Linkage::External: return "external";
    }
}

template <> std::string generate<PreemptionSpecifier>([[maybe_unused]] PreemptionSpecifier specifier) {
    std::stringstream ss;
    return ss.str();
}

template <> std::string generate<Visibility>([[maybe_unused]] Visibility visibility) {
    std::stringstream ss;
    return ss.str();
}

template <> std::string generate<DLLStorageClass>(DLLStorageClass storage_class) {
    switch (storage_class) {
        case DLLStorageClass::DLLImport: return "dllimport";
        case DLLStorageClass::DLLExport: return "dllexport";
    }
}

template <> std::string generate<ThreadLocal>(ThreadLocal thread_local_) {
    std::stringstream ss;
    ss << "thread_local(";
    switch (thread_local_) {
        case ThreadLocal::LocalDynamic: ss << "localdynamic"; break;
        case ThreadLocal::InitialExec: ss << "initialexec"; break;
        case ThreadLocal::LocalExec: ss << "localexec"; break;
    }
    ss << ")";
    return ss.str();
}

template <> std::string generate<CodeModel>(CodeModel model) {
    switch (model) {
        case CodeModel::Tiny: return "tiny";
        case CodeModel::Small: return "small";
        case CodeModel::Kernel: return "kernel";
        case CodeModel::Medium: return "medium";
        case CodeModel::Large: return "large";
    }
}

template <> std::string generate<CallingConvention>(CallingConvention cc) {
    switch (cc) {
        case CallingConvention::C: return "c";
        case CallingConvention::Fast: return "fast";
        case CallingConvention::Cold: return "cold";
        case CallingConvention::GHC: return "ghc";
        case CallingConvention::CC11: return "cc11";
        case CallingConvention::AnyReg: return "anyreg";
        case CallingConvention::PreserveMost: return "preservemost";
        case CallingConvention::PreserveAll: return "preserveall";
        case CallingConvention::CXXFastTLS: return "cxxfasttls";
        case CallingConvention::Tail: return "tail";
        case CallingConvention::Swift: return "swift";
        case CallingConvention::SwiftTail: return "swifttail";
        case CallingConvention::CFGuardCheck: return "cfguardcheck";
    }
}

template <> std::string generate<Type>(Type type) {
    std::stringstream ss;
    switch (type.kind) {
        case Type::Kind::Integer: ss << "i" << type.size; break;
        case Type::Kind::Pointer: ss << generate<Type>(*type.inner) << "*"; break;
        case Type::Kind::Array: ss << "[" << type.size << " x " << generate<Type>(*type.inner) << "]"; break;
        default: PANIC("TODO!", "");
    }
    return ss.str();
}

template <> std::string generate<Constant>(Constant constant) {
    std::stringstream ss;
    switch (constant.type) {
        case Constant::Type::Boolean: ss << constant.bool_value; break;
        case Constant::Type::Integer: ss << constant.int_value; break;
        case Constant::Type::Float: ss << constant.float_value; break;
        case Constant::Type::String: ss << "c\"" << constant.string_value << "\""; break;
        case Constant::Type::LocalVariable: ss << "%" << constant.variable_name; break;
        case Constant::Type::GlobalVariable: ss << "@" << constant.variable_name; break;
        default: PANIC("TODO!", "");
    }
    return ss.str();
}

template <> std::string generate<Instruction>(Instruction inst) {
    std::stringstream ss;
    if (inst.name.has_value())
        ss << "%" << inst.name.value() << " = ";
    switch (inst.type) {
        case Instruction::Type::Ret: {
            auto ret = *std::get<InstructionDetails::Ret *>(inst.var);
            ss << "ret " << generate<Type>(ret.type);
            if (ret.value.has_value())
                ss << " " << generate<Constant>(ret.value.value());
        } break;
        case Instruction::Type::Alloca: {
            auto alloca = *std::get<InstructionDetails::Alloca *>(inst.var);
            ss << "alloca ";
            if (alloca.inalloca) ss << "inalloca ";
            ss << generate<Type>(alloca.type);
            if (alloca.elements > 1)
                ss << ", " << generate<Type>(alloca.type) << " " << alloca.elements;
            if (alloca.alignment.has_value()) ss << ", align " << alloca.alignment.value();
            if (alloca.addrspace.has_value()) ss << ", addrspace(" << alloca.addrspace.value() << ")";
        } break;
        case Instruction::Type::Load: {
            auto load = *std::get<InstructionDetails::Load *>(inst.var);
            ss << "load " << (load.volatile_ ? "volatile " : "");
            ss << generate<Type>(load.value_type) << ", ";
            ss << generate<Type>(load.point_type) << " " << generate<Constant>(load.point);
            if (load.alignment.has_value()) ss << ", align " << load.alignment.value();
        } break;
        case Instruction::Type::Store: {
            auto store = *std::get<InstructionDetails::Store *>(inst.var);
            ss << "store " << (store.volatile_ ? "volatile " : "");
            ss << generate<Type>(store.value_type) << " " << generate<Constant>(store.value) << ", ";
            ss << generate<Type>(store.point_type) << " " << generate<Constant>(store.point);
            if (store.alignment.has_value()) ss << ", align " << store.alignment.value();
        } break;
        case Instruction::Type::GetElementPtr: {
            // %msg_ptr = getelementptr [13 x i8], [13 x i8]* @msg, i32 0, i32 0
            auto gep = *std::get<InstructionDetails::GetElementPtr *>(inst.var);
            ss << "getelementptr " << generate<Type>(gep.type);
            ss << ", " << generate<Type>(gep.ptr_type) << " " << generate<Constant>(gep.ptr_value);
            ss << ", i32 0, i32 0"; // this is a temporary solution to missing fields.
        } break;
        case Instruction::Type::Call: {
            auto call = *std::get<InstructionDetails::Call *>(inst.var);
            if (call.tail.has_value()) ss << generate<InstructionDetails::Call::TailCall>(call.tail.value()) << " ";
            ss << "call ";
            if (call.calling_convention.has_value()) ss << generate<CallingConvention>(call.calling_convention.value()) << " ";
            if (call.addrspace.has_value()) ss << "addrspace(" << call.addrspace.value() << ") ";
            ss << generate<Type>(call.return_type) << " @" << call.name << "(";
            for (int i = 0; i < call.arguments.size(); i++) {
                InstructionDetails::Call::Argument argument = call.arguments.at(i);
                ss << generate<Type>(argument.type) << " " << generate<Constant>(argument.value);
                if (i < call.arguments.size() - 1)
                    ss << ", ";
            }
            ss << ")";
        } break;
        default: PANIC("TODO!", "");
    }
    return ss.str();
}

template <> std::string generate<InstructionDetails::Call::TailCall>(InstructionDetails::Call::TailCall tc) {
    switch (tc) {
        case InstructionDetails::Call::TailCall::Tail: return "tail";
        case InstructionDetails::Call::TailCall::MustTail: return "musttail";
        case InstructionDetails::Call::TailCall::NoTail: return "notail";
    }
}

template <> std::string generate<BasicBlock>(BasicBlock bb) {
    std::stringstream ss;
    ss << bb.name << ":\n";
    for (const auto& instruction : bb.instructions)
        ss << "    " << generate<Instruction>(instruction) << "\n";
    return ss.str();
}

template <> std::string generate<GlobalVariable>(GlobalVariable var) {
    std::stringstream ss;
    ss << "@" << var.global_var_name << " = ";
    if (var.linkage.has_value()) ss << generate<Linkage>(var.linkage.value()) << " ";
    if (var.preemption_specifier.has_value())
        ss << generate<PreemptionSpecifier>(var.preemption_specifier.value()) << " ";
    if (var.visibility.has_value()) ss << generate<Visibility>(var.visibility.value()) << " ";
    if (var.dll_storage_class.has_value()) ss << generate<DLLStorageClass>(var.dll_storage_class.value()) << " ";
    if (var.thread_local_.has_value()) ss << generate<ThreadLocal>(var.thread_local_.value()) << " ";
    if (var.unnamed_addr) ss << "unnamed_addr ";
    if (var.local_unnamed_addr) ss << "local_unnamed_addr ";
    if (var.addr_space.has_value()) ss << "addrspace(" << var.addr_space.value() << ") ";
    if (var.externally_initialized) ss << "external ";
    if (var.global) ss << "global ";
    else ss << "constant ";
    ss << generate<Type>(var.type) << " " << generate<Constant>(var.initializer_constant);

    if (var.section.has_value()) ss << ", section \"" << var.section.value() << "\"";
    if (var.partition.has_value()) ss << ", partition \"" << var.partition.value() << "\"";
    if (var.alignment.has_value()) ss << ", align " << var.alignment.value();
    if (var.code_model.has_value()) ss << ", codemodel \"" << generate<CodeModel>(var.code_model.value()) << "\"";
    if (var.no_sanitize_address) ss << ", no_sanitize_address";
    if (var.no_sanitize_hwaddress) ss << ", no_sanitize_hwaddress";
    if (var.sanitize_address_dyninit) ss << ", sanitize_address_dyninit";
    if (var.sanitize_memtag) ss << ", sanitize_memtag";

    ss << "\n";

    return ss.str();
}

template <> std::string generate<Function>(Function fn) {
    std::stringstream ss;

    ss << "define ";
    if (fn.linkage.has_value()) ss << generate<Linkage>(fn.linkage.value()) << " ";
    if (fn.preemption_specifier.has_value())
        ss << generate<PreemptionSpecifier>(fn.preemption_specifier.value()) << " ";
    if (fn.visibility.has_value()) ss << generate<Visibility>(fn.visibility.value()) << " ";
    if (fn.dll_storage_class.has_value()) ss << generate<DLLStorageClass>(fn.dll_storage_class.value()) << " ";
    if (fn.calling_convention.has_value()) ss << generate<CallingConvention>(fn.calling_convention.value()) << " ";

    ss << generate<Type>(fn.return_type) << " @" << fn.function_name;
    ss << "(";

    for (int i = 0; i < fn.parameters.size(); i++) {
        auto parameter = fn.parameters.at(i);
        ss << generate<Type>(parameter.type);
        if (parameter.name.has_value()) ss << " %" << parameter.name.value();
        if (i < fn.parameters.size() - 1)
            ss << ", ";
    }

    ss << ")";

    if (fn.unnamed_addr) ss << "unnamed_addr ";
    if (fn.local_unnamed_addr) ss << "local_unnamed_addr ";
    if (fn.addr_space.has_value()) ss << "addrspace(" << fn.addr_space.value() << ") ";
    if (fn.section.has_value()) ss << ", section \"" << fn.section.value() << "\"";
    if (fn.partition.has_value()) ss << ", partition \"" << fn.partition.value() << "\"";
    if (fn.alignment.has_value()) ss << ", align " << fn.alignment.value();

    ss << " {\n";

    for (const auto& bb : fn.body)
        ss << generate<BasicBlock>(bb);

    ss << "}\n";

    return ss.str();
}

template <> std::string generate<ExternalFunction>(ExternalFunction fn) {
    std::stringstream ss;

    ss << "declare ";
    if (fn.linkage.has_value()) ss << generate<Linkage>(fn.linkage.value()) << " ";
    if (fn.visibility.has_value()) ss << generate<Visibility>(fn.visibility.value()) << " ";
    if (fn.dll_storage_class.has_value()) ss << generate<DLLStorageClass>(fn.dll_storage_class.value()) << " ";
    if (fn.calling_convention.has_value()) ss << generate<CallingConvention>(fn.calling_convention.value()) << " ";

    ss << generate<Type>(fn.return_type) << " @" << fn.function_name;
    ss << "(";

    for (int i = 0; i < fn.parameters.size(); i++) {
        auto parameter = fn.parameters.at(i);
        ss << generate<Type>(parameter.type);
        if (parameter.name.has_value()) ss << " %" << parameter.name.value();
        if (i < fn.parameters.size() - 1)
            ss << ", ";
    }

    ss << ")";

    if (fn.unnamed_addr) ss << "unnamed_addr ";
    if (fn.local_unnamed_addr) ss << "local_unnamed_addr ";
    if (fn.alignment.has_value()) ss << ", align " << fn.alignment.value();

    return ss.str();
}

} // namespace LLVM
