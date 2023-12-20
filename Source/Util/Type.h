//
// Clark Kromenaker
//
// Defines a "Type" and macros for adding runtime type info (RTTI) to a base class and subclasses.
//
#include <cstddef>
#include <functional>   // for std::hash support
#include <string>

// A class's type will just be a size_t alias for now.
typedef std::size_t Type;

// Converts passed in symbol to a string (MyClass => "MyClass").
#define SYMBOL_TO_STR(x) #x

// Generates a Type for a particular symbol.
// Allows a "Type" to be generated for *any* class, not just those that use TYPE_DECL/DEF macros below.
// Note: std::hash will not necessarily return the same value across runs of a program!
#define GENERATE_TYPE(x) std::hash<std::string>()(SYMBOL_TO_STR(x));

// USAGE: Add TYPE_DECL macro inside the class declaration.
#define TYPE_DECL()                                                 \
private:                                                            \
    static const Type type;                                         \
    static const char* typeName;                                    \
                                                                    \
public:                                                             \
    static Type GetType() { return type; }                          \
    static const char* GetTypeName() { return typeName; }           \
    virtual bool IsTypeOf(const Type t) const;
    //TODO: static PClassType* CreateInstance() { return new PClassType(); }\

// USAGE: Add TYPE_DEF macro inside the class implementation file.
// USAGE: If it is a child class, use TYPE_DEF_CHILD instead.
#define TYPE_DEF(PClass)                                                                \
const Type PClass::type = std::hash<std::string>()(SYMBOL_TO_STR(PClass));              \
const char* PClass::typeName = SYMBOL_TO_STR(PClass);                                   \
bool PClass::IsTypeOf(const Type t) const {                                             \
    return t == type;                                                                   \
}

#define TYPE_DEF_CHILD(PParentClass, PClass)                                              \
const Type PClass::type = std::hash<std::string>()(SYMBOL_TO_STR(PClass));                \
const char* PClass::typeName = SYMBOL_TO_STR(PClass);                                     \
bool PClass::IsTypeOf(const Type t) const {                                               \
    return t == PClass::type ? true : PParentClass::IsTypeOf(t);                          \
}