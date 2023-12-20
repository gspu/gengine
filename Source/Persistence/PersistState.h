#pragma once
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "TextWriter.h"

class PersistState
{
public:
    enum class IOMode
    {
        Read,
        Write
    };
    enum class Format
    {
        Text,
        Binary
    };

    PersistState(const char* fileName, IOMode mode, Format format);
    ~PersistState();

    void XferString(const char* name, const std::string& str);
    void XferStringNoSize(const char* name, const std::string& str);

    void Xfer(const char* name, uint32_t& value);
    void Xfer(const char* name, int32_t& value);

    void Xfer(const char* name, bool& value);

private:
    IOMode mMode = IOMode::Read;
    Format mFormat = Format::Text;

    BinaryReader* mBinaryReader = nullptr;

    BinaryWriter* mBinaryWriter = nullptr;
    TextWriter* mTextWriter = nullptr;
    
    void WriteSectionHeader(const char* header);
};