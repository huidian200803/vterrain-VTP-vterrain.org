/*
 * Test code.  Very rough.  Just throwing around an idea here.
 */

/*
  Some examples of the kind of word input we might expect to handle, using
  Editing of Buildings is the example subject.

  Selection of whole objects will be with mouse, not words, but sub-parts of
  buildings have their own selection.

  "Select floor/level <number>"
  "Select [the] <counter> floor/level"
  "Delete/remove [it]"
  "Copy/duplicate [it]"
  "Move [it] up"
  "Move [it] down"
  "[it has]/[set [it] to] <number> floors/stories/floor/story"
  "[make it]/[set it to] <color>"
  "[make it]/[set it to] <material>"
  "[height]/[it] [is] <number> meters"
  "<rooftype> [roof]"

  <counter> = "first, second, third ..." or "1st, 2nd, 3rd ..."
  <number> = any string containing only digits/./, + spellings of numbers
  <color> = "red, blue ..."
  <material> = "plain, wood, siding, glass, brick, painted brick, rolled roofing, cement"
  <rooftype> = "flat, shed, hip, gable, vertical"
*/

#include "Vocab.h"
#include "MaterialDescriptor.h"

struct ColorEntry
{
	short r, g, b, words;
	const char *name;
};

ColorEntry ColorEntries[] =
{
	{ 240, 248, 255,	2, "alice blue" },
	{ 250, 235, 215,	2, "antique white" },
	{ 50, 191, 193,		1, "aquamarine" },
	{ 240, 255, 255,	1, "azure" },
	{ 245, 245, 220,	1, "beige" },
	{ 255, 228, 196,	1, "bisque" },
	{ 0, 0, 0,			1, "black" },
	{ 255, 235, 205,	2, "blanched almond" },
	{ 0, 0, 255,		1, "blue" },
	{ 138, 43, 226,		2, "blue violet" },
	{ 165, 42, 42,		1, "brown" },
	{ 222, 184, 135,	1, "burlywood" },
	{ 95, 146, 158,		2, "cadet blue" },
	{ 127, 255, 0,		1, "chartreuse" },
	{ 210, 105, 30,		1, "chocolate" },
	{ 255, 114, 86,		1, "coral" },
	{ 34, 34, 152,		2, "cornflower blue" },
	{ 255, 248, 220,	1, "cornsilk" },
	{ 0, 255, 255,		1, "cyan" },
	{ 184, 134, 11,		2, "dark goldenrod" },
	{ 84, 84, 84,		2, "dark gray" },
	{ 84, 84, 84,		2, "dark grey" },
	{ 0, 86, 45,		2, "dark green" },
	{ 189, 183, 107,	2, "dark khaki" },
	{ 222, 0, 165,		2, "dark nessy" },
	{ 85, 86, 47,		3, "dark olive green" },
	{ 255, 140, 0,		2, "dark orange" },
	{ 139, 32, 139,		2, "dark orchid" },
	{ 233, 150, 122,	2, "dark salmon" },
	{ 143, 188, 143,	3, "dark sea green" },
	{ 56, 75, 102,		3, "dark slate blue" },
	{ 47, 79, 79,		3, "dark slate gray" },
	{ 47, 79, 79,		4, "dark slate grey" },
	{ 0, 166, 166,		2, "dark turquoise" },
	{ 148, 0, 211,		2, "dark violet" },
	{ 255, 20, 147,		2, "deep pink" },
	{ 0, 191, 255,		3, "deep sky blue" },
	{ 30, 144, 255,		2, "dodger blue" },
	{ 142, 35, 35,		2, "fire brick" },
	{ 255, 250, 240,	2, "floral white" },
	{ 80, 159, 105,		2, "forest green" },
	{ 220, 220, 220,	1, "gainsboro" },
	{ 248, 248, 255,	2, "ghost white" },
	{ 218, 170, 0,		1, "gold" },
	{ 239, 223, 132,	1, "goldenrod" },
	{ 126, 126, 126,	1, "gray" },
	{ 126, 126, 126,	1, "grey" },
	{ 240, 255, 240,	1, "honeydew" },
	{ 255, 105, 180,	2, "hot pink" },
	{ 107, 57, 57,		2, "indian red" },
	{ 255, 255, 240,	1, "ivory" },
	{ 179, 179, 126,	1, "khaki" },
	{ 230, 230, 250,	1, "lavender" },
	{ 255, 240, 245,	2, "lavender blush" },
	{ 124, 252, 0,		2, "lawn green" },
	{ 255, 250, 205,	1, "lemon" },
	{ 255, 250, 205,	2, "lemon chiffon" },
	{ 176, 226, 255,	2, "light blue" },
	{ 240, 128, 128,	2, "light coral" },
	{ 224, 255, 255,	2, "light cyan" },
	{ 238, 221, 130,	2, "light goldenrod" },
	{ 250, 250, 210,	3, "light goldenrod yellow" },
	{ 168, 168, 168,	2, "light gray" },
	{ 168, 168, 168,	2, "light grey" },
	{ 255, 128, 210,	2, "light nessy" },
	{ 255, 182, 193,	2, "light pink" },
	{ 255, 160, 122,	2, "light salmon" },
	{ 32, 178, 170,		3, "light sea green" },
	{ 135, 206, 250,	3, "light sky blue" },
	{ 132, 112, 255,	3, "light slate blue" },
	{ 119, 136, 153,	3, "light slate gray" },
	{ 119, 136, 153,	3, "light slate grey" },
	{ 124, 152, 211,	3, "light steel blue" },
	{ 255, 255, 224,	2, "light yellow" },
	{ 0, 175, 20,		1, "lime" },
	{ 0, 175, 20,		2, "lime green" },
	{ 250, 240, 230,	1, "linen" },
	{ 255, 0, 255,		1, "magenta" },
	{ 143, 0, 82,		1, "maroon" },
	{ 0, 147, 143,		2, "medium aquamarine" },
	{ 50, 50, 204,		2, "medium blue" },
	{ 50, 129, 75,		3, "medium forest green" },
	{ 209, 193, 102,	2, "medium goldenrod" },
	{ 189, 82, 189,		2, "medium orchid" },
	{ 147, 112, 219,	2, "medium purple" },
	{ 52, 119, 102,		3, "medium sea green" },
	{ 106, 106, 141,	3, "medium slate blue" },
	{ 35, 142, 35,		3, "medium spring green" },
	{ 0, 210, 210,		2, "medium turquoise" },
	{ 213, 32, 121,		3, "medium violet red" },
	{ 47, 47, 100,		2, "midnight blue" },
	{ 245, 255, 250,	1, "mint" },
	{ 255, 228, 225,	2, "misty rose" },
	{ 255, 228, 181,	1, "moccasin" },
	{ 255, 222, 173,	2, "navajo white" },
	{ 35, 35, 117,		1, "navy" },
	{ 35, 35, 117,		2, "navy blue" },
	{ 255, 66, 210,		1, "nessy" },
	{ 253, 245, 230,	2, "old lace" },
	{ 107, 142, 35,		1, "olive" },
	{ 107, 142, 35,		2, "olive drab" },
	{ 255, 135, 0,		1, "orange" },
	{ 255, 69, 0,		2, "orange red" },
	{ 239, 132, 239,	1, "orchid" },
	{ 238, 232, 170,	2, "pale goldenrod" },
	{ 115, 222, 120,	2, "pale green" },
	{ 175, 238, 238,	2, "pale turquoise" },
	{ 219, 112, 147,	2, "pale violetred" },
	{ 255, 239, 213,	1, "papaya" },
	{ 255, 239, 213,	2, "papaya whip" },
	{ 255, 218, 185,	1, "peach" },
	{ 255, 218, 185,	2, "peach puff" },
	{ 205, 133, 63,		1, "peru" },
	{ 255, 181, 197,	1, "pink" },
	{ 197, 72, 155,		1, "plum" },
	{ 176, 224, 230,	2, "powder blue" },
	{ 160, 32, 240,		1, "purple" },
	{ 255, 0, 0,		1, "red" },
	{ 188, 143, 143,	2, "rosy brown" },
	{ 65, 105, 225,		2, "royal blue" },
	{ 139, 69, 19,		2, "saddle brown" },
	{ 233, 150, 122,	1, "salmon" },
	{ 244, 164, 96,		1, "sandy" },
	{ 244, 164, 96,		1, "sandy brown" },
	{ 82, 149, 132,		2, "sea green" },
	{ 255, 245, 238,	1, "seashell" },
	{ 150, 82, 45,		1, "sienna" },
	{ 114, 159, 255,	2, "sky blue" },
	{ 126, 136, 171,	2, "slate blue" },
	{ 112, 128, 144,	2, "slate gray" },
	{ 112, 128, 144,	2, "slate grey" },
	{ 255, 250, 250,	1, "snow" },
	{ 65, 172, 65,		2, "spring green" },
	{ 84, 112, 170,		2, "steel blue" },
	{ 222, 184, 135,	1, "tan" },
	{ 216, 191, 216,	1, "thistle" },
	{ 255, 99, 71,		1, "tomato" },
	{ 25, 204, 223,		1, "turquoise" },
	{ 156, 62, 206,		1, "violet" },
	{ 243, 62, 150,		2, "violet red" },
	{ 245, 222, 179,	1, "wheat" },
	{ 255, 255, 255,	1, "white" },
	{ 245, 245, 245,	2, "white smoke" },
	{ 255, 255, 0,		1, "yellow" },
	{ 50, 216, 56,		2, "yellow green" }
};

struct MatEntry
{
	const char *bmat;
	short words;
	const char *name;
};

MatEntry MatEntries[] = {
	{ BMAT_NAME_PLAIN, 1, "plain" },
	{ BMAT_NAME_WOOD, 1, "wood" },
	{ BMAT_NAME_SIDING, 1, "siding" },
	{ BMAT_NAME_BRICK, 1, "brick" },
	{ BMAT_NAME_PAINTED_BRICK, 2, "painted brick" },
	{ BMAT_NAME_ROLLED_ROOFING, 2, "rolled roofing" },
	{ BMAT_NAME_ROLLED_ROOFING, 1, "roofing" },
	{ BMAT_NAME_CEMENT, 1, "cement" },
	{ BMAT_NAME_CORRUGATED, 1, "corrugated" },
};

const char *counters[] = { "zeroeth", "first", "second", "third",
	"fourth", "fifth", "sixth", "seventh", "eighth", "ninth", "tenth",
	"eleventh", "twelveth", "thirteenth", "fourteenth", "fifteenth",
	"sixteenth", "seventeenth", "eighteenth", "nineteenth", "twentieth" };

int TestParser::IsCounter(InputToken &t)
{
	int i, j;
	for (i = 0; i < (sizeof(counters) / sizeof(char *)); i++)
	{
		if (t.str == counters[i])
		{
			t.type = COUNTER;
			t.counter = i;
			return 1;
		}
	}

	vtString end = t.str.Right(2);
	if (end == "st" || end == "nd" || end == "rd" || end == "th")
	{
		vtString begin = t.str.Left(t.str.GetLength() - 2);
		bool isnum = true;
		for (j = 0; j < begin.GetLength(); j++)
		{
			if (!isdigit(begin[j]))
				isnum = false;
		}
		if (isnum)
		{
			t.type = COUNTER;
			t.counter = atoi((const char *) begin);
			return 1;
		}
	}

	return 0;
}

int TestParser::IsNumber(InputToken &t)
{
	int j;
	for (j = 0; j < t.str.GetLength(); j++)
	{
		char c = t.str[j];
		if (!isdigit(c) && c != '.' && c != ',' )
			return 0;
	}
	t.type = NUMBER;
	t.number = (float) atof((const char *) t.str);
	return 1;
}

int TestParser::IsColor(int index)
{
	char color_string[64];

	int i, j, k;
	for (i = 0; i < (sizeof(ColorEntries) / sizeof(ColorEntry)); i++)
	{
		ColorEntry &col = ColorEntries[i];
		strcpy(color_string, col.name);
		char *word = strtok(color_string, " ");
		for (j = 0; j < col.words && index+j < iWords; j++)
		{
			if (sen[index+j]->str != word)
				break;
			word = strtok(NULL, " ");
		}
		if (j == col.words)
		{
			// Found a match
			InputToken &tok = *sen[index];
			tok.type = COLOR;
			tok.color.Set(col.r, col.g, col.b);

			// glom the other tokens into this one
			for (k = 1; k < col.words; k++)
			{
				tok.str += " ";
				tok.str += sen[index+k]->str;
			}
			for (k = 1; k < col.words; k++)
			{
				sen.RemoveAt(index+1);
			}
			iWords = sen.GetSize();
			return col.words;
		}
	}
	return 0;
}

int TestParser::IsMaterial(int index)
{
	char mat_string[64];

	int i, j, k;
	for (i = 0; i < (sizeof(MatEntries) / sizeof(MatEntry)); i++)
	{
		MatEntry &mat = MatEntries[i];
		strcpy(mat_string, mat.name);
		char *word = strtok(mat_string, " ");
		for (j = 0; j < mat.words && index+j < iWords; j++)
		{
			if (sen[index+j]->str != word)
				break;
			word = strtok(NULL, " ");
		}
		if (j == mat.words)
		{
			// Found a match
			InputToken &tok = *sen[index];
			tok.type = COLOR;
			tok.material = mat.bmat;

			// glom the other tokens into this one
			for (k = 1; k < mat.words; k++)
			{
				tok.str += " ";
				tok.str += sen[index+k]->str;
			}
			for (k = 1; k < mat.words; k++)
			{
				sen.RemoveAt(index+1);
			}
			iWords = sen.GetSize();
			return mat.words;
		}
	}
	return 0;
}

void TestParser::ParseInput(const char *string)
{
	sen.Clear();

	InputToken *t;
	char *word = strtok((char *)string, " ");
	while (word)
	{
		t = new InputToken;
		t->type = LITERAL;
		t->str = word;
		t->str.MakeLower();
		word = strtok(NULL, " ");
		sen.Append(t);
	}
	iWords = sen.GetSize();

	int i, words;
	for (i = 0; i < iWords; i++)
	{
		// compare words from word n onwards
		t = sen[i];

		words = IsCounter(*t);
		if (words != 0)
			continue;

		words = IsNumber(*t);
		if (words != 0)
			continue;

		words = IsColor(i);
		if (words != 0)
			continue;

		words = IsMaterial(i);
		if (words != 0)
			continue;
	}
}

void TestParser::Apply()
{
}

/**
 * Test whether the parsed sentence matches a given pattern.
 * If so, return true and copy the non-literal values to the pattern tokens.
 */
bool TestParser::Matches(SentenceMatch &pattern)
{
	uint i, j;
	int pos = 0;	// position in the array of input tokens
	uint pat_toks = pattern.GetSize();

	// rather than iterate through the words of the input, interate
	// through the words of the pattern

	for (i = 0; i < pat_toks; i++)
	{
		MatchToken *mtok = pattern[i];

		if (pos >= iWords)
		{
			// we've run out of input.  if there is anything required at this
			// point, fail.
			if (mtok->m_bRequired)
				return false;
			else
				continue;
		}

		InputToken *itok = sen[pos];

		// Literals must match directly, other tokens must match in type
		bool match;
		if (mtok->type == LITERAL)
		{
			match = (mtok->str == itok->str);

			// try alternates strings as well
			for (j = 0; j < mtok->alternates.size(); j++)
				match = match || (mtok->alternates[j] == itok->str);
		}
		else
		{
			match = (mtok->type == itok->type);
		}
		if (match)
		{
			// match, proceed through input, copy values to pattern
			pos++;
			if (mtok->type != LITERAL)
				*((InputToken *)mtok) = *itok;
		}
		else
		{
			// fails match.  if required, exit.  otherwise, leave pos
			// unchanged, because the next mtok might match the input.
			if (mtok->m_bRequired)
				return false;
		}
	}

	// if we make it all the way through, we succeeded!
	return true;
}


MatchToken *SentenceMatch::AddLiteral(bool required, const char *str1,
	const char *str2, const char *str3, const char *str4)
{
	MatchToken *mtok = new MatchToken;
	mtok->type = LITERAL;
	mtok->str = str1;
	mtok->m_bRequired = required;
	Append(mtok);

	if (str2 != NULL)
		mtok->alternates.push_back(vtString(str2));
	if (str3 != NULL)
		mtok->alternates.push_back(vtString(str3));
	if (str4 != NULL)
		mtok->alternates.push_back(vtString(str4));

	return mtok;
}

MatchToken *SentenceMatch::AddToken(bool required, enum TokenType type)
{
	MatchToken *mtok = new MatchToken;
	mtok->type = type;
	mtok->m_bRequired = required;
	Append(mtok);
	return mtok;
}
