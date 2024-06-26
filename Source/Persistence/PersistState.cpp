#include "PersistState.h"

#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "IniParser.h"
#include "IniWriter.h"

PersistState::PersistState(const char* filePath, PersistFormat format, PersistMode mode)
{
    if(mode == PersistMode::Save)
    {
        if(format == PersistFormat::Text)
        {
            mIniWriter = new IniWriter(filePath);
        }
        else if(format == PersistFormat::Binary)
        {
            mBinaryWriter = new BinaryWriter(filePath);
        }
    }
    else if(mode == PersistMode::Load)
    {
        if(format == PersistFormat::Text)
        {
            mIniReader = new IniParser(filePath);
        }
        else if(format == PersistFormat::Binary)
        {
            mBinaryReader = new BinaryReader(filePath);
        }
    }
}

PersistState::~PersistState()
{
    delete mBinaryReader;
    delete mBinaryWriter;
    delete mIniReader;
    delete mIniWriter;
}

void PersistState::Xfer(const char* name, char* value, size_t valueSize)
{
    if(mBinaryReader != nullptr)
    {
        mBinaryReader->Read(value, valueSize);
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->Write(value, valueSize);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        //TODO
    }
}

void PersistState::Xfer(const char* name, std::string& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadString32();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteMedString(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, value.c_str());
    }   
}

void PersistState::Xfer(const char* name, int32_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadInt();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteInt(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, uint32_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadUInt();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteUInt(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, int16_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadShort();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteShort(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}

void PersistState::Xfer(const char* name, uint16_t& value)
{
    if(mBinaryReader != nullptr)
    {
        value = mBinaryReader->ReadUShort();
    }
    else if(mBinaryWriter != nullptr)
    {
        mBinaryWriter->WriteUShort(value);
    }
    else if(mIniReader != nullptr)
    {
        //TODO
    }
    else if(mIniWriter != nullptr)
    {
        mIniWriter->WriteKeyValue(name, std::to_string(value).c_str());
    }
}
