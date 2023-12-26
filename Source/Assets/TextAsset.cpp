#include "TextAsset.h"

TextAsset::~TextAsset()
{
    delete[] mText;
}

void TextAsset::Load(uint8_t* data, uint32_t dataLength)
{
    mText = data;
    mTextLength = dataLength;
}
