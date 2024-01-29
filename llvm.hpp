#ifndef LLVM_H
#define LLVM_H

#include <string>
#include <optional>
#include <vector>
#include <variant>

namespace LLVM {

using usz = unsigned long;
template <typename T> using Opt = std::optional<T>;
inline constexpr auto None = std::nullopt;
template <typename T> using Vec = std::vector<T>;
template <typename...Args> using Var = std::variant<Args...>;

[[noreturn]] void panic(usz, const char *, const char *,...);
#define PANIC(fmt, ...) panic(__LINE__, __FILE__, fmt, __VA_ARGS__);

template <typename T> std::string generate(T);

enum class Linkage {
    Private, Internal, AvailableExternally,
    Linkonce, Weak, Common, Appending,
    ExternWeak, LinkonceODR, WeakODR, External
};
enum class PreemptionSpecifier { DSOPreemptable, DSOLocal };
enum class Visibility { Default, Hidden, Protected };
enum class DLLStorageClass { DLLImport, DLLExport };
enum class ThreadLocal { LocalDynamic, InitialExec, LocalExec };
enum class CodeModel { Tiny, Small, Kernel, Medium, Large };
enum class CallingConvention {
    C,
    Fast,
    Cold,
    GHC,
    CC11,
    AnyReg,
    PreserveMost,
    PreserveAll,
    CXXFastTLS,
    Tail,
    Swift,
    SwiftTail,
    CFGuardCheck,
    // TODO: cc <n>
};

template <> std::string generate<Linkage>(Linkage);
template <> std::string generate<PreemptionSpecifier>(PreemptionSpecifier);
template <> std::string generate<Visibility>(Visibility);
template <> std::string generate<DLLStorageClass>(DLLStorageClass);
template <> std::string generate<ThreadLocal>(ThreadLocal);
template <> std::string generate<CodeModel>(CodeModel);
template <> std::string generate<CallingConvention>(CallingConvention);

struct Type {
    enum class Kind {
        Void, Function, Integer,
        Half, BFloat, Float, Double, fp128, x86_fp80, ppc_fp128,
        x86_amx, x86_mmx,
        Pointer,
        Vector, Label, Array,
        Structure, OpaqueStructure,
    };

    Kind kind;
    Type *inner{nullptr}; // For Pointer, Vector, and Array
    usz size{0}; // For Vector, Array, and Integer
};

inline constexpr Type I32 = Type{ .kind = Type::Kind::Integer, .size = 32 };

template <> std::string generate<Type>(Type);

struct Constant {
    enum class Type { Boolean, Integer, Float, Null, String, LocalVariable, GlobalVariable };

    Type type;
    union {
        bool bool_value;
        long long int_value;
        double float_value;
        const char *string_value;
        const char *variable_name;
    };
};

template <> std::string generate<Constant>(Constant);

struct FunctionParameter {
    Type type;
    // TODO: parameter attrs (what in the fuck nuts is an attribute and why the fuck is it not documented.)
    Opt<std::string> name{None};
};

struct Ret { ::LLVM::Type type; Opt<Constant> value{None}; };
struct Alloca {
    bool inalloca{false};
    ::LLVM::Type type;
    usz elements{1};
    Opt<usz> alignment{None};
    Opt<usz> addrspace{None};
};
struct Load {
    bool volatile_{false};
    ::LLVM::Type value_type;
    ::LLVM::Type point_type; Constant point;
    Opt<usz> alignment{None};
};
struct Store {
    bool volatile_{false};
    ::LLVM::Type value_type; Constant value;
    ::LLVM::Type point_type; Constant point;
    Opt<usz> alignment{None};
};
struct GetElementPtr {
    // TODO: <result> = getelementptr <ty>, ptr <ptrval>{, [inrange] <ty> <idx>}*
    // TODO: <result> = getelementptr inbounds <ty>, ptr <ptrval>{, [inrange] <ty> <idx>}*
    // <result> = getelementptr <ty>, <N x ptr> <ptrval>, [inrange] <vector index type> <idx>
    Type type;
    Type ptr_type;
    Constant ptr_value;
    // TODO: vector index (???)
};
struct Call {
    // <result> = [tail | musttail | notail ] call [fast-math flags] [cconv] [ret attrs] [addrspace(<num>)]
    //            <ty>|<fnty> <fnptrval>(<function args>) [fn attrs] [ operand bundles ]
    enum class TailCall { Tail, MustTail, NoTail };

    Opt<TailCall> tail{None};
    // TODO: fast-math flags
    Opt<CallingConvention> calling_convention{None};
    // TODO: ret attrs (WHAT THE FUCK IS A RETURN ATTRIBUTE)
    Opt<usz> addr_space{None};
    Type return_type;
    // TODO: fnty (???)
    std::string name;
    Vec<FunctionParameter> arguments{};
    // TODO: fn attrs
    // TODO: operand bundles
};

// https://llvm.org/docs/LangRef.html#instruction-reference
struct Instruction {
    enum class Type {
        // Terminator Instructions
        Ret, Br, Switch, IndirectBr, Invoke, Callbr, Resume, Catchswitch, Catchret, Cleanupret, Unreachable,
        // Unary Operations
        Fneg,
        // Binary Operations
        Add, Fadd, Sub, Fsub, Mul, Fmul, Udiv, Sdiv, Fdiv, Urem, Srem, Frem,
        // Bitwise Binary Operations
        Shl, Lshr, Ashr, And, Or, Xor,
        // Vector Operations
        ExtractElement, InsertElement, ShuffleVector,
        // Aggregate Operations
        ExtractValue, InsertValue,
        // Memery access and Adressing Operations
        Alloca, Load, Store, Fence, Cmpxchg, AtomicRmw, GetElementPtr,
        // Conversion Operations
        Trunc, Zext, Sext, Fptrunc, Fpext, Fptoui, Fptosi, Uitofp, Sitofp, Ptrtoint, Inttoptr, Bitcast, Addrspacecast,
        // Other Operations
        Icmp, Fcmp, Phi, Select, Freeze, Call, VaArg, LandingPad, CatchPad, CleanupPad,
    };

    Type type;
    Opt<std::string> name{None};
    Var<Ret *, Alloca *, Load *, Store *> var;
};

template <> std::string generate<Instruction>(Instruction);

struct BasicBlock {
    std::string name;
    Vec<Instruction> instructions;
};

template <> std::string generate<BasicBlock>(BasicBlock);

struct GlobalVariable {
    std::string global_var_name /* REQUIRED */;
    Opt<Linkage> linkage{None};
    Opt<PreemptionSpecifier> preemption_specifier{None};
    Opt<Visibility> visibility{None};
    Opt<DLLStorageClass> dll_storage_class{None};
    Opt<ThreadLocal> thread_local_{None};
    bool unnamed_addr{false}, local_unnamed_addr{false};
    Opt<usz> addr_space{None};
    bool externally_initialized{false};
    bool global{false}; // If false, then it's constant
    Type type /* REQUIRED */;
    Constant initializer_constant /* REQUIRED */;

    Opt<std::string> section{None}, partition{None};
    // TODO: Comdat
    Opt<usz> alignment{None};
    Opt<CodeModel> code_model{None};
    bool no_sanitize_address{false},
         no_sanitize_hwaddress{false},
         sanitize_address_dyninit{false},
         sanitize_memtag{false};
    // TODO: metadata
};

template <> std::string generate<GlobalVariable>(GlobalVariable);

// https://llvm.org/docs/LangRef.html#functions 
struct Function {
    Opt<Linkage> linkage{None};
    Opt<PreemptionSpecifier> preemption_specifier{None};
    Opt<Visibility> visibility{None};
    Opt<DLLStorageClass> dll_storage_class{None};
    Opt<CallingConvention> calling_convention{None};
    // TODO: ret attrs (no idea what this is, doesn't help that it's not documented from what i can see.)

    Type return_type; /* required */
    std::string function_name; /* required */
    Vec<FunctionParameter> parameters{};

    bool unnamed_addr{false};
    bool local_unnamed_addr{false};
    Opt<usz> addr_space{None};
    // TODO: fn attrs (again, no idea what this is)
    Opt<std::string> section{None}, partition{None};
    // TODO: Comdat
    Opt<usz> alignment{None};
    // TODO: gc (garbage collector)
    // TODO: prefix constant
    // TODO: prologue constant
    // TODO: personality constant
    // TODO: metadata
    Vec<BasicBlock> body;
};

template <> std::string generate<Function>(Function);

struct ExternalFunction {
    Opt<Linkage> linkage{None};
    Opt<Visibility> visibility{None};
    Opt<DLLStorageClass> dll_storage_class{None};
    Opt<CallingConvention> calling_convention{None};
    // TODO: ret attrs (no idea what this is, doesn't help that it's not documented from what i can see.)

    Type return_type; /* required */
    std::string function_name; /* required */
    Vec<FunctionParameter> parameters{};

    bool unnamed_addr{false};
    bool local_unnamed_addr{false};
    Opt<usz> alignment{None};
    // TODO: gc (garbage collector)
    // TODO: prefix constant
    // TODO: prologue constant
};

template <> std::string generate<ExternalFunction>(ExternalFunction);

} // namespace LLVM

#endif // LLVM_H
