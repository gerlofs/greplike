#include "regex.h"

// TODO:
// 1. Verify grouping works with restricted inputs
// 2. Implement classes:
//	Add class build functions (with parsing to return node if it exists) [DONE].
//	Add and check class matching [IN PROGRESS].
// 3. Implement input regex validation.
// 4. Implement tests.
// 5. Find and fix inevitable memory leaks.

expression_list *group = NULL;
expression_list *class = NULL;

int regex_find(char *regular_expression, char *line_text) {
	/*  Find the position to start regular expression matching from.
	 *  If the first character of the regular expression is a caret (^) we need to start searching 
		from the start of the line and use the regular expression minus the first character (^) to match against.
	 *  Otherwise search the line character by character in a loop.
	*/

	// If we have a caret, run the match once only from the string start 
	// (only match in the next _n_ characters where n is strlen(regex+1)).
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
	else if ( current_char == 0x28 && *(regular_expression-1) != 0x5C) {
		unsigned offset = ( current_char == 0x28 ) ? 1 : 2;
		expression_list *node = create_group(regular_expression+offset);
		regular_expression += (node->length)+1;
		return multi_match_group(node, ++regular_expression, line_text);	
	}
	else if ( current_char == 0x5B && *(regular_expression-1) != 0x5C) {
		expression_list *node = create_class(++regular_expression);
		while ( *(regular_expression++) != 0x5D );
		return match_class(node, regular_expression+1, line_text);
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
et noswapfile	*	Once we have an end position (where we know there is no longer a match), work backwards to ensure the last
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
	
	if ( match_n > 0 && text_ptr == line_text ) return 0; // If *, ignore this, otherwise check we have actually matched.
	
	do {
		// Match the new string pointer to the regular expression character.
		if ( regex_match(regular_expression, text_ptr)) return 1;
	} while (text_ptr-- > line_text); // Move backwards until we reach the 'start' of the text.
	
	return 0;
}

expression_list *create_class(char *regex_ptr) {
	/*    Create a class struct containing a new expression of characters to match 
	*         including the string length. 
	*     Input must start from the character immediately following the opening bracket and 
	*         must contain a closing bracket, e.g. abc]+d. Following the closing bracket is
	*         the rest of the regular expression to check against (including the optional
	*         metacharacter). 
	*/ 
	char *read_ptr = regex_ptr; // Starts from the character following the opening bracket.
	size_t regex_len = strlen(regex_ptr);

	// Pre-check the class list for a matching class. 
	expression_list *head = class;
	char *class_ptr = NULL;	

	while ( head != NULL ) {
		if ( head->expression != NULL ) {
			for ( read_ptr = regex_ptr, class_ptr = head->expression; 
				// Parse through regular expression, check that each character
				// matches, if so, we can return it.
				*class_ptr != 0x00 && *read_ptr != 0x5D; class_ptr++, read_ptr++) {
				// If the characters at position *n* do not match, check that
				// the character is not a hyphen, if it is, we should jump 
				// the class_ptr ahead by the difference between the next 
				// value in the read_ptr minus the previous one.
				if ( *read_ptr != *class_ptr && *read_ptr == 0x2D && 
				(*(read_ptr+1) != 0x00 && *(read_ptr+1) != 0x5D) ) {
					class_ptr += *(read_ptr+1) - *(class_ptr);
					if ( *class_ptr != *(++read_ptr) ) break;			
				} else if ( *read_ptr != *class_ptr ) break;			
			} 
			
			if ( (*read_ptr == 0x00 || *read_ptr == 0x5D) && *class_ptr == 0x00 ) {
				printf("Found match... %s\n", head->expression);
				return head;			
			}
			// If the pointers are at an end, we can return the node. 

		}
		
		head = head->next;			
	}
	
	// If there's no matching node, we need to create one, reuse the class_ptr and allocate to it.
	class_ptr = (char *) error_checked_malloc(regex_len-1);
	*class_ptr = (char) 0x00;
	char *write_ptr = class_ptr;
	
	for (read_ptr = regex_ptr; *read_ptr != 0x00 && *read_ptr != 0x5D; read_ptr++ ) { // Read from the start of the regex string until we reach a closing bracket.
		// A non-hyphen character is present followed by a hyphen (indicating a range).
		if ( *(read_ptr+1) == 0x2D && *(read_ptr+2) != 0x5D ) {
			// TODO: Add checking for additional hyphens within the class.
			class_ptr = generate_range(class_ptr, strlen(class_ptr), *read_ptr, *(read_ptr+2));
			write_ptr = (class_ptr+strlen(class_ptr));
			read_ptr += 2; // This needs a check along with it.
		} else { 
			*(write_ptr++) = *read_ptr;
			*write_ptr = (char) 0x00;
		}
	}
	
	*write_ptr = (char) 0x00;
	
	expression_list *ec = create_node();
	ec->expression = class_ptr;
	ec->length = strlen(class_ptr);
	ec->match_required = (*(++read_ptr) == 0x2B);

	if ( class == NULL ) class = ec;
	else append_node(class, ec);
	return ec;
}

char *generate_range(char *class_ptr, size_t current_len, char a, char b) {
	/*    Generate a range of ascii values between 'a' and 'b'. 
	*     
	*     TODO: Add clause for inputs being identical, just add char and return.
	*/
	if ( (int) a >  (int) b ) {
		a ^= b;
		b ^= a;
		a ^= b; // Swap vars.
	}
	unsigned range_len = (int) b - (int) a;

	// Reallocate the class_ptr.
	class_ptr = (char *) error_checked_realloc(class_ptr, current_len + range_len);
	char *write_ptr = (class_ptr+current_len);

	for (unsigned len = current_len; len <= (range_len + current_len); len++,write_ptr++) {
		*write_ptr = (char) a++;
	}

	*write_ptr = (char) 0x00;

	return class_ptr;
}

int match_class(expression_list *node, char *regex_ptr, char *line_ptr) {
	/*    Struct contains expression (all the characters that could match) as well 
	*         as a boolean to determine whether we need to match against the chars
	*         or not.  
	*/

	// Match all qualifying characters, if there's not a match regress until we match the 
	// final character in the expression. If there's no match we will reach the end of the
	// class_ptr string (0x00).

	char *read_ptr = line_ptr;
	char *check_ptr = node->expression;

	printf("%s %s\n", check_ptr, read_ptr);

	while ( read_ptr != 0x00 ) {
		if ( *check_ptr == 0x00 ) break;
		else if ( *check_ptr == *read_ptr ) {
			check_ptr = node->expression;
			read_ptr++;
		} else check_ptr++;
	}

	if ( node->match_required && read_ptr == line_ptr ) return 0;

	do { 
		if ( *read_ptr == *regex_ptr ) return 1;
	} while ( read_ptr-- > line_ptr);
	
	return 0;
}

/* GROUPS (Linked List helpers) */
expression_list *create_node() {
	// Initalise a node structure before populating it.
	expression_list *node = (expression_list *) error_checked_malloc(sizeof(expression_list));
	node->expression= NULL;
	node->length = 0;
	node->match_required = 0;
	node->next = NULL;
	return node;
}

void append_node(expression_list *head, expression_list *node_to_append) {
	// Traverse then add node to the end of the linked list. 
	// Note: node_to_add must be populated with data before calling this function.
	expression_list *node = head;
	while ( node->next != NULL ) node = node->next;
	node->next = node_to_append;
}

expression_list *create_group(char *regex_ptr) {
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
	expression_list *node = group;
	
	// Check that group is populated already: parse through regex string checking characters match existing regex group.
	while ( node != NULL ) {
		if ( node->expression != NULL ) {
			char *node_read_ptr = node->expression;
			for ( read_ptr = regex_ptr; *read_ptr == *node_read_ptr; read_ptr++,node_read_ptr++ );
		}
		if ( node->length == (read_ptr - regex_ptr)) return node;
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
	node->expression = (char *) error_checked_malloc(group_len+1);
	strncpy(node->expression, regex_ptr, group_len);
	node->expression[group_len] = (char) 0x00;
	node->length = strlen(node->expression);
	// If char immediately following the closed parenthesis is a * or ?, we do not need to match, set to zero.
	// Otherwise set to 1 and force a match during group matching.
	char regex_opt = *(regex_ptr+2);
	node->match_required = (regex_opt == 0x2B);
	
	if ( group == NULL ) group = node;
	else append_node(group, node);
	return node;
}

void group_teardown() {
	expression_list *head = group;
	expression_list *next = NULL;
	
	while ( head != NULL ) {
			next = head->next;
			free(head);
			head = next;
	}
	
	group = NULL;
}

int multi_match_group(expression_list *node, char *regex_ptr, char *line_ptr) {
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
	char *node_ptr = node->expression;
	
	// Parse, the resulting pointer position (read_ptr) on a good day will contain the last character to match ((abd)+d gives d from 'abcd')
	for (read_ptr = line_ptr; *read_ptr != 0x00 && (*read_ptr == *node_ptr || *node_ptr == 0x2E); read_ptr++) {
		if ( *(node_ptr+1) == 0x00 ) node_ptr -= (node->length-1); // Reset the group pointer.
		else node_ptr++;
	}
	
	// If we need to match and we haven't matched (length of the read_ptr is less than it should be), return with no match.
	if ( node->match_required && (read_ptr-line_ptr) < node->length) return 0;
	
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
	char txt[] = "Languages that parse regular expressions include Perl.";
	char reg[] = "P[a-z]+r";
	printf("%d\n", regex_find(reg, txt));
	//char txt[] = "erf abcabcabcdr abcdeeeffffff";
	//printf("%d\n", regex_find(reg, txt));
	//group_teardown();
	
}


