#ifndef UNIT_H
#define UNIT_H
/*
*	See: MinUnit (http://www.jera.com/techinfo/jtns/jtn002.html).
*
*/
#define unit_assert(message, test) do { if (!(test)) return message; } while (0)
#define run_unit_test(test) do { char *message = test(); tests_run++; if(message) return message; } while(0)

extern int tests_run;

#endif /* UNIT_H */