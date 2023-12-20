//
// Clark Kromenaker
//
// The instance database tracks instances of objects by type.
// You can think of this as a 2D array, with "type" on one axis and "instance" on the other.
//
// The primary value in tracking instances in a database like this is to make save/load easier.
// When we want to save or load the game, we can simply iterate all the instances here.
//
#pragma once
#include <vector>

class TypeInfoBase;

class InstanceDatabase
{
public:
    static InstanceDatabase& Get()
    {
        // A global instance won't work for InstanceDatabase due to order of initialization issues.
        // In short, static TypeInfos might try to register *before* the InstanceDatabase is initialized!
        
        // To fix that, we take more control over initialization order by using a static member variable here.
        static InstanceDatabase database;
        return database;
    }

    void RegisterType(TypeInfoBase* typeInfo)
    {
        // Since this is only called from TypeInfo constructor, we can assume no duplicates.
        mTypes.push_back(typeInfo);
    }

    void Persist();

private:
    // All the types that have registered into the database.
    // Each TypeInfo stores a list of instances.
    // We need to use a base class (and pointers) because TypeInfo is generic!
    std::vector<TypeInfoBase*> mTypes;
};