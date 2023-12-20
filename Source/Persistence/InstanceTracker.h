#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "TypeInfo.h" // Convenient b/c if using InstanceTracker probably also need TYPEINFO_DECL/DEF.
#include "PersistState.h"

template<typename T>
class InstanceTracker
{
public:
    InstanceTracker()
    {
        printf("Create instance.\n");

        // Add this instance to the type's instance list.
        // Note that this means the type MUST have a static TypeInfo called sTypeInfo!
        T::sTypeInfo.AddInstance(static_cast<T*>(this));
    }

    ~InstanceTracker()
    {
        printf("Destroy instance.\n");
        T::sTypeInfo.RemoveInstance(static_cast<T*>(this));
    }

private:
    
};