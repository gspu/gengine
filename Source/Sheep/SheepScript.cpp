//
//  SheepScript.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//
#include "SheepScript.h"

#include <iostream>
#include <fstream>

#include "BinaryReader.h"
#include "SheepScriptBuilder.h"
#include "StringUtil.h"

SheepScript::SheepScript(const std::string& name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

SheepScript::SheepScript(const std::string& name, SheepScriptBuilder& builder) : Asset(name)
{
    // Just copy these directly.
    mSysImports = builder.GetSysImports();
    mStringConsts = builder.GetStringConsts();
    mVariables = builder.GetVariables();
    mFunctions = builder.GetFunctions();
	
    // Bytecode needs to convert from std::vector to byte array.
    std::vector<char> bytecodeVec = builder.GetBytecode();
    mBytecodeLength = (int)bytecodeVec.size();
    mBytecode = new char[mBytecodeLength];
    std::copy(bytecodeVec.begin(), bytecodeVec.end(), mBytecode);
}

SysImport* SheepScript::GetSysImport(int index)
{
    if(index < 0 || index >= mSysImports.size()) { return nullptr; }
    return &mSysImports[index];
}

std::string* SheepScript::GetStringConst(int offset)
{
    auto it = mStringConsts.find(offset);
    if(it != mStringConsts.end())
    {
        return &it->second;
    }
	return nullptr;
}

int SheepScript::GetFunctionOffset(std::string functionName)
{
	// All sheep functions are case-insensitive. So, we only use lowercase names for consistency.
	std::string lowerFunctionName = StringUtil::ToLowerCopy(functionName);
	
	// Find it and return it, or fail with -1 offset.
    auto it = mFunctions.find(lowerFunctionName);
    if(it != mFunctions.end())
    {
        return it->second;
    }
    return -1;
}

void SheepScript::Dump()
{
    std::cout << "Dumping sheep " << mName << std::endl << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
    std::cout << "Component   : GK3Sheep" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
}

void SheepScript::ParseFromData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 8 bytes: file identifier "GK3Sheep".
    std::string identifier = reader.ReadString(8);
    if(identifier != "GK3Sheep")
    {
        std::cout << "Not valid GK3Sheep data!" << std::endl;
        return;
    }
    
    // 4 bytes: maybe a format version number?
    reader.Skip(4);
    
    // 4 bytes: size of header
    int headerSize = reader.ReadInt();
    
    // 4 bytes: size of header, duped
    // 4 bytes: size of file contents, minus 44 byte header
    reader.Skip(8);
    
    int dataCount = reader.ReadInt();
    std::vector<int> dataOffsets(dataCount);
    for(int i = 0; i < dataCount; i++)
    {
        dataOffsets[i] = reader.ReadInt();
    }
    
    for(int i = 0; i < dataCount; i++)
    {
        int offset = dataOffsets[i] + headerSize;
        reader.Seek(offset);
        
        std::string section = reader.ReadString(12);
        if(section == "SysImports")
        {
            ParseSysImportsSection(reader);
        }
        else if(section == "StringConsts")
        {
            ParseStringConstsSection(reader);
        }
        else if(section == "Variables")
        {
            ParseVariablesSection(reader);
        }
        else if(section == "Functions")
        {
            ParseFunctionsSection(reader);
        }
        else if(section == "Code")
        {
            ParseCodeSection(reader);
        }
        else
        {
            std::cout << "Unknown component: " << section << std::endl;
        }
    }
}

void SheepScript::ParseSysImportsSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    // Don't need byte size of all imports.
    reader.Skip(12);
    
    int functionCount = reader.ReadInt();
    
    // Don't really need offset values for functions.
    reader.Skip(4 * functionCount);
    
    for(int i = 0; i < functionCount; i++)
    {
        SysImport import;
        
        // Read in name from length.
        // Length is always one more, due to null terminator.
        short nameLength = reader.ReadShort();
        import.name = reader.ReadString(nameLength + 1);;
        
        import.returnType = reader.ReadByte();
        
        char argumentCount = reader.ReadByte();
        for(int j = 0; j < argumentCount; j++)
        {
            import.argumentTypes.push_back(reader.ReadByte());
        }
        
        mSysImports.push_back(import);
    }
}

void SheepScript::ParseStringConstsSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    reader.Skip(8);
    
    int contentSize = reader.ReadInt();
    int stringCount = reader.ReadInt();
    std::vector<int> stringOffsets(stringCount);
    for(int i = 0; i < stringCount; i++)
    {
        stringOffsets[i] = reader.ReadInt();
    }
    
    int dataBaseOffset = reader.GetPosition();
    for(int i = 0; i < stringCount; i++)
    {
        int startOffset = dataBaseOffset + stringOffsets[i];
        int endOffset;
        if(i < stringCount - 1)
        {
            endOffset = dataBaseOffset + stringOffsets[i + 1];
        }
        else
        {
            endOffset = dataBaseOffset + contentSize;
        }
        std::string str = reader.ReadString(endOffset - startOffset);
        mStringConsts[startOffset - dataBaseOffset] = str;
    }
}

void SheepScript::ParseVariablesSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    // Don't need byte size of all variables.
    reader.Skip(12);
    
    // Don't really need offset values for variables.
    int variableCount = reader.ReadInt();
    reader.Skip(4 * variableCount);
    
    // Read in each variable.
    for(int i = 0; i < variableCount; i++)
    {
        SheepValue value;
        
        // Read in name from length.
        // Length is always one more, due to null terminator.
        short nameLength = reader.ReadShort();
        std::string name = reader.ReadString(nameLength + 1);
        
        // Type is either int, float, or string.
        int type = reader.ReadInt();
        if(type == 1)
        {
            value.type = SheepValueType::Int;
            value.intValue = reader.ReadInt();
        }
        else if(type == 2)
        {
            value.type = SheepValueType::Float;
            value.floatValue = reader.ReadFloat();
        }
        else if(type == 3)
        {
            value.type = SheepValueType::String;
            reader.ReadInt();
            value.stringValue = nullptr;
        }
        else
        {
            std::cout << "Unknown type: " << type << std::endl;
        }
        mVariables.push_back(value);
    }
}

void SheepScript::ParseFunctionsSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    // Don't need byte size of all functions.
    reader.Skip(12);
    
    int functionCount = reader.ReadInt();
    
    // Don't really need offset values for functions.
    reader.Skip(4 * functionCount);
    
    for(int i = 0; i < functionCount; i++)
    {
        // Read in name from length.
        // Length is always one more, due to null terminator.
        short nameLength = reader.ReadShort();
        std::string name = reader.ReadString(nameLength + 1);
		
		// Always lowercase the name, since sheepscript is case-insensitive.
		// This ensures we can always lookup, regardless of given name case.
		StringUtil::ToLower(name);
		
		// 2 bytes: unknown
        reader.Skip(2);
		
		// 4 bytes: code offset for this function.
        int codeOffset = reader.ReadInt();
		
		// Save mapping of function name to code offset.
        mFunctions[name] = codeOffset;
    }
}

void SheepScript::ParseCodeSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header sizes.
    reader.Skip(8);
    
    // Get size in bytes of code section after header.
    mBytecodeLength = reader.ReadInt();
    
    // Next is number of code sections - should always be one.
    int codeContentCount = reader.ReadInt();
    if(codeContentCount != 1)
    {
        std::cout << "Expected one!" << std::endl;
        return;
    }
    
    // Next is the offset to each code content. But since
    // there's only one, this will always be zero.
    reader.ReadInt();
    
    // The rest is just bytecode!
    mBytecode = new char[mBytecodeLength];
    reader.Read(mBytecode, mBytecodeLength);
}

/**
 * BELOW HERE: MESSY/EXPERIMENTAL DECOMPILER CODE!
 * Converts a compiled sheepscript back to human readable form (more or less).
 */

void WriteOut(std::ofstream& out, const std::string& str, int indentLevel)
{
    for(int i = 0; i < indentLevel; i++)
    {
        out << "    ";
    }
    out << str << std::endl;
}

void SheepScript::Decompile()
{
    // Just use the asset's name in this case - writes to exe folder.
    Decompile(GetName());
}

void SheepScript::Decompile(const std::string& filePath)
{
    // Create reader for the bytecode.
    BinaryReader reader(mBytecode, mBytecodeLength);
    if(!reader.OK()) { return; }

    // Create output file.
    std::ofstream out(filePath, std::ios::out);
    if(!out.good())
    {
        std::cout << "Can't write to file " << filePath << "!" << std::endl;
        return;
    }

    // Convert stored (functionName => offset) map to a (offset => functionName) map.
    // Allow us to determine when a new function has started while reading the bytes.
    std::unordered_map<int, std::string> offsetsToFunctionNames;
    for(auto& pair : mFunctions)
    {
        offsetsToFunctionNames[pair.second] = pair.first;
    }

    // Write heading.
    int indentLevel = 0;
    WriteOut(out, "// Decompiled Sheepscript " + GetName(), indentLevel);

    // Write symbols section.
    WriteOut(out, "symbols", indentLevel);
    WriteOut(out, "{", indentLevel);
    ++indentLevel;

    // Write out all variable types, generated names, and default values.
    std::vector<std::string> variableNames; 
    for(int i = 0; i < mVariables.size(); ++i)
    {
        std::string varName = mVariables[i].GetTypeString() + "Var" + std::to_string(i);
        variableNames.push_back(varName);

        std::string varDecl = mVariables[i].GetTypeString() + " " + varName + " = ";
        if(mVariables[i].type == SheepValueType::Int)
        {
            varDecl += std::to_string(mVariables[i].GetInt());
        }
        else if(mVariables[i].type == SheepValueType::Float)
        {
            varDecl += std::to_string(mVariables[i].GetFloat());
        }
        else
        {
            varDecl += "\"" + mVariables[i].GetString() + "\"";
        }
        varDecl += ";";

        WriteOut(out, varDecl, indentLevel);
    }

    // Write an empty space if no variables.
    if(mVariables.empty())
    {
        WriteOut(out, "\n", indentLevel);
    }

    --indentLevel;
    WriteOut(out, "}\n", indentLevel);

    // Write code section.
    WriteOut(out, "code", indentLevel);
    WriteOut(out, "{", indentLevel);
    ++indentLevel;

    // While fake-executing bytecode, we'll misuse the stack to hold combined tokens to regenerate the code text.
    // This works out well with SheepValues, since they can just hold or convert anything to strings.
    // But they only hold char*, so we need someplace concrete to store strings as we use them.
    std::vector<std::string> savedStrings;
    savedStrings.reserve(1000);

    // The logic for detecting and formatting if/elseif/else blocks is a bit shakey and bespoke and heuristic.
    // These variables are used to remember where if blocks should end, whether we're in a block, etc.
    // Could probably be improved.
    std::vector<int> endBlockAddresses;
    int lastBranchAddress = 0;
    bool inIfElseBlock = false;

    // Run through the bytecode, reading data as needed, but not actually executing various functions.
    // We do maintain a stack to help simulate expected values, but no actually system calls occur.
    // The idea is to try to write out, in human readable form, the logic as much as possible. 
    SheepStack stack;
    while(true)
    {
        // Write out function ends and starts.
        auto offsetsIt = offsetsToFunctionNames.find(reader.GetPosition());
        if(offsetsIt != offsetsToFunctionNames.end())
        {
            if(indentLevel > 1)
            {
                --indentLevel;
                WriteOut(out, "}\n", indentLevel);
            }

            WriteOut(out, offsetsIt->second, indentLevel);
            WriteOut(out, "{", indentLevel);
            ++indentLevel;

            inIfElseBlock = false;
        }
        
        // If we hit the address at the back of the "end block addresses" stack, it indicates we've hit the end of an if block.
        // So, we need to close the current block!
        while(!endBlockAddresses.empty() && endBlockAddresses.back() == reader.GetPosition())
        {
            endBlockAddresses.pop_back();

            --indentLevel;
            WriteOut(out, "}", indentLevel);
        }

        // Read instruction.
        char byte = reader.ReadUByte();
        SheepInstruction instruction = (SheepInstruction)byte;
        
        // Break when read instruction fails (perhaps due to reading past end of file/mem stream).
        if(!reader.OK()) { break; }

        // Interpret instruction.
        switch(instruction)
        {
        case SheepInstruction::ReturnV:
        {
            break;
        }
        case SheepInstruction::CallSysFunctionV:
        case SheepInstruction::CallSysFunctionI:
        case SheepInstruction::CallSysFunctionS:
        case SheepInstruction::CallSysFunctionF:
        {
            int functionIndex = reader.ReadInt();
            SysImport* sysFunc = GetSysImport(functionIndex);

            // Build function call command from stack.
            std::string funcCall = sysFunc->name + "(";
            int argCount = stack.Pop().intValue;
            for(int i = 0; i < argCount; ++i)
            {
                SheepValue& sheepValue = stack.Peek(argCount - 1 - i);
                funcCall += sheepValue.GetString();
                if(i < argCount - 1)
                {
                    funcCall += ", ";
                }
            }
            stack.Pop(argCount);
            funcCall += ")";

            // If this is a void func, then it must be single line. Can't be part of an if statement or variable equality.
            if(instruction == SheepInstruction::CallSysFunctionV)
            {
                funcCall += ";";
                WriteOut(out, funcCall, indentLevel);
                stack.PushInt(0);
            }
            else
            {
                // This function _might be_ (probably should be) part of a bigger statement, so let's keep it in our back pocket.
                savedStrings.push_back(funcCall);
                stack.PushString(savedStrings.back().c_str());
            }
            break;
        }
        case SheepInstruction::Branch:
        {
            int branchAddress = reader.ReadInt();
            lastBranchAddress = branchAddress;

            // See if there are any more "Branch" instructions with this particular branch address from here to the address.
            // If not, this is the start of an "else" block.
            bool isElse = true;
            BinaryReader tempReader(mBytecode, mBytecodeLength);
            tempReader.Seek(reader.GetPosition());
            while(tempReader.GetPosition() < branchAddress)
            {
                SheepInstruction instruction = (SheepInstruction)tempReader.ReadByte();
                if(instruction == SheepInstruction::Branch)
                {
                    int nextBranchAddress = tempReader.ReadInt();
                    if(branchAddress == nextBranchAddress)
                    {
                        isElse = false;
                    }
                }
            }

            // Generate "else" statement if this is an else beginning.
            if(isElse)
            {
                --indentLevel;
                WriteOut(out, "}", indentLevel);
                endBlockAddresses.pop_back();

                WriteOut(out, "else", indentLevel);
                WriteOut(out, "{", indentLevel);
                indentLevel++;

                // To properly close an else statement, we need to push the branch address here.
                endBlockAddresses.push_back(branchAddress);

                // There's a problem/bug where nested if blocks will appears as "else ifs" with current logic.
                // This is kind of a HACK, but just reset flag indicating that we're in an if block so the next if will be an "if" rather than "else if".
                inIfElseBlock = false;
            }
            break;
        }
        case SheepInstruction::BranchGoto:
        {
            int branchAddress = reader.ReadInt();
            WriteOut(out, "goto <unknown>;", indentLevel); //TODO: Would be cool to generate labels, but tricky if goto is after label. Need to do two passes.
            break;
        }
        case SheepInstruction::BranchIfZero:
        {
            // The branch address indicates where this if block ends, so save that.
            int branchAddress = reader.ReadInt();
            endBlockAddresses.push_back(branchAddress);

            // A bit of (somewhat brittle) logic to determine if this is an "if" or "else if".
            std::string statement = stack.Pop().GetString();
            if(reader.GetPosition() >= lastBranchAddress || !inIfElseBlock)
            {
                WriteOut(out, "if(" + statement + ")", indentLevel);
                inIfElseBlock = true;
            }
            else
            {
                WriteOut(out, "else if(" + statement + ")", indentLevel);
            }
            WriteOut(out, "{", indentLevel);
            ++indentLevel;
            break;
        }
        case SheepInstruction::BeginWait:
        {
            WriteOut(out, "wait", indentLevel);
            WriteOut(out, "{", indentLevel);
            ++indentLevel;
            break;
        }
        case SheepInstruction::EndWait:
        {
            if(indentLevel > 1)
            {
                --indentLevel;
                WriteOut(out, "}", indentLevel);
            }
            break;
        }
        case SheepInstruction::StoreI:
        case SheepInstruction::StoreF:
        case SheepInstruction::StoreS:
        {
            int varIndex = reader.ReadInt();
            std::string statement = variableNames[varIndex] + " = " + stack.Pop().GetString() + ";";
            WriteOut(out, statement, indentLevel);
            break;
        }
        case SheepInstruction::LoadI:
        case SheepInstruction::LoadF:
        case SheepInstruction::LoadS:
        {
            int varIndex = reader.ReadInt();
            stack.PushString(variableNames[varIndex].c_str());
            break;
        }
        case SheepInstruction::PushI:
        {
            stack.PushInt(reader.ReadInt());
            break;
        }
        case SheepInstruction::PushF:
        {
            stack.PushFloat(reader.ReadFloat());
            break;
        }
        case SheepInstruction::PushS:
        {
            stack.PushStringOffset(reader.ReadInt());
            break;
        }
        case SheepInstruction::GetString:
        {
            SheepValue& offsetValue = stack.Pop();
            std::string* stringPtr = GetStringConst(offsetValue.intValue);
            if(stringPtr != nullptr)
            {
                std::string fullString = "\"" + *stringPtr + "\"";
                savedStrings.push_back(fullString);
                stack.PushString(savedStrings.back().c_str());
            }
            break;
        }
        case SheepInstruction::Pop:
        {
            stack.Pop(1);
            break;
        }
        case SheepInstruction::AddI:
        case SheepInstruction::AddF:
        {
            std::string statement = stack.Peek(1).GetString() + " + " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::SubtractI:
        case SheepInstruction::SubtractF:
        {
            std::string statement = stack.Peek(1).GetString() + " - " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::MultiplyI:
        case SheepInstruction::MultiplyF:
        {
            std::string statement = stack.Peek(1).GetString() + " * " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::DivideI:
        case SheepInstruction::DivideF:
        {
            std::string statement = stack.Peek(1).GetString() + " / " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::NegateI:
        {
            stack.Peek(0).intValue *= -1;
            break;
        }
        case SheepInstruction::NegateF:
        {
            stack.Peek(0).floatValue *= -1.0f;
            break;
        }
        case SheepInstruction::IsEqualI:
        case SheepInstruction::IsEqualF:
        {
            std::string statement = stack.Peek(1).GetString() + " == " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::IsNotEqualI:
        case SheepInstruction::IsNotEqualF:
        {
            std::string statement = stack.Peek(1).GetString() + " != " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::IsGreaterI:
        case SheepInstruction::IsGreaterF:
        {
            std::string statement = stack.Peek(1).GetString() + " > " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::IsLessI:
        case SheepInstruction::IsLessF:
        {
            std::string statement = stack.Peek(1).GetString() + " < " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::IsGreaterEqualI:
        case SheepInstruction::IsGreaterEqualF:
        {
            std::string statement = stack.Peek(1).GetString() + " >= " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::IsLessEqualI:
        case SheepInstruction::IsLessEqualF:
        {
            std::string statement = stack.Peek(1).GetString() + " <= " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::IToF:
        case SheepInstruction::FToI:
        {
            // Need to read the int here, but don't actually have to do anything.
            reader.ReadInt();
            break;
        }
        case SheepInstruction::Modulo:
        {
            std::string statement = stack.Peek(1).GetString() + " % " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::And:
        {
            std::string statement = stack.Peek(1).GetString() + " && " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::Or:
        {
            std::string statement = stack.Peek(1).GetString() + " || " + stack.Peek(0).GetString();
            stack.Pop(2);
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::Not:
        {
            std::string statement = "!(" + stack.Peek(0).GetString() + ")";
            stack.Pop();
            savedStrings.push_back(statement);
            stack.PushString(savedStrings.back().c_str());
            break;
        }
        case SheepInstruction::DebugBreakpoint:
        {
            WriteOut(out, "DebugBreakpoint;", indentLevel);
        }
        default:
            break;
        }
    }

    // Close any open braces.
    while(indentLevel > 0)
    {
        --indentLevel;
        WriteOut(out, "}", indentLevel);
    }
}
