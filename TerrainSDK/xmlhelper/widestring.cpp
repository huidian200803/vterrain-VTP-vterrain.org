//
// widestring.cpp
//

#include "widestring.h"
#include <cstdlib>

#if __GNUC__ == 4 && __GNUC_MINOR__ >= 3
  #include <cstring>
  #include <cstdlib>
#endif

#if !__DARWIN_OSX__		// Mac doesn't need this class.

char widestring::s_buffer[MAX_WSTRING2_SIZE];

widestring::widestring() : std::wstring()
{
}

widestring::widestring(const wchar_t *__s) : std::wstring(__s)
{
}

widestring::widestring(const char *__s)
{
	int len = strlen(__s);
	wchar_t *tmp = new wchar_t[len*2+1];
	mbstowcs(tmp, __s, len+1);
	// now copy the result into our own storage
	*((std::wstring*)this) = tmp;
	// and get rid of the temporary buffer
	delete [] tmp;
}

const char *widestring::mb_str() const
{
	const wchar_t *guts = c_str();
	wcstombs(s_buffer, guts, MAX_WSTRING2_SIZE);
	return s_buffer;
}

#define WC_UTF16

static size_t encode_utf16(unsigned int input, wchar_t *output)
{
	if (input<=0xffff)
	{
		if (output) *output++ = (wchar_t) input;
		return 1;
	}
	else if (input>=0x110000)
	{
		return (size_t)-1;
	}
	else
	{
		if (output)
		{
			*output++ = (wchar_t) ((input >> 10)+0xd7c0);
			*output++ = (wchar_t) ((input&0x3ff)+0xdc00);
		}
		return 2;
	}
}

static size_t decode_utf16(const wchar_t *input, unsigned int &output)
{
	if ((*input<0xd800) || (*input>0xdfff))
	{
		output = *input;
		return 1;
	}
	else if ((input[1]<0xdc00) || (input[1]>=0xdfff))
	{
		output = *input;
		return (size_t)-1;
	}
	else
	{
		output = ((input[0] - 0xd7c0) << 10) + (input[1] - 0xdc00);
		return 2;
	}
}

static unsigned int utf8_max[]=
	{ 0x7f, 0x7ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff, 0xffffffff };

size_t widestring::from_utf8(const char *psz)
{
	size_t safe_length = strlen(psz)+1;
	wchar_t *tmp = new wchar_t[safe_length*2];
	wchar_t *buf = tmp;
	size_t n = safe_length;

	size_t len = 0;

	while (*psz && ((!buf) || (len < n)))
	{
		unsigned char cc = *psz++, fc = cc;
		unsigned cnt;
		for (cnt = 0; fc & 0x80; cnt++)
			fc <<= 1;
		if (!cnt)
		{
			// plain ASCII char
			if (buf)
				*buf++ = cc;
			len++;
		}
		else
		{
			cnt--;
			if (!cnt)
			{
				// invalid UTF-8 sequence
				return (size_t)-1;
			}
			else
			{
				unsigned ocnt = cnt - 1;
				unsigned int res = cc & (0x3f >> cnt);
				while (cnt--)
				{
					cc = *psz++;
					if ((cc & 0xC0) != 0x80)
					{
						// invalid UTF-8 sequence
						return (size_t)-1;
					}
					res = (res << 6) | (cc & 0x3f);
				}
				if (res <= utf8_max[ocnt])
				{
					// illegal UTF-8 encoding
					return (size_t)-1;
				}
				size_t pa = encode_utf16(res, buf);
				if (pa == (size_t)-1)
				  return (size_t)-1;
				if (buf)
					buf += pa;
				len += pa;
			}
		}
	}
	if (buf && (len < n))
		*buf = 0;

	// now copy the result into our own storage
	*((std::wstring*)this) = tmp;
	// and get rid of the temporary buffer
	delete [] tmp;

	return len;
}

const char *widestring::to_utf8() const
{
	char *buf = s_buffer;
	size_t len = 0;
	size_t n = MAX_WSTRING2_SIZE;
	const wchar_t *psz = c_str();

	while (*psz && ((!buf) || (len < n)))
	{
		unsigned int cc;
#ifdef WC_UTF16
		size_t pa = decode_utf16(psz, cc);
		psz += (pa == (size_t)-1) ? 1 : pa;
#else
		cc=(*psz++) & 0x7fffffff;
#endif
		unsigned cnt;
		for (cnt = 0; cc > utf8_max[cnt]; cnt++) {}
		if (!cnt)
		{
			// plain ASCII char
			if (buf)
				*buf++ = (char) cc;
			len++;
		}
		else
		{
			len += cnt + 1;
			if (buf)
			{
				*buf++ = (char) ((-128 >> cnt) | ((cc >> (cnt * 6)) & (0x3f >> cnt)));
				while (cnt--)
					*buf++ = (char) (0x80 | ((cc >> (cnt * 6)) & 0x3f));
			}
		}
	}

	if (buf && (len<n)) *buf = 0;
	return s_buffer;
}

#endif // __DARWIN_OSX__
