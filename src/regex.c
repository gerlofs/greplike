/* GREPLIKE */
/* REGEX.C */

#include "regex.h"
#define ANSI_COLOUR_RED 	"\x1b[31m"
#define COLOUR_STOP 		"\x1b[0m"

/// TODO:
// 1. Implement NFA / Thompson method.
// 2. Find and fix inevitable memory leaks.

expression_list *group = NULL;
expression_list *class = NULL;

void print_match(char *line_text, char *match_ptr) {
	/*
	*	In the case of an expression containing a caret (^) which only 
	*	searches in the first set of characters, the easiest solution
	*	is just to print the line up to and including the match.		
	*/

	char *read_ptr = line_text;
	size_t nbytes = 0;
	fwrite(ANSI_COLOUR_RED, sizeof(char), 5, stdout);
	for ( ; *read_ptr != 0x00 && read_ptr != match_ptr; nbytes++,read_ptr++ );
	fwrite(line_text, sizeof(char), nbytes, stdout);
	fwrite(COLOUR_STOP, sizeof(char), 4, stdout);
	fwrite(read_ptr, sizeof(char), sizeof(read_ptr), stdout);
	fwrite("\n", sizeof(char), 1, stdout);
}

void print_many(uint16_t *adr_offsets, uint16_t n_offsets, char *line_start) {
	/*
	 * 	In the general case, there are going to be several matches a line
	 * 	which requires a set of highlighted words within a block of text.
	 * 	Rather than print each line including the match and find where to 
	 * 	properly break the line, we take a copy of the line start address
	 * 	prior to matching and print that line out in chunks.
	 *
	 * 	adr_offsets is an array containing 16-bit integers in sets of two.
	 * 	The first index in each pair is the offset from the start of the 
	 * 	line to get to the matching word, the second index is the length
	 * 	of bytes in the matching word. The general rule is to keep track
	 * 	of what the current offset from the start of the string is to 
	 * 	make sure we're printing the line correctly including matches.
	 * 	e.g. [3, 2, 16, 2]
	 * 	Hel__o world qwe__yiop
	*/
	if (!n_offsets) return;
	uint16_t total_offset = 0;
	for(uint16_t o = 0; o < n_offsets; o+=2) {
		fwrite(line_start+total_offset, sizeof(char), adr_offsets[o]-total_offset, stdout);
		fwrite(ANSI_COLOUR_RED, sizeof(char), 5, stdout);
		fwrite(line_start+adr_offsets[o], sizeof(char), adr_offsets[o+1], stdout);
		fwrite(COLOUR_STOP, sizeof(char), 4, stdout);
		total_offset=adr_offsets[o]+adr_offsets[o+1];
	}
	fwrite(line_start+total_offset, sizeof(char), strlen(line_start)-total_offset, stdout);
	fwrite("\n", sizeof(char), 1, stdout);
}

int regex_find(char *regular_expression, char *line_text) {
	/*  
	 *  Find the position to start regular expression matching from.
	 *  If the first character of the regular expression is a caret (^) we need to start searching 
	 *	from the start of the line and use the regular expression minus the first character (^) to match against.
	 *  Other_charwise search the line character by character in a loop.
	*/
	
	size_t line_len = strlen(line_text);
	char *start_ptr = line_text;
	char *match_ptr = NULL;
	int matched = 0;
		
	if ( line_text[line_len-1] == 0x0A ) line_text[line_len-1] = 0x00;
	// If we have a caret, run the match once only from the string start 
	// (only match in the next _n_ characters where n is strlen(regex+1)).
	if ( regular_expression[0] == 0x5E) {
		match_ptr = regex_match(regular_expression+1, line_text, 0);
		if ( match_ptr != NULL ) {
			print_match(line_text, match_ptr);
			matched++;

		}
	} else {
		uint16_t *adr_offsets = error_checked_malloc(sizeof(uint16_t) * 2);
		uint16_t i = 0;
		//int8_t allow_escaping = 0; // Use of the backslash is only permitted if we're past the first char.
		do { // Try to match the line, if the line ends, we return zero.
			if ( (match_ptr = regex_match(regular_expression, line_text, 0)) != NULL ) {
				adr_offsets[i++] = line_text - start_ptr; // Offset from line start.
				adr_offsets[i++] = match_ptr - line_text;
				adr_offsets = error_checked_realloc(adr_offsets, sizeof(uint16_t) * (i+2));
				line_text = match_ptr;
				matched |= 1;
			}
		} while ( *(line_text++) != 0x00 );
		print_many(adr_offsets, i, start_ptr);
		free(adr_offsets);
	}
	
	return matched;
}

char *regex_match(char *regular_expression, char *line_text, int8_t allow_escaping) {
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
	char current = regular_expression[0]; 
	char next = (current == 0x00) ? (char) 0x00 : regular_expression[1];
	
	// If current regex char is $ and the next regex char is a NULL, check we've exhausted the line_text as well.
	if ( current == 0x24 && next == 0x00 && *line_text == 0x00) { 
		return line_text;
	}
	// If we've exhausted the regex pattern, we've matched.
	else if ( current == 0x00 ) {
		return line_text;
	}
	// If we have an un-escaped bracket, we're grouping.
	else if (*(regular_expression-(1*allow_escaping)) != 0x5C && current == 0x28 ) {
		expression_list *node = create_group(++regular_expression);
		while ( *(regular_expression++) != 0x29 );
		return match_group(node, regular_expression, line_text);	
	}
	// If we have an un-escaped square-bracket, there's a class.
	else if ( *(regular_expression-(1*allow_escaping)) != 0x5C && current == 0x5B ) {
		expression_list *node = create_class(++regular_expression);
		while ( *(regular_expression++) != 0x5D );
		return match_class(node, regular_expression, line_text);
	}
	// A character alternation is present, if regex+0 matches line+0 then we're good, 
	//		otherwise, try to match to the other possible character.
	else if ( current != 0x5C && next == 0x7C ) {
		if ( *line_text != current ) return regex_match(regular_expression+2, line_text, allow_escaping|=1);
		else return regex_match(regular_expression+3, line_text+1, allow_escaping|=1);
	}
	// If next character is an optional, either the next character matches the character after the optional OR the current one does.
	else if ( next == 0x3F ) {
		if ( current == *line_text ) return regex_match(regular_expression+2, line_text+1, allow_escaping);
		else return regex_match(regular_expression+2, line_text, allow_escaping|=1);
	}
	// If there's a 'one or more' or a 'none or more' operator we start a different expression check (see func).
	else if ( next == 0x2A || next == 0x2B ) {
		allow_escaping |= 1;
		return multi_match_single_char((next == 0x2B), current, regular_expression+2, line_text);
	}		
	// If we're not at line end and we can match any character (besides line break) OR the char matches the text, continue matching (recurr).
	// Additionally, if the current regex character is a backslash, we ignore it.
	else if ( (current == 0x5C ) || (*line_text != 0x00 && ( current == 0x2E || current == *line_text)) ) {
		return regex_match(regular_expression+1, line_text+1, allow_escaping|=1);
	}
	// If we reach the end of the regex matching without returning, we've failed to match.
	return NULL;
}

char *multi_match_single_char(unsigned match_n, char to_match, char *regular_expression, char *line_text) {		
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
	
	if ( match_n > 0 && text_ptr == line_text ) return NULL; // If *, ignore this, otherwise check we have actually matched.
	
	char *match_ptr = NULL;
	do {
		// Match the new string pointer to the regular expression character.
		if ( (match_ptr=regex_match(regular_expression, text_ptr, 1)) != NULL ) return match_ptr;
	} while (text_ptr-- > line_text); // Move backwards until we reach the 'start' of the text.
	
	return NULL;
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

	if (!regex_len) { // Check for an empty expression, can cause int rollover otherwise.
		fprintf(stderr, "Empty regular expression found, check that the input expression is valid.\n");
		exit(1);
	}

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
				if ( *(read_ptr+1) == head->match_char ) return head;
				else if ( head->next == NULL ) break;
				else head = head->next;
			}
			// If the pointers are at an end, we can return the node. 

		}
		
		head = head->next;			
	}

	// Parse and check there is an end bracket. 
	int valid_expr = 0;
	read_ptr = regex_ptr;
	while ((valid_expr |= (*(read_ptr++) == 0x5D)) <= 0 && *read_ptr != 0x00);
	if ( !valid_expr ) {
		fprintf(stdout, "No matching ] found for given class: %s\n", regex_ptr);
		exit(1);
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
	read_ptr++;
	ec->match_flags = !(*read_ptr == 0x2A || *read_ptr == 0x3F);
	ec->match_char = *read_ptr;

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
	uint16_t range_len = (int) b - (int) a;
	// Reallocate the class_ptr.
	class_ptr = (char *) error_checked_realloc(class_ptr, (current_len + (range_len+1))+1);
	uint16_t len;

	for (len = current_len; len <= (range_len + current_len); len++) {
		class_ptr[len] = (char) a++;
	}

	class_ptr[len] = (char) 0x00;
	return class_ptr;
}

char *match_class(expression_list *node, char *regex_ptr, char *line_ptr) {
	/*    Struct contains expression (all the characters that could match) as well 
	*         as a boolean to determine whether we need to match against the chars
	*         or not.  
	*/
	
	char *read_ptr = line_ptr;
	char *check_ptr = node->expression;
	int match_multiple = (*regex_ptr == 0x2B || *regex_ptr == 0x2A);
	
	// TODO: Fix class matching, should match only characters within the class, intermediates chars are disallowed.
	// e.g. [l]+s would not match class, [l]*s would because l is optional.
	
	while ( read_ptr != 0x00 ) {
		if ( *check_ptr == 0x00 ) break;
		else if ( *check_ptr == *read_ptr ) {
			read_ptr++;
			if ( !match_multiple ) break;
			else check_ptr = node->expression;
		} else check_ptr++;
	}

	if ( is_bit_set(node->match_flags, 0) && read_ptr == line_ptr ) return NULL;
		
	if ( match_multiple || *regex_ptr == 0x3F ) regex_ptr++;

	char *match_ptr = NULL;
	do { 
		if ( (match_ptr=regex_match(regex_ptr, read_ptr, 1)) ) return match_ptr;
	} while ( read_ptr-- > line_ptr);
	
	return NULL;
}

/* GROUPS (Linked List helpers) */
expression_list *create_node() {
	// Initalise a node structure before populating it.
	expression_list *node = (expression_list *) error_checked_malloc(sizeof(expression_list));
	node->expression= NULL;
	node->length = 0;
	node->match_flags = 0;
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
		if ( node->length == (read_ptr - regex_ptr)) {
			if ( *(read_ptr+1) == node->match_char ) {
				return node;
			}
			else node = node->next;
		}
		else node = node->next;
	}
	
	int alternation_check = 0;
	// If the node isn't populated to begin with (no groups), parse the new group string.
	if ( read_ptr == regex_ptr ) {
		for ( ; *read_ptr != 0x29; read_ptr++) {
			if ( *read_ptr == 0x00 ) {
				fprintf(stderr, "No closed parentheses found, check the expression is valid\n");
				exit(1);
			}
			if ( !alternation_check && *read_ptr == 0x7C ) alternation_check++;
		}
	}

	size_t group_len = (read_ptr - regex_ptr); // Size of group in bytes.

	// Assign, allocate, and populate group linked list node with the group expression string.
	node = create_node();
	node->expression = (char *) error_checked_malloc(group_len+1);
	strncpy(node->expression, regex_ptr, group_len);
	node->expression[group_len] = (char) 0x00;
	// If char immediately following the closed parenthesis is a * or ?, we do not need to match, set to zero.
	// Otherwise set to 1 and force a match during group matching.

	// Get to the end of the group to find any optional metacharacters.
	for (; *read_ptr != 0x00 && *(read_ptr-1) != 0x29; read_ptr++ );
	uint8_t flags = 1 * ( *read_ptr != 0x2A && *read_ptr != 0x3F ) + 
			2 * ( *read_ptr == 0x2A || *read_ptr == 0x2B)  +
			4 * ( alternation_check);
		
	
	node->match_flags = flags;
	node->match_char = *read_ptr;
	node->length = strlen(node->expression);
	if ( group == NULL ) group = node;
	else append_node(group, node);
	return node;
}

void node_teardown() {
	expression_list *head = group;
	expression_list *next = NULL;
	
	while ( head != NULL ) {
			next = head->next;
			free(head);
			head = next;
	}
	
	group = NULL;
	
	head = class;
	next = NULL;
	
	while ( head != NULL ) {
			next = head->next;
			free(head);
			head = next;
	}
	
	class = NULL;
}

char *match_group(expression_list *node, char *regex_ptr, char *line_ptr) {
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
	
	/*
		TODO: Implement system that checks alternations in tandem, additionally check whole line from start point.
	   (ab|cd)
		^ n_p
			^ n_n_p
			^ n_p
	*/

	enum flags {
		PARTIAL = 1,
		MULTI = 2,
		FULL = 4
	};
	
	char *read_ptr = line_ptr; 
	char *node_ptr = node->expression;
	uint8_t match_flags = 0;
	 /* Match flag bits:
	 *	0: Partially matching within an alternation. [1]
	 *	1: Fully matching [2]
	 *	2: Matched a previous partial but attempting a subsequent match (* or +). [4]
	 */
	
	int single_match = 0;
	int alt = is_bit_set(node->match_flags, 2);
	int multi = is_bit_set(node->match_flags, 1);
	int req = is_bit_set(node->match_flags, 0);

	while ( match_flags < FULL && *read_ptr != 0x00 && *node_ptr != 0x00 ) {	
		// Store the current pointer position for comparison.
		char *p_store = node_ptr;
		
		// Are we matching a single character?
		single_match = (*read_ptr == *node_ptr);
		// Set bit conditionally.
		match_flags ^= (-single_match ^ match_flags) & 1; 
		// Move the pointers 
		read_ptr += single_match;
		node_ptr += single_match;
		if ( p_store == node_ptr ) { // No match.
			if ( alt ) {
				for ( ; *node_ptr != 0x00 && *node_ptr != 0x7C; node_ptr++);
				node_ptr++; // Remove pipe, can't be done inside the for loop.
			} else break;
		}
		
		// Set match_flags to 3 if we're matched and the next char is a pipe or end point.
		single_match = (single_match && (*node_ptr == 0x00 || *node_ptr == 0x7C));
		match_flags ^= (-single_match ^ match_flags) & FULL;
		if ( multi && single_match ) { 	
			// If we've matched and might need to match again, use a new flag which can be checked 		
			match_flags &= ~(1UL << 2); // Disable FULL flag.
			match_flags |= (1UL << 1); // Enable MULTI flag.
			node_ptr = node->expression;
		}
	}

	// Check that we're matching (if only bit 0 is set, we've not matched).
	if ( req && match_flags < MULTI ) return NULL;
	else if ( *regex_ptr == 0x00 ) {
		if ( !req || ( req && match_flags >= MULTI ) ) return read_ptr;
	}
	
	regex_ptr += (*regex_ptr == 0x2A || *regex_ptr == 0x2B || *regex_ptr == 0x3F);

	char *match_ptr = NULL;
	do { // Match similarly to the single character match in multi_match_single_char.
		if ( (match_ptr=regex_match(regex_ptr, read_ptr, 1)) ) return match_ptr;
	} while (read_ptr-- > line_ptr);
	
	return NULL;	

}
