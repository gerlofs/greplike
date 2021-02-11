#include <stdio.h>
#include <string.h>
#include "../../regex.h"
#include "../../../file.h"

typedef struct test_res_struct {
	unsigned tests_run;
	unsigned tests_passed;
	unsigned tests_failed;
	unsigned *fail_log;
} test_results;

void log_fail(test_results *res_ptr) {
	res_ptr->tests_failed++;
	res_ptr->fail_log = realloc(res_ptr->fail_log, res_ptr->tests_failed * sizeof(unsigned));
	res_ptr->fail_log[res_ptr->tests_failed-1] = res_ptr->tests_run;
}

 test_results test_single_lines() {
	// Read in test plain-text file: The format sets the regular expression on one line and the test string on the next line.
	FILE *fp = fopen("single_line_tests", "r"); 
	char *line_ptr = NULL;
	char *regex_ptr = malloc(2048);
	test_results *results = malloc(sizeof(test_results));
	results->fail_log = calloc(1, sizeof(unsigned));
	results->tests_passed = results->tests_run = results->tests_failed = 0;
	
	while ( (line_ptr = read_line(fp)) != NULL ) {
		strncpy(regex_ptr, line_ptr, strlen(line_ptr));
		if((line_ptr = read_line(fp)) != NULL) {
			if ( regex_find(regex_ptr, line_ptr) ) results->tests_passed++;
			else log_fail(results);
		} else log_fail(results);
		results->tests_run++;
	} 
	return *results;
}

int main() {
	test_results result = test_single_lines();
	printf("%u: %u: %u: %u\n", result.tests_run, result.tests_passed, result.tests_failed, result.fail_log[0]);
}