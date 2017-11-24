
#include "vtString.h"
#include "Array.h"
#include "Building.h"

enum TokenType
{
	LITERAL,
	COUNTER,
	NUMBER,
	COLOR,
	MATERIAL,
	ROOFTYPE
};

class InputToken
{
public:
	enum TokenType type;
	vtString str;

	// Don't bother with a union, no need to be efficient here
	int counter;
	float number;
	RGBi color;
	vtString material;
	RoofType rooftype;
};

class MatchToken : public InputToken
{
public:
	bool m_bRequired;
	vtStringArray alternates;
};

typedef vtArray<InputToken *> SentenceIn;
class SentenceMatch : public vtArray<MatchToken *>
{
public:
	virtual ~SentenceMatch() { Clear(); free(m_Data); m_Data = NULL; m_MaxSize = 0; }
	void DestructItems(uint first, uint last) {
		for (uint i = first; i <= last; i++)
			delete GetAt(i);
	}
	MatchToken *AddLiteral(bool required, const char *str1,
		const char *str2 = NULL, const char *str3 = NULL, const char *str4 = NULL);
	MatchToken *AddToken(bool required, enum TokenType type);
};

class TestParser
{
public:
	SentenceIn sen;
	int iWords;

	int IsCounter(InputToken &t);
	int IsNumber(InputToken &t);
	int IsColor(int i);
	int IsMaterial(int i);

	void ParseInput(const char *string);
	void Apply();
	bool Matches(SentenceMatch &pattern);
};

