#include "InstanceDatabase.h"

#include "TypeInfo.h"

void InstanceDatabase::Persist()
{
    for(TypeInfoBase* type : mTypes)
    {
        type->Persist();
    }
}