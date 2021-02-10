#include "regex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int regex_find(char *regular_expression, char *line_text) {
	/*  Find the position to start regular expression matching from.
		If the first character of the regular expression is a caret (^) we need to start searching 
			from the start of the line and use the regular expression minus the first character (^) to match against.
		Otherwise search the line character by character in a loop.
	*/
	// Check against ASCII caret only (remove all other bits).
	
	size_t buff_len = strlen(regular_expression);
	char *matched_string = (char *) malloc(buff_len);
	
	// If we have a caret, run the match once only  from the string start (only match in the next _n_ characters where n is strlen(regex+1)).
	if ( regular_expression[0] == 0x5E) return regex_match(regular_expression+1, line_text);
	else {
		do { // Try to match the line, if the line ends, we return zero.
			if ( regex_match(regular_expression, line_text) == 1 ) {
				// line_text pointer offset is currently set to the position of the first character that matched.
				strncpy(matched_string, line_text, buff_len+1);
				matched_string[buff_len] = 0x00;
				printf("%s %s ", line_text, matched_string);
				free(matched_string); // Just to prove we have _actually_ matched.
				return 1;
			}
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
	// If there's a 'none or more' operator we start a different expression check...
	else if ( next_char == 0x2A ) return zero_or_more_match(current_char, regular_expression+2, line_text);
	// If we're not at line end and we can match any character (besides line break) OR the char matches the text, continue matching (recurr).
	else if ( *line_text != 0x00 && ( current_char == 0x2E || current_char == *line_text) )
		return regex_match(regular_expression+1, line_text+1);
	
	// If we reach the end of the regex matching without returning True (1), we've failed to match.
	return 0;
}

int zero_or_more_match(char to_match, char *regular_expression, char *line_text) {
	// Select all characters in the line_text string that matches zero or more times, using leftmost longest matching.
	char *text_ptr; // Create a new read pointer.
	
	// Run through the remaining text identifying where we stop matching (file end, no char match, no '.' operator).
	for (text_ptr = line_text; *text_ptr != 0x00 && (*text_ptr == to_match || to_match == 0x2E); text_ptr++);
	
	do {
		// Match the new string pointer to the regular expression character following the asterisk to trim the string down (see png for example).
		if ( regex_match(regular_expression, text_ptr)) return 1;
	} while (text_ptr-- > line_text); // Move backwards until we reach the 'start' of the text.
	return 0;
}

int main() {
	char* txt_3 = "this and that";
	char* txt_4 = "that and this";
	char* reg = "^this";
	
	printf("%d\n", regex_find(reg, txt_3));
	printf("%d\n", regex_find(reg, txt_4));
}