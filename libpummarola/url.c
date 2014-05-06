/*
 * This is my attempt at being fancy with parsers, heavily
 * inspired by Matthew Endsley's tinyhttp:
 * 	https://github.com/mendsley/tinyhttp
 * 	https://mendsley.github.io/2012/12/19/tinyhttp.html
 * and, in turn, Bjoern Hoehrmann's UTF-8 parser:
 * 	http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
 *
 * Of course, this is done with none of the elegance of the
 * above.
 *
 * It's entirely possible I've overcomplicated things. This
 * parser is not as simple as the ones above, mostly because
 * these URIs can skip around a bit between classes. That is,
 * the URI can be parsed with or without the protocol, with
 * or without a port number, with or without a query string,
 * etc. But there are probably cleaner ways of doing it.
 *
 * This does not support URIs with username/password, although
 * it should be relatively easy to add that if desired.
 *
 * (c) 2014 Claudio Leite <leitec@staticky.com>
 */

#include "libpummarola.h"

#define _TMP_BUF_SIZE 64

/* Begin parser-related things */

/*
 * The different return values we can get back from
 * url_char. The "hostname_end_port_begin" is weird: it
 * signals the end of a value, and the first character
 * of the next. This is due to ambiguity: ':' can mean
 * either of:
 * 	http-:-// end of protocol, or
 * 	hostname:1234 
 *
 * This isn't super robust on edge cases. It made sense
 * to skip most error handling, since this is intended
 * for a library not really taking user input.
 */
enum {
	protocol_end,
	hostname_end,
	port_end,
	path_end,
	qstring_end,
	hostname_end_port_begin,
	protocol_char,
	hostname_char,
	port_char,
	path_char,
	qstring_char,
	invalid_char,
	ignore_char
};

#define CHAR_TYPES_START protocol_char

/*
 * The lower nybble contains the new state. The higher
 * indicates if this is a transition or a character
 * to save:
 * 	1	straightforward transitions
 * 	2	"lookahead" transitions
 * 	3	used for transitions where 
 * 		the type of value is ambiguous
 * 		(i.e. we get to the same state
 * 		 from multiple previous states)
 *  	8	a character to save (i.e. write to
 *  		buffer)
 *
 * The "lookahead" is not really that, it's checking
 * the character after an initial ':' to see if we
 * are starting the '://' block, going into sepA and B,
 * or starting the port number. These are the xx_end_yy_begin
 * return values above.
 */
uint8_t state_table[] = {
	/* all   int   ':'   '/'   '?'   '\0'  PAD   PAD  */
/* init */ 0x80, 0x80, 0x01, 0x15, 0xff, 0x15, 0x00, 0x00,
/* sepA */ 0xff, 0x24, 0xff, 0x02, 0xff, 0xff, 0x00, 0x00,
/* sepB */ 0xff, 0xff, 0xff, 0x13, 0xff, 0xff, 0x00, 0x00,
/* host */ 0x83, 0x83, 0x14, 0x15, 0xff, 0x15, 0x00, 0x00,
/* port */ 0xff, 0x84, 0xff, 0x35, 0xff, 0x35, 0x00, 0x00,
/* path */ 0x85, 0x85, 0x85, 0x85, 0x16, 0x16, 0x00, 0x00,
/* qstr */ 0x86, 0x86, 0x86, 0x86, 0xff, 0x17, 0x00, 0x00
};

static inline int url_char(uint8_t * state, const char ch);

static inline int url_char(uint8_t * state, const char ch)
{
	int class;
	uint8_t *newstate;

	if (isdigit(ch))
		class = 1;
	else if (ch == ':')
		class = 2;
	else if (ch == '/')
		class = 3;
	else if (ch == '?')
		class = 4;
	else if (ch == '\0')
		class = 5;
	else
		class = 0;

	newstate = state_table + (*state * 8) + class;
	*state = *newstate & 0x0f;

	switch (*newstate) {
	case 0x13:
		return protocol_end;
	case 0x14:
	case 0x15:
		return hostname_end;
	case 0x16:
		return path_end;
	case 0x17:
		return qstring_end;
	case 0x24:
		return hostname_end_port_begin;
	case 0x35:
		return port_end;
	case 0x80:
		return protocol_char;
	case 0x83:
		return hostname_char;
	case 0x84:
		return port_char;
	case 0x85:
		return path_char;
	case 0x86:
		return qstring_char;
	case 0xff:
		return invalid_char;
	}

	return ignore_char;
}

/* PROTO */
int url_parse(const char *url, url_t * components)
{
	size_t bsize, len;
	uint8_t state = 0;
	int i, j, ret;
	char *buf;

	len = strlen(url);
	buf = malloc(_TMP_BUF_SIZE + 1);
	bsize = _TMP_BUF_SIZE;

	for (i = 0, j = 0; i <= len; i++) {
		ret = url_char(&state, url[i]);
		if (ret < CHAR_TYPES_START) {
			buf[j] = '\0';
			j = 0;
		}

		switch (ret) {
		case protocol_end:
			components->protocol = strdup(buf);
			break;
		case hostname_end:
			components->hostname = strdup(buf);
			break;
		case port_end:
			components->port = (uint16_t) atoi(buf);
			break;
		case path_end:
			components->path = strdup(buf);
			break;
		case qstring_end:
			components->query_string = strdup(buf);
			break;
		case hostname_end_port_begin:
			components->hostname = strdup(buf);
			/* actually the first char of port */
		case protocol_char:
		case hostname_char:
		case port_char:
		case path_char:
		case qstring_char:
			if (j >= bsize) {
				bsize += _TMP_BUF_SIZE;
				buf = realloc(buf, bsize);
			}

			buf[j++] = url[i];
			break;
		case invalid_char:
			goto error;
		case ignore_char:
			break;
		}
	}

	free(buf);
	return 1;
 error:
	free(buf);
	return 0;
}

/* PROTO */
void url_free(url_t * url)
{
	if (url->protocol)
		free(url->protocol);
	if (url->hostname)
		free(url->hostname);
	if (url->path)
		free(url->path);
	if (url->query_string)
		free(url->query_string);
}
