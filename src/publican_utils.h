#pragma once

extern inline bool IsLineEnd(char c)
{
	bool result = ((c == '\n') || (c == '\r'));
	return(result);
}

extern inline bool IsWhitespace(char c)
{
	bool result = ((c == ' ') || (c == '\t') ||
		      (c == '\v') || (c == '\f') ||
		      IsLineEnd(c));
	return(result);
}

extern inline bool StringsAreEqual(uintptr_t length, 
				   char *a,
				   char *b)
{
	bool result = false;
	if(b) {
		char *at = b;
		for(uintptr_t i = 0; i < length; ++i, ++at) {
			if((*at == 0) || (a[i] != *at)) {
				return(false);
			}
		}
		result = (*at == 0);
	} else {
		result = (length == 0);
	}
	return(result);
}

static int32_t IntFromZInternal(char **atInit)
{
	int32_t result = 0;
	char *at = *atInit;
	while((*at >= '0') && (*at <= '9')) {
		result *= 10;
		result += (*at - '0');
		++at;
	}
	*atInit = at;
	return(result);
}

extern int32_t IntFromZ(char *at)
{
	//char *ignored = at;
	int32_t result = IntFromZInternal(&at);
	return(result);
}