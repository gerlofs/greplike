#include "regex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// TODO:
// 1. Implement + operator (similar to asterisk, seek all possible matches and work backwards to eliminate invalid character matches).
// 2. Implement grouping with ( and ) (unescaped). Might have to stop matching (or simply do this before starting to match), move the encapsulated expression into a new string and 
//		run a seperate regex on it using the subsequent operator (if there is one, if there's not we just ignore the parentheses).
// 3. Implement optional operator (?): Ignore the character entirely and continue the match on line_text+1 with regex+2. [DONE].

int regex_find(char *regular_expression, char *line_text) {
	/*  Find the position to start regular expression matching from.
		If the first character of the regular expression is a caret (^) we need to start searching 
			from the start of the line and use the regular expression minus the first character (^) to match against.
		Otherwise search the line character by character in a loop.
	*/
	// Check against ASCII caret only (remove all other bits).
	
	// If we have a caret, run the match once only  from the string start (only match in the next _n_ characters where n is strlen(regex+1)).
	if ( regular_expression[0] == 0x5E) return regex_match(regular_expression+1, line_text);
	else {
		do { // Try to match the line, if the line ends, we return zero.
			if ( regex_match(regular_expression, line_text) == 1 ) return 1;
		} while ( *(line_text++) != 0x00 );
	}
	return 0;
}

int regex_match(char *regular_expression, char *line_text) {
	char current_char = regular_expression[0]; char next_char = regular_expression[1];
	printf("%c:%c, %c\n", current_char, next_char, *line_text);
	
	// If current regex char is $ and the next regex char is a NULL, check we've exhausted the line_text as well.
	if ( current_char == 0x24 && next_char == 0x00 ) return (*line_text == 0x00);
	// If we've exhausted the regex pattern, we've matched.
	else if ( current_char == 0x00 ) return 1;
	// If next character is an optional, either the next character matches the character after the optional OR the current one does.
	else if ( next_char == 0x3F ) return (regex_match(regular_expression+2, line_text) || regex_match(regular_expression+2, line_text+1));
	// If there's a 'one or more' or a 'none or more' operator we start a different expression check (see func).
	else if ( next_char == 0x2A ) return multi_match(0, current_char, regular_expression+2, line_text);
	else if ( next_char == 0x2B ) return multi_match(1, current_char, regular_expression+2, line_text);
	// If we're not at line end and we can match any character (besides line break) OR the char matches the text, continue matching (recurr).
	else if ( *line_text != 0x00 && ( current_char == 0x2E || current_char == *line_text) )
		return regex_match(regular_expression+1, line_text+1);
	
	// If we reach the end of the regex matching without returning True (1), we've failed to match.
	return 0;
}

int multi_match(unsigned match_n, char to_match, char *regular_expression, char *line_text) {	
	// Select all characters in the line_text string that matches zero or more times, using leftmost longest matching.
	char *text_ptr; // Create a new read pointer.
	
	// Run through the remaining text identifying where we stop matching (file end, no char match, no '.' operator).
	for (text_ptr = line_text; *text_ptr != 0x00 && (*text_ptr == to_match || to_match == 0x2E); text_ptr++);
	
	if ( match_n == 0 && text_ptr == line_text ) return 0; // If '+' is used and we haven't found any matches, we fail here.
	
	do {
		// Match the new string pointer to the regular expression character following the asterisk to trim the string down (see png for example).
		if ( regex_match(regular_expression, text_ptr)) return 1;
	} while (text_ptr-- > line_text); // Move backwards until we reach the 'start' of the text.
	return 0;
}

int main() {
	char* txt = "A suit of armouuuur for the king";
	char* reg = "armou+r";
	
	printf("%d\n", regex_find(reg, txt));
}