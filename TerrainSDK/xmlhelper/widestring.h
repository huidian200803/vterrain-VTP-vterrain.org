//
// widestring.h
//

#include <string>
#include <wchar.h>

#if !__DARWIN_OSX__		// Mac doesn't need this class.

#define MAX_WSTRING2_SIZE 4096
/**
 * Another string class.  This one always stores string internally as 16-bit
 * wide characters, and can convert to and from other representations as
 * desired.
 *
 * Unlike the C++ standard "wstring" class, on which it is based, this class
 * can convert to and from UTF8 and default encoded 8-bit character strings.
 */
class widestring : public std::wstring
{
public:
	widestring();
	widestring(const wchar_t *__s);
	widestring(const char *__s);
	const char *mb_str() const;		// multibyte string in local encoding

	size_t from_utf8(const char *in);
	const char *to_utf8() const;

private:
	static char s_buffer[MAX_WSTRING2_SIZE];
};

#endif // __DARWIN_OSX__
