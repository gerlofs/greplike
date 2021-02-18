#include "regex.h"

// TODO:
// 1. Verify grouping works with restricted inputs
// 2. Implement classes.
// 3. Implement input regex validation.
// 4. Implement tests.

groups *group = NULL;

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
	/*	Take an input regular expression and an input line of text (usually from a file).
	*	The first and second characters of the regular expression pointer determines the how to parse and check the line.
	*		[0] == $ (0x24) || \0 (0x00) checks the regular expression and line text are both empty.
	*		[0] == ( (0x28) [unescaped] starts a match-by-group (group a set of chars and check them all).
	*		[1] == ? (0x3F) ignores the current and next characters and splits the regex checking into two branches (the 
	*			character following the optional characters must either be at position n or position n+1, so check both.
	*		[1] == * (0x2A) attempts to match against the current character and the successive character at regular_expression+2,
	*			the former is allowed to fail.
	*		[1] == + (0x2B) is the same as the above but requires both characters match.
	*		Default: If the line is not at an end (0x00) and [0] is . or a matching character (== *line_text), continue matching
	*			by recursively calling regex_match with each pointer incremented.
	*/
	
	char current_char = regular_expression[0]; char next_char = regular_expression[1];
	
	// If current regex char is $ and the next regex char is a NULL, check we've exhausted the line_text as well.
	if ( current_char == 0x24 && next_char == 0x00 ) return (*line_text == 0x00);
	// If we've exhausted the regex pattern, we've matched.
	else if ( current_char == 0x00 ) return 1;
	// If we have an un-escaped bracket, we're grouping.
	else if ( current_char == 0x28 && (*regular_expression-1) != 0x5C) {
		unsigned offset = ( current_char == 0x28 ) ? 1 : 2;
		groups *node = create_group(regular_expression+offset);
		regular_expression += (node->regex_len)+1;
		return multi_match_group(node, ++regular_expression, line_text);	
	}
	// If next character is an optional, either the next character matches the character after the optional OR the current one does.
	else if ( next_char == 0x3F ) {
		if ( current_char == *line_text ) return regex_match(regular_expression+2, line_text+1);
		else return regex_match(regular_expression+2, line_text);
	}
	// If there's a 'one or more' or a 'none or more' operator we start a different expression check (see func).
	else if ( next_char == 0x2A || next_char == 0x2B ) return multi_match_single_char((next_char == 0x2B), current_char, regular_expression+2, line_text);
	// If we're not at line end and we can match any character (besides line break) OR the char matches the text, continue matching (recurr).
	// Additionally, if the current regex character is a backslash, we ignore it.
	else if ( (current_char == 0x5C ) || (*line_text != 0x00 && ( current_char == 0x2E || current_char == *line_text)) )
		return regex_match(regular_expression+1, line_text+1);
	
	// If we reach the end of the regex matching without returning True (1), we've failed to match.
	return 0;
}

int multi_match_single_char(unsigned match_n, char to_match, char *regular_expression, char *line_text) {		
	/*	
	*	Select all characters in the line_text string that matches zero or more times, using leftmost longest matching.
	*	Starting from the position in the line string that *should* contain a match, increment the line pointer as long
	*		as there IS a match OR the regular expression character to match is a period/dot (match anything) AND
	*		we haven't reached the end of the line.
	*	Once we have an end position (where we know there is no longer a match), work backwards to ensure the last
	*		character of this partial line is next regular expression character.
	*	If none of the above fail, we can return true (found a match) if, and only if, we are matching against a 
	*		'zero or more' clause (* operator), otherwise we have to check there really is a match by comparing the
	*		pointers against each other before returning true/false.
	*
	*	Example: (.*) can be used to find all the content between two parentheses, the way this works is actually
	*		kind of annoyingly wrong, matching ALL TEXT between two parentheses working backwards from the line end.
	*		so "(flag) this is not a (flag)" will be matched across the whole line rather than in two discrete matches.
	*		This is because ')' is treated as a valid match during the initial for loop and is not removed from the 
	*		final match because a parenthesis is reached and the function is ended.
	*/
	
	char *text_ptr; // Create a new read pointer.
		
	// Run through the remaining text identifying where we stop matching (file end, no char match, no '.' operator).
	for (text_ptr = line_text; *text_ptr != 0x00 && (*text_ptr == to_match || to_match == 0x2E); text_ptr++); 
	
	do {
		// Match the new string pointer to the regular expression character.
		if ( regex_match(regular_expression, text_ptr)) { 
			if ( match_n > 0 && text_ptr == line_text ) return 0; // If *, ignore this, otherwise check we have actually matched.
			return 1;
		}
	} while (text_ptr-- > line_text); // Move backwards until we reach the 'start' of the text.
	
	return 0;
}

/* GROUPS (Linked List helpers) */
groups *create_node() {
	// Initalise a node structure before populating it.
	groups *node = (groups *) error_checked_malloc(sizeof(groups));
	node->regex = NULL;
	node->regex_len = 0;
	node->need_to_match = 0;
	node->next = NULL;
	return node;
}

void print_list()
{
	for (groups *node = group; node != NULL; node = node->next) printf("%s, %u\n", node->regex, node->regex_len);
}

void append_node(groups *head, groups *node_to_append) {
	// Traverse then add node to the end of the linked list. 
	// Note: node_to_add must be populated with data before calling this function.
	groups *node = head;
	while ( node->next != NULL ) node = node->next;
	node->next = node_to_append;
}

groups *create_group(char *regex_ptr) {
	/*	Create and allocate a group to the global linked list (var groups* group). 
	*	A group is simply a struct containing a regular expression (a partial of the full expression given to 
	*		regex_match) string, the length of that string, and bit/flag signifying whether that expression
	*		is an optional match or not. This struct is then appended to a global linked list. 
	*	
	*	A group is created by parsing through the regular expression string (pointer starting from the position of
	*		the opening parenthesis + 1) until the closing parenthesis is reached. The characters in between are checked
	*		against nodes in the linked list until we find a match (returning the matched node). If there's no match, 
	*		the sub-string is copied into a newly allocated node which is then appended to the global list (after populating
	*		with string length and match conditions). 
	*	
	*	The group node is returned after the global list is updated.
	*/
	
	char *read_ptr = regex_ptr;
	groups *node = group;	
	
	// Check that group is populated already: parse through regex string checking characters match existing regex group.
	while ( node != NULL ) {
		if ( node->regex != NULL ) {
			char *node_read_ptr = node->regex;
			for ( read_ptr = regex_ptr; *read_ptr == *node_read_ptr; read_ptr++,node_read_ptr++ );
		}
		if ( node->regex_len == (read_ptr - regex_ptr)) return node;
		else node = node->next;
	}
	
	// If the node isn't populated to begin with (no groups), parse the new group string.
	if ( read_ptr == regex_ptr ) for ( ; *read_ptr != 0x29; read_ptr++) {
		if ( *read_ptr == 0x00 ) {
			fprintf(stderr, "Invalid regex provided, null byte found instead of end parenthesis");
			exit(0);
		}
	}

	size_t group_len = (read_ptr - regex_ptr); // Size of group in bytes.
	
	// Assign, allocate, and populate group linked list node with the group expression string.
	node = create_node();
	node->regex = (char *) error_checked_malloc(group_len+1);
	strncpy(node->regex, regex_ptr, group_len);
	node->regex[group_len] = (char) 0x00;
	node->regex_len = strlen(node->regex);
	// If char immediately following the closed parenthesis is a * or ?, we do not need to match, set to zero.
	// Otherwise set to 1 and force a match during group matching.
	char regex_opt = *(regex_ptr+2);
	node->need_to_match = (regex_opt != 0x2A && regex_opt != 0x3F);
	
	if ( group == NULL ) group = node;
	else append_node(group, node);
	return node;
}

void group_teardown() {
	groups *head = group;
	groups *next = NULL;
	
	while ( head != NULL ) {
			next = head->next;
			free(head);
			head = next;
	}
	
	group = NULL;
}

int multi_match_group(groups *node, char *regex_ptr, char *line_ptr) {
	/*	Match a group, a set of literal characters - metacharacters are not supported - within a
	*		set of parentheses. e.g. (abc) followed by an optional metacharacter that determines 
	*		how many times, if at all the group should be matched. 
	*		e.g. (abc)+d
	*		> derabcabcd (MATCH)
	*		> ababd (NO MATCH)
	*		> abcf (NO MATCH)
	*
	*	Groups are matched in a similar way to matching single characters followed by metacharacters
	*		e.g. 'a+' 'b?' 'c*'. The groups partial regular expression extracted from the input regular
	*		expression is used to match against characters in the line, then we backtrack to find the
	*		character immediately following any optional metacharacters (e.g. (abc)+d, 'd') and attempt
	*		to match that. If we must match (+ following the group) then we also check the length of the 
	*		remaining string (minus the character to match as discussed prior) is equal to the length of 
	*
	*/
	
	char *read_ptr;
	char *node_ptr = node->regex;
	
	// Parse, the resulting pointer position (read_ptr) on a good day will contain the last character to match ((abd)+d gives d from 'abcd')
	for (read_ptr = line_ptr; *read_ptr != 0x00 && (*read_ptr == *node_ptr || *node_ptr == 0x2E); read_ptr++) {
		if ( *(node_ptr+1) == 0x00 ) node_ptr -= (node->regex_len-1); // Reset the group pointer.
		else node_ptr++;
	}
	
	// If we need to match and we haven't matched (length of the read_ptr is less than it should be), return with no match.
	if ( node->need_to_match && (read_ptr-line_ptr) < node->regex_len) return 0;
	
	// If there is a metacharacter proceeding the closing parenthesis, we need to increment to avoid trying to match it.
	// Note: Not having a metacharacter is entirely valid so we have to check for them here before continuing.
	regex_ptr += ( *regex_ptr == 0x2B || *regex_ptr == 0x2A || *regex_ptr == 0x3F );

	do { // Match similarly to the single character match in multi_match_single_char.
		if ( regex_match(regex_ptr, read_ptr)) return 1;
	} while (read_ptr-- > line_ptr);
	
	return 0;
}

int main(void) {
	// colou?r 
	// coloring - if it doesn't match, continue regex+2 at current line pointer position.
	// colouring - if it does match, continue as normal with regex+2.
	char reg[] = "(abc)+d e(fgh)+i";
	char txt[] = "erf abdabcabcd efghij";
	printf("%d\n", regex_find(reg, txt));
	group_teardown();
}
