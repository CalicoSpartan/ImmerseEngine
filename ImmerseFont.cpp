#include "ImmerseFont.h"

ImmerseFont::ImmerseFont(std::string fontName, int fontWidth, int fontHeight)
{
	this->fontName = fontName;
	this->fontWidth = fontWidth;
	this->fontHeight = fontHeight;
	glyphWidth = fontWidth / numColumns;
	glyphHeight = fontHeight / numRows;
	assert(glyphWidth * numColumns == fontWidth);
	assert(glyphHeight * numRows == fontHeight);
}

XMFLOAT4 ImmerseFont::MapGlyphQuad(char c)
{
	assert(c >= firstChar && c <= lastChar);
	int glyphIndex = c - firstChar;
	int yGlyph = glyphIndex / numColumns;
	int xGlyph = glyphIndex %  numColumns;
	return XMFLOAT4(xGlyph * glyphWidth,yGlyph * glyphHeight,glyphWidth,glyphHeight);
}
