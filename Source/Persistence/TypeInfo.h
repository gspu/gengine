//
// Clark Kromenaker
//
// TypeInfo is a static member of a class that tells us some useful data about its type.
// This is *kind of* RTTI related, but it's meant more for the save/load system to hook into things.
//  
// 
#pragma once
#include "InstanceDatabase.h"
#include "PersistState.h"

// A macro that hides the complex syntax of calling a member function pointer on an object.
// Recommended by https://isocpp.org/wiki/faq/pointers-to-members).
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))

#define TYPEINFO_DECL(PClass) \
public: \
    static TypeInfo<PClass> sTypeInfo;

#define TYPEINFO_DEF(PClass, PPersistFunc) \
TypeInfo<PClass> PClass::sTypeInfo(&PPersistFunc);

// This base class is required so we can store a list of TypeInfo in the InstanceDatabase.
class TypeInfoBase
{
public:
    virtual ~TypeInfoBase() = default;
    virtual void Persist() = 0;
};

template<typename T>
class TypeInfo : public TypeInfoBase
{
public:
    TypeInfo(void(T::* func)(PersistState&, uint32_t)) :
        mPersistFunc(func)
    {
        // Register our type in the instance database.
        InstanceDatabase::Get().RegisterType(this);
    }

    void AddInstance(T* instance)
    {
        // Add instance.
        // Since this is ONLY called in InstanceTracker constructor, let's assume no dupe checking is needed.
        mInstances.push_back(instance);
    }

    void RemoveInstance(T* instance)
    {
        // Find and remove instance.
        auto it = std::find(mInstances.begin(), mInstances.end(), instance);
        if(it != mInstances.end())
        {
            mInstances.erase(it);
        }
    }

    void Persist() override
    {
        // We need a persist function to save/load instances.
        if(mPersistFunc != nullptr)
        {
            // Iterate each instance and either save or load!
            for(T* instance : mInstances)
            {
                //PersistState state;
                //CALL_MEMBER_FN(*instance, mPersistFunc)(state, 1);
            }
        }
    }

private:
    // A list of instances of this type.
    std::vector<T*> mInstances;

    // A pointer to a function used to persist (aka save/load) instances of this type.
    typedef void(T::*PersistFunc)(PersistState&, uint32_t);
    PersistFunc mPersistFunc = nullptr;
};