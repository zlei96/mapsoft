/*
    Misc utilities.

    Copyright (C) 2002-2005 Robert Lipe, robertlipe@usa.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

 */

#include "utils.h"
#include "gpsmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>

// First test Apple's clever macro that's really a runtime test so
// that our universal binaries work right.
#if defined __BIG_ENDIAN__
#define i_am_little_endian !__BIG_ENDIAN__
#else
#if defined WORDS_BIGENDIAN
# define i_am_little_endian 0
#else
# define i_am_little_endian 1
#endif
#endif

#ifdef DEBUG_MEM
#define DEBUG_FILENAME "/tmp/gpsbabel.debug"

static FILE *debug_mem_file = NULL;
void 
debug_mem_open() 
{
	debug_mem_file = xfopen( DEBUG_FILENAME, "a", "debug" );
}

void
debug_mem_output(char *format, ...) 
{
	va_list args;
	va_start( args, format );
	if ( debug_mem_file ) {
		vfprintf( debug_mem_file, format, args );
		fflush( debug_mem_file );
	}
	va_end( args );
}

void
debug_mem_close()
{
	if ( debug_mem_file ) {
		fclose(debug_mem_file);
	}
	debug_mem_file = NULL;
}
#endif

void *
#ifdef DEBUG_MEM
XMALLOC(size_t size, DEBUG_PARAMS)
#else
xmalloc(size_t size)
#endif
{
	void *obj = malloc(size);

#ifdef DEBUG_MEM
	debug_mem_output( "malloc, %x, %d, %s, %d\n", 
			obj, size, file, line );
#endif
	if (!obj) {
		fprintf(stderr, "gpsbabel: Unable to allocate %ld bytes of memory.\n", (unsigned long) size);
	}

	return obj;
}

void *
#ifdef DEBUG_MEM
XCALLOC(size_t nmemb, size_t size, DEBUG_PARAMS)
#else
xcalloc(size_t nmemb, size_t size)
#endif
{
	void *obj = calloc(nmemb, size);
#ifdef DEBUG_MEM
	debug_mem_output( "calloc, %x, %d, %d, %s, %d\n", 
			obj, nmemb, size, file, line );
#endif

	if (!obj) {
		fprintf(stderr, "gpsbabel: Unable to allocate %ld units of %ld bytes of memory.\n", (unsigned long) nmemb, (unsigned long) size);
	}

	return obj;
}

void
#ifdef DEBUG_MEM
XFREE( void *mem, DEBUG_PARAMS )
#else
xfree( void *mem )
#endif
{
	free(mem);
#ifdef DEBUG_MEM
	debug_mem_output( "free, %x, %s, %d\n", 
			mem, file, line );
#endif
}

char *
#ifdef DEBUG_MEM
XSTRDUP(const char *s, DEBUG_PARAMS )
#else
xstrdup(const char *s)
#endif
{
	char *o = s ? strdup(s) : strdup("");
#ifdef DEBUG_MEM
	debug_mem_output( "strdup, %x, %x, %s, %d\n", 
			o, s, file, line );
#endif

	if (!o) {
		fprintf(stderr, "gpsbabel: Unable to allocate %ld bytes of memory.\n", (unsigned long) strlen(s));
	}

	return o;
}

/*
 * Duplicate at most sz bytes in str.
 */
char *
#ifdef DEBUG_MEM
XSTRNDUP(const char *str, size_t sz, DEBUG_PARAMS )
#else
xstrndup(const char *str, size_t sz)
#endif
{
	size_t newlen = 0;
	char *cin = (char *)str;
	char *newstr;

	while ((newlen < sz) && (*cin != '\0')) {
		newlen++;
		cin++;
	}
	
	newstr = (char *) xmalloc(newlen + 1);
	memcpy(newstr, str, newlen);    
	newstr[newlen] = 0;

	return newstr;
}

/*
 * Lazily trim whitespace (though not from allocated version) 
 * while copying.
 */
char *
#ifdef DEBUG_MEM
XSTRNDUPT(const char *str, size_t sz, DEBUG_PARAMS )
#else
xstrndupt(const char *str, size_t sz)
#endif
{
	size_t newlen = 0;
	char *cin = (char *)str;
	char *newstr;

	while ((newlen < sz) && (*cin != '\0')) {
		newlen++;
		cin++;
	}
	
	newstr = (char *) xmalloc(newlen + 1);
	memcpy(newstr, str, newlen);    
	newstr[newlen] = 0;
	rtrim(newstr);

	return newstr;
}

void *
#ifdef DEBUG_MEM
XREALLOC(void *p, size_t s, DEBUG_PARAMS )
#else
xrealloc(void *p, size_t s)
#endif
{
	char *o = (char *) realloc(p,s);
#ifdef DEBUG_MEM
	if (p != NULL)
		debug_mem_output( "realloc, %x, %x, %x, %s, %d\n", o, p, s, file, line );
	else
		debug_mem_output( "malloc, %x, %d, %s, %d\n", o, s, file, line );
#endif

	if (!o) {
		fprintf(stderr, "gpsbabel: Unable to realloc %ld bytes of memory.\n", (unsigned long) s);
	}

	return o;
}

/*
* For an allocated string, realloc it and append 's'
*/
char *
#ifdef DEBUG_MEM
XSTRAPPEND(char *src, const char *newd, DEBUG_PARAMS)
#else
xstrappend(char *src, const char *newd)
#endif
{
	size_t newsz;

	if (!src) {
		return xxstrdup(newd, file, line);
	}
	if (!newd) {
		return xxstrdup(src, file, line);
	}

	newsz = strlen(src) + strlen(newd) + 1;
	src = xxrealloc(src, newsz, file, line);
	strcat(src, newd);

	return src;
}

/*
 * Allocate a string using a format list with optional arguments
 * Returns -1 on error.
 * If return value is anything else, *strp will be populated with an
 * allocated string containging the formatted buffer.
 * 
 * Freeing that is the responsbility of the caller.
 */

int
xasprintf(char **strp, const char *fmt, ...)
{
	va_list args;
	int res;
	
	va_start(args, fmt);
	res = xvasprintf(strp, fmt, args);
	va_end(args);
	
	return res;
}

int
xvasprintf(char **strp, const char *fmt, va_list ap)
{
/* From http://perfec.to/vsnprintf/pasprintf.c */
/* size of first buffer malloc; start small to exercise grow routines */
#ifdef DEBUG_MEM
# define	FIRSTSIZE	64
#else
# define	FIRSTSIZE	1
#endif
	char *buf = NULL;
	int bufsize;
	char *newbuf;
	size_t nextsize = 0;
	int outsize;
	va_list args;

	bufsize = 0;
	for (;;) {
		if (bufsize == 0) {
			if ((buf = xmalloc(FIRSTSIZE)) == NULL) {
				*strp = NULL;
				return -1;
			}
			bufsize = FIRSTSIZE;
		} else if ((newbuf = xrealloc(buf, nextsize)) != NULL) {
			buf = newbuf;
			bufsize = nextsize;
		} else {
			xfree(buf);
			*strp = NULL;
			return -1;
		}

		va_copy(args, ap);
		outsize = vsnprintf(buf, bufsize, fmt, args);
		va_end(args);
		
		if (outsize == -1) {
			/* Clear indication that output was truncated, but no
			 * clear indication of how big buffer needs to be, so
			 * simply double existing buffer size for next time.
			 */
			nextsize = bufsize * 2;

		} else if (outsize == bufsize) {
			/* Output was truncated (since at least the \0 could
			 * not fit), but no indication of how big the buffer
			 * needs to be, so just double existing buffer size
			 * for next time.
			 */
			nextsize = bufsize * 2;

		} else if (outsize > bufsize) {
			/* Output was truncated, but we were told exactly how
			 * big the buffer needs to be next time. Add two chars
			 * to the returned size. One for the \0, and one to
			 * prevent ambiguity in the next case below.
			 */
			nextsize = outsize + 2;

		} else if (outsize == bufsize - 1) {
			/* This is ambiguous. May mean that the output string
			 * exactly fits, but on some systems the output string
			 * may have been trucated. We can't tell.
			 * Just double the buffer size for next time.
			 */
			nextsize = bufsize * 2;

		} else {
			/* Output was not truncated */
			break;
		}
	}
	/* Prevent us from allocating millions of unused bytes. */
	/* O.K.: I think this is not the final solution. */
	if (bufsize > outsize + 1) {
		const unsigned ptrsz = sizeof(buf);
		if (((bufsize + ptrsz + 1) / ptrsz) > ((outsize + ptrsz + 1) / ptrsz))
			buf = xrealloc(buf, outsize + 1);	

	}
	*strp = buf;
	return outsize;
}


/* 
 * Duplicate a pascal string into a normal C string.
 */
char *
pstrdup(char *src)
{
	int len = src[0];
	char *obuf = xmalloc(len + 1);

	memcpy(obuf, src + 1, len);
	obuf[len] = 0;

	return obuf;
}

void 
rtrim(char *s)
{
	char *t = s;

	if (!s || !*s) {
		return;
	}

	while (*s) {
		s++;
	}

	s--;
	while ((s >= t) && isspace (*s)) {
		*s = 0;
		s--;
	}
}

/*
 * Like trim, but trims whitespace from both beginning and end.
 */
char *
lrtrim(char *buff)
{
	char *c;

	if (buff[0] == '\0')
		return buff;

	c = buff + strlen(buff);
	while ((c >= buff) && ((unsigned char)*c <= ' ')) *c-- = '\0';

	c = buff;
	while ((*c != '\0') && ((unsigned char)*c <= ' ')) c++;
	
	if (c != buff) {
		char *src = c;
		char *dst = buff;
		
		while (*src) *dst++ = *src++;
		*dst = '\0';
	}

	return buff;
}

/*
 *   Like strcmp, but case insensitive.  Like Berkeley's strcasecmp.
 */

int 
case_ignore_strcmp(const char *s1, const char *s2)
{
	for(;toupper(*s1) == toupper(*s2); ++ s1, ++s2) {
		if (*s1 == 0)
			return 0;
	}
	return (toupper(*s1) < toupper(*s2)) ? -1 : +1;

}

int 
case_ignore_strncmp(const char *s1, const char *s2, int n)
{
	int rv = 0;

	while (n && ((rv = toupper(*s1) - toupper(*s2)) == 0)
		&& *s1) {
		s1++;
		s2++;
		n--;
	}
	return rv;
}

/*
 * compare str with match
 * match may contain wildcards "*" and "?"
 *
 * examples:
 *		str_match("ABCDE", "*BC*") ->	1
 *		str_match("ABCDE", "A*C*E") ->	1
 *		str_match("?ABCDE", "\\?A*") ->	1
 *		str_match("", "*A") -> 		0
 */
 
int
str_match(const char *str, const char *match)
{
	char *m, *s;
	
	s = (char *)str;
	m = (char *)match; 
	
	while (*m || *s)
	{
		switch(*m) 
		{
			
			case '\0':
				/* there is something left in s, FAIL */
				return 0;

			case '*':
				/* skip all wildcards */
				while ((*m == '*') || (*m == '?')) m++;
				if (*m == '\0') return 1;
				
				if (*m == '\\')				/* ? escaped ? */
				{
					m++;
					if (*m == '\0') return 0;
				}

				do
				{
					char *mx, *sx;
					
					while (*s && (*s != *m)) s++;
					if (*s == '\0') return 0;
					
					sx = s + 1;
					mx = m + 1;
					
					while (*sx)
					{
						if (*mx == '\\')	/* ? escaped ? */
						{
							mx++;
							if (*mx == '\0') return 0;
							
						}
						if (*sx == *mx)
						{
							sx++;
							mx++;
						}
						else
							break;
					}
					if (*mx == '\0')		/* end of match */
					{
						if (*sx == '\0') return 1;
						s++;
					}
					else if ((*mx == '?') || (*mx == '*'))
					{
						s = sx;
						m = mx;
						break;
					}
					else
						s++;
				} while (*s);
				break;
				
			case '?':
				if (*s == '\0') return 0;	/* no character left */
				m++;
				s++;
				break;
				
			case '\\':
				m++;
				if (*m == '\0') return 0; /* incomplete escape sequence */
				/* pass-through next character */
				
			default:
				if (*m != *s) return 0;
				m++;
				s++;
		}
	}
	return ((*s == '\0') && (*m == '\0'));
}

/*
 * as str_match, but case insensitive 
 */
 
int
case_ignore_str_match(const char *str, const char *match)
{
	char *s1, *s2;
	int res;
	
	s1 = strupper(xstrdup(str));
	s2 = strupper(xstrdup(match));
	res = str_match(s1, s2);
	xfree(s1);
	xfree(s2);
	
	return res;
}

char *
strenquote(const char *str, const char quot_char)
{
	int len;
	const char *cin;
	char *cout;
	char *tmp;

	if (str == NULL) cin = "";
	else cin = (char *)str;
	
	len = strlen(cin);
	cout = tmp = xmalloc((len * 2) + 3);
	
	*cout++ = quot_char;
	while (*cin) {
		*cout++	= *cin;
		if (*cin++ == quot_char)
			*cout++	= quot_char;
	}
	*cout++ = quot_char;
	*cout = '\0';
	
	cout = xstrdup(tmp);
	xfree(tmp);
	return cout;
}

void
printposn(const double c, int is_lat)
{
	char d;
	if (is_lat) {
		if (c < 0) d = 'S'; else d = 'N';
	} else {
		if (c < 0) d = 'W'; else d = 'E';
	}
	printf("%f%c ", fabs(c), d);
}

/*
 * Read 4 bytes in big-endian.   Return as "int" in native endianness.
 */
signed int
be_read32(const void *p)
{
	unsigned char *i = (unsigned char *) p;
	return i[0] << 24 | i[1] << 16  | i[2] << 8 | i[3];
}

signed int
be_read16(const void *p)
{
	unsigned char *i = (unsigned char *) p;
	return i[0] << 8 | i[1];
}

void
be_write16(void *addr, const unsigned value)
{
	unsigned char *p = addr;
	p[0] = value >> 8;
	p[1] = value;
	
}

void
be_write32(void *pp, const unsigned i)
{
	char *p = (char *)pp;

	p[0] = (i >> 24) & 0xff;
	p[1] = (i >> 16) & 0xff;
	p[2] = (i >> 8) & 0xff;
	p[3] = i & 0xff;
}

signed int
le_read16(const void *addr)
{
	const unsigned char *p = addr;
	return p[0] | (p[1] << 8);
}

unsigned int
le_readu16(const void *addr)
{
	const unsigned char *p = addr;
	return p[0] | (p[1] << 8);
}

signed int
le_read32(const void *addr)
{
	const unsigned char *p = addr;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

unsigned int
le_readu32(const void *addr)
{
	const unsigned char *p = addr;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

/*
 *  Read a little-endian 64-bit value from 'src' and return it in 'dest' 
 *  in host endianness.
 */
void
le_read64(void *dest, const void *src)
{
	char *cdest = dest;
	const char *csrc = src;

	if (i_am_little_endian) {
		memcpy(dest, src, 8);
	} else {
		int i;
		for (i = 0; i < 8; i++) {
			cdest[i] = csrc[7-i];
		}
	}
}

void
le_write16(void *addr, const unsigned value)
{
	unsigned char *p = addr;
	p[0] = value;
	p[1] = value >> 8;
	
}

void 
le_write32(void *addr, const unsigned value)
{
	unsigned char *p = addr;
	p[0] = value;
	p[1] = value >> 8;
	p[2] = value >> 16;
	p[3] = value >> 24;
}

signed int 
si_round( double d )
{
	if ( d < 0 ) {
		return (signed int)(d-0.5);
	}
	else {
		return (signed int)(d+0.5);
	}
}

/*
 *  Return a time_t suitable for adding to a time_t that is in GMT to
 *  make it a local time.
 *  Obsolete: to use mkgmtime instead.
 */
signed int 
get_tz_offset(void)
{
	time_t now = current_time();
	time_t later = mktime(gmtime(&now));

	if (later == -1) {
		return 0;
	} else {
		return (signed int) difftime(now, later);
	}
}

/*
	mkgmtime -- convert tm struct in UTC to time_t

	works just like mktime but without all the mucking
	around with timezones and daylight savings

	obsoletes get_tz_offset()

	Borrowed from lynx GPL source code
	http://lynx.isc.org/release/lynx2-8-5/src/mktime.c

	Written by Philippe De Muyter <phdm@macqel.be>.
*/

time_t
mkgmtime(struct tm *t)
{
	short  month, year;
	time_t result;
	static int      m_to_d[12] =
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	month = t->tm_mon;
	year = t->tm_year + month / 12 + 1900;
	month %= 12;
	if (month < 0)
	{
		year -= 1;
		month += 12;
	}
	result = (year - 1970) * 365 + m_to_d[month];
	if (month <= 1)
		year -= 1;
	result += (year - 1968) / 4;
	result -= (year - 1900) / 100;
	result += (year - 1600) / 400;
	result += t->tm_mday;
	result -= 1;
	result *= 24;
	result += t->tm_hour;
	result *= 60;
	result += t->tm_min;
	result *= 60;
	result += t->tm_sec;
	return(result);
}

/*
 * mklocaltime: same as mktime, but try to recover the "Summer time flag",
 *              which is evaluated by mktime
 */
time_t
mklocaltime(struct tm *t)
{
	time_t result;
	struct tm check = *t;
	
	check.tm_isdst = 0;
	result = mktime(&check);
	check = *localtime(&result);
	if (check.tm_isdst == 1) {	/* DST is in effect */
		check = *t;
		check.tm_isdst = 1;
		result = mktime(&check);
	}
	return result;
}

/*
 * A wrapper for time(2) that allows us to "freeze" time for testing.
 */
time_t
current_time(void)
{
	if (getenv("GPSBABEL_FREEZE_TIME")) {
		return 0;
	}

	return time(NULL);
}

/*
 * Return the (zero based) month number of the year or -1 for failure.
 */
signed int
month_lookup(const char *m)
{
	static const char *months[] = {
		"JAN", "FEB", "MAR", "APR", "MAY", "JUN", 
		"JUL", "AUG", "SEP", "OCT", "NOV", "DEC", NULL };
	const char **mp;

	for (mp = months; *mp; mp++) {
		if (0 == case_ignore_strcmp(*mp, m))
			return mp - months;
	}
	return -1;
}

double
endian_read_double(void* ptr, int read_le)
{
  double ret;
  char r[8];
  void *p;
  int i;
  
  if ( i_am_little_endian == read_le ) {
	  p = ptr;
  }
  else {
	  for (i = 0; i < 8; i++)
	  {
		r[i] = ((char*)ptr)[7-i];
	  }
	  p = r;
  }
  
  memcpy(&ret, p, 8);
  return ret;
}

float
endian_read_float(void* ptr, int read_le)
{
  float ret;
  char r[4];
  void *p;
  int i;
  
  if ( i_am_little_endian == read_le ) {
	  p = ptr;
  }
  else {
	  for (i = 0; i < 4; i++)
	  {
		r[i] = ((char*)ptr)[3-i];
	  }
	  p = r;
  }
  
  memcpy(&ret, p, 4);
  return ret;
}

void
endian_write_double(void* ptr, double d, int write_le)
{
  char *r = (char *)(void *)&d;
  int i;
  char *optr = ptr;

  if ( i_am_little_endian == write_le ) {
	  memcpy( ptr, &d, 8);
  }
  else {
	  for (i = 0; i < 8; i++)
	  {
		*optr++ = r[7-i];
	  }
  }
}

void
endian_write_float(void* ptr, float f, int write_le)
{
  char *r = (char *)(void *)&f;
  int i;
  char *optr = ptr;

  if ( i_am_little_endian == write_le ) {
	  memcpy( ptr, &f, 4);
  }
  else {
	  for (i = 0; i < 4; i++)
	  {
		*optr++ = r[3-i];
	  }
  }
}

float
le_read_float( void *ptr ) {return endian_read_float(ptr, 1);}

void
le_write_float( void *ptr, float f ) {endian_write_float(ptr,f,1);}

float
be_read_float( void *ptr ) {return endian_read_float(ptr, 0);}

void
be_write_float( void *ptr, float f ) {endian_write_float(ptr,f,0);}

double 
le_read_double( void *ptr ) {return endian_read_double(ptr,1);}

void
le_write_double( void *ptr, double d ) {endian_write_double(ptr,d,1);}

double 
be_read_double( void *ptr ) {return endian_read_double(ptr,0);}

void
be_write_double( void *ptr, double d ) {endian_write_double(ptr,d,0);}


/* Magellan and PCX formats use this DDMM.mm format */
double ddmm2degrees(double pcx_val) {
	double minutes;
	signed int deg;
	deg = (signed int) (pcx_val / 100.0);
	minutes = (((pcx_val / 100.0) - deg) * 100.0) / 60.0;
	return (double) deg + minutes;
}

double degrees2ddmm(double deg_val) {
	signed int deg;
	deg = (signed int) deg_val;
	return (double) (deg * 100.0) + ((deg_val - deg) * 60.0);
}

/*
 * replace a single occurrence of "search" in  "s" with "replace".
 * Returns an allocated copy if substitution was made, otherwise returns NULL.
 * Doesn't try to make an optimally sized dest buffer.
 */
char *
strsub(const char *s, const char *search, const char *replace)
{
       char *p;
       int len = strlen(s);
       int slen = strlen(search);
       int rlen = strlen(replace);
       char *d;

       p = strstr(s, search);
       if (!slen || !p) {
               return NULL;
       }
       
       d = xmalloc(len + rlen);

       /* Copy first part */
       len = p - s;
       memcpy(d, s, len);
       d[len] = 0;

       /* Copy replacement */
       strcat(d, replace);

       /* Copy last part */
       strcat(d, p + slen);
       return d;
}

/*
 *  As strsub, but do it globally.
 */
char *
gstrsub(const char *s, const char *search, const char *replace)
{
	int ooffs = 0;
	char *o, *c;
	char *src = (char *)s;
	int olen = strlen(src);
	int slen = strlen(search);
	int rlen = strlen(replace);

	o = xmalloc(olen + 1);
	
	while ((c = strstr(src, search))) {
		olen += (rlen - slen);
		o = xrealloc(o, olen + 1);
		memcpy(o + ooffs, src, c - src);
		ooffs += (c - src);
		src = c + slen;
		if (rlen) {
			memcpy(o + ooffs, replace, rlen);
			ooffs += rlen;
		}
	}

	if (ooffs < olen)
		memcpy(o + ooffs, src, olen - ooffs);
	o[olen] = '\0';
	return o;
}

/*
 * Like strstr, but starts from back of string.
 */
char *
xstrrstr(const char *s1, const char *s2) 
{
	char *r = NULL, *next = NULL; 

	while (next = strstr(s1, s2), NULL != next) {
		r = next;
		s1 = next + 1;
	}
	return r;
}

/*
 *
 */
char *
strupper(char *src)
{
	char *c;
	
	for (c = src; *c; c++) {
		*c = toupper(*c);
	}
	return src;
}

/*
 *
 */
char *
strlower(char *src)
{
	char *c;
	
	for (c = src; *c; c++) {
		*c = tolower(*c);
	}
	return src;
}

char *
rot13( const char *s )
{
	char *result = xstrdup( s );
	char *cur = result;
	int flip = 1;
	while (cur && *cur ) {
		if ( flip ) {
			if (*cur == '[') flip = 0;
			else if ( *cur >= 'A' && *cur <= 'Z' ) {
				*cur = 'A' + ((*cur-'A')+13)%26;
			}
			else if ( *cur >= 'a' && *cur <= 'z' ) {
				*cur = 'a' + ((*cur-'a')+13)%26;
			}
		}
		else if ( *cur == ']' ) flip = 1;
		cur++;
	}
	return result;
}

/* 
 * Return a decimal degree pair as
 * DD.DDDDD  DD MM.MMM or DD MM SS.S
 * fmt = ['d', 'm', 's']
 * sep = string between lat and lon (separator)
 * html = 1 for html output otherwise text
 */
char *
pretty_deg_format(double lat, double lon, char fmt, const char *sep, int html) 
{
	double  latmin, lonmin, latsec, lonsec;
	int     latint, lonint;
	char	latsig, lonsig;
	char	*result;
	latsig = lat < 0 ? 'S':'N';
	lonsig = lon < 0 ? 'W':'E';
	latint = abs((int) lat);
  	lonint = abs((int) lon);
	latmin = 60.0 * (fabs(lat) - latint);
	lonmin = 60.0 * (fabs(lon) - lonint);
	latsec = 60.0 * (latmin - floor(latmin));
	lonsec = 60.0 * (lonmin - floor(lonmin));
	if (sep == NULL) sep = " ";	/* default " " */
	if (fmt == 'd') { /* ddd */
		xasprintf ( &result, "%c%6.5f%s%s%c%6.5f%s",
			latsig, fabs(lat), html?"&deg;":"", sep,
			lonsig, fabs(lon), html?"&deg;":"" );
	}
	else if (fmt == 's') { /* dms */
		xasprintf ( &result, "%c%d%s%02d'%04.1f\"%s%c%d%s%02d'%04.1f\"",
                        latsig, latint, html?"&deg;":" ", (int)latmin, latsec, sep,
			lonsig, lonint, html?"&deg;":" ", (int)lonmin, lonsec);
	}
	else { /* default dmm */
		xasprintf ( &result,  "%c%d%s%06.3f%s%c%d%s%06.3f",
			latsig, latint, html?"&deg;":" ", latmin, sep,
			lonsig, lonint, html?"&deg;":" ", lonmin);
	} 
	return result;
}



/* 
 * Get rid of potentially nasty HTML that would influence another record
 * that includes;
 * <body> - to stop backgrounds/background colours from being loaded
 * </body> and </html>- stop processing altogether
 * <style> </style> - stop overriding styles for everything
 */
char *
strip_nastyhtml(const char * in)
{
	char *returnstr, *sp;
	char *lcstr, *lcp;
	
	sp = returnstr = xstrdup(in);
	lcp = lcstr = strlower(xstrdup(in));
	
	while (lcp = strstr(lcstr, "<body>"), NULL != lcp) {
		sp = returnstr + (lcp - lcstr) ; /* becomes <!   > */
		sp++; *sp++ = '!'; *sp++ = ' '; *sp++ = ' '; *sp++ = ' ';
		*lcp = '*';         /* so we wont find it again */
	}
	while (lcp = strstr(lcstr, "<body"), lcp != NULL) {   /* becomes <!--        --> */
		sp = returnstr + (lcp - lcstr) ;
		sp++; *sp++ = '!'; *sp++ = '-';  *sp++ = '-';  
		while ( (*sp) && (*sp != '>') ) {
		  sp++;
		}
		*--sp = '-'; *--sp = '-'; 
		*lcp = '*';         /* so we wont find it again */
	}
	while (lcp = strstr(lcstr, "</body>"), NULL != lcp) {
		sp = returnstr + (lcp - lcstr) ; /* becomes <!---- */
		sp++; *sp++ = '!'; *sp++ = '-'; *sp++ = '-'; *sp++ = '-'; *sp++ = '-'; 
		*lcp = '*';         /* so we wont find it again */
	}
	while (lcp = strstr(lcstr, "</html>"), NULL != lcp) {
		sp = returnstr + (lcp - lcstr) ; /* becomes </---- */
		sp++; *sp++ = '!'; *sp++ = '-'; *sp++ = '-'; *sp++ = '-'; *sp++ = '-'; 
		*lcp = '*';         /* so we wont find it again */
	}
	while (lcp = strstr(lcstr, "<style"), NULL != lcp) {
		sp = returnstr + (lcp - lcstr) ; /* becomes <!--   */
		sp++; *sp++ = '!'; *sp++ = '-'; *sp++ = '-';  *sp++ = ' '; *sp++ = ' '; *sp = ' ';
		*lcp = '*';         /* so we wont find it again */
	}
	while (lcp = strstr(lcstr, "</style>"), NULL != lcp) {
		sp = returnstr + (lcp - lcstr) ; /* becomes    --> */
		*sp++ = ' '; *sp++ = ' '; *sp++ = ' '; *sp++ = ' '; *sp++ = ' '; *sp++ = '-'; *sp++ = '-'; 
		*lcp = '*';         /* so we wont find it again */
	}
	while (lcp = strstr(lcstr, "<image"), NULL != lcp) {
		sp = returnstr + (lcp - lcstr) ; /* becomes <img */
		sp+=3; *sp++ = 'g'; *sp++ = ' '; *sp++ = ' ';
		*lcp = '*';
	}
	xfree (lcstr);
	return (returnstr);
}
	

char *get_filename(const char *fname)
{
	char *res, *cb, *cs;
	
	cb = strrchr(fname, '\\');
	cs = strrchr(fname, '/');
	
	if (cb == NULL) res = cs;
	else if (cs == NULL) res = cb;
	else res = (cs > cb) ? cs : cb;
	
	return (res == NULL) ? (char *) fname : ++res;
}
