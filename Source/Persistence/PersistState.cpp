#include "PersistState.h"

#include <string>

PersistState::PersistState(const char* fileName, IOMode mode, Format format) :
    mMode(mode),
    mFormat(format)
{
    if(mMode == IOMode::Read)
    {
        //TODO:
    }
    else
    {
        if(mFormat == Format::Text)
        {
            mTextWriter = new TextWriter(fileName);
        }
        else if(mFormat == Format::Binary)
        {
            mBinaryWriter = new BinaryWriter(fileName);
        }

        // Write out header.
        WriteSectionHeader("Header");
        XferStringNoSize("Product ID", "GK3!");
        XferStringNoSize("Save ID", "Save");

        uint32_t version = 4;
        Xfer("Save Version", version);

        uint32_t saveHeaderSize = 232;
    }
}

PersistState::~PersistState()
{
    delete mBinaryReader;
    delete mBinaryWriter;
    delete mTextWriter;
}

void PersistState::XferString(const char* name, const std::string& str)
{
    if(mMode == IOMode::Read)
    {
        if(mFormat == Format::Text)
        {

        }
        else
        {
            mBinaryWriter->WriteMedString(str);
        }
    }
    else
    {
        if(mFormat == Format::Text)
        {
            mTextWriter->WriteLine(std::string(name) + " = " + str);
        }
        else
        {
            mBinaryWriter->WriteMedString(str);
        }
    }
}

void PersistState::XferStringNoSize(const char* name, const std::string& str)
{
    if(mMode == IOMode::Read)
    {
        if(mFormat == Format::Text)
        {

        }
        else
        {

        }
    }
    else
    {
        if(mFormat == Format::Text)
        {
            mTextWriter->WriteLine(std::string(name) + " = " + str);
        }
        else
        {
            mBinaryWriter->WriteString(str, 4);
        }
    }
}

void PersistState::Xfer(const char* name, uint32_t& value)
{
    if(mMode == IOMode::Read)
    {
        if(mFormat == Format::Text)
        {

        }
        else
        {

        }
    }
    else
    {
        if(mFormat == Format::Text)
        {
            mTextWriter->WriteLine(std::string(name) + " = " + std::to_string(value));
        }
        else
        {
            mBinaryWriter->WriteUInt(value);
        }
    }
}

void PersistState::Xfer(const char* name, int32_t& value)
{
    if(mMode == IOMode::Read)
    {
        if(mFormat == Format::Text)
        {

        }
        else
        {

        }
    }
    else
    {
        if(mFormat == Format::Text)
        {
            mTextWriter->WriteLine(std::string(name) + " = " + std::to_string(value));
        }
        else
        {
            mBinaryWriter->WriteInt(value);
        }
    }
}

void PersistState::Xfer(const char* name, bool& value)
{
    
}

void PersistState::WriteSectionHeader(const char* name)
{
    if(mTextWriter != nullptr)
    {
        mTextWriter->WriteLine("[" + std::string(name) + "]");
    }
}