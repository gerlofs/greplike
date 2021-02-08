Most of the notes made are taken from reading the following resources:

- *Mastering Regular Expressions* by Jeffrey Friedl

- Rob Pike's section on Regular Expressions in *Beautiful Code*.

### Notes on regular expressions and regex engines:

*Regex* is a meta-programming-language used in tools and programming languages to handle tasks which involve pattern matching strings; such as finding duplicated instances of words within a code file,
or finding and removing tags from a HTML page. The *Regex* patterns are strung-together from text (*literals*) and special characters (*metacharacters*) that modifies the subsequent or antecedent characters, 
or changes the limits applied by the parser (engine) - for example, `^.*$` is a pattern of compounded special characters that _literally_ (in *regular expression* terms) means "match any character
that appears between the beginning and end of a line zero or more times", thus broadly matching all the text on a line.

- The caret (^) and dollar ($) denote where in the line the pattern must reside (`[a-z]$` matches a single lower-case character at the end of the line, which, I suppose, is useful for finding missing periods ).

- The square brackets denote a *character class* which matches a list or range or characters, in the example above the class encompasses all lower-case characters from a to z. Another really useful example for error-prone spellers is to substitute literals for classes in the middle of patterns: 

```
differ[ea]nce =>
	That is the difference between difference and differance.
	> difference
	> difference
	> differance
```

- Combining the caret and the character class together negates the class so that all characters that are **not** in the class are treated as matches. This requires a character to follow, as shown in the example of searching a word list with `[Qq][^u]` which excludes the word *Iraq* as the final q is not followed by a letter.

- The period `.` meta-character denotes any other possible character, e.g. searching a set of numbers separated by a number of special characters (points, dashes, slashes, etc) can be deduced and searched for a specific sub-set using `01.23.45.67`.

- Alternative sub-expressions can be implemented using the pipe | character, which means *this* OR *that* (`this|that`). A pipe can replace the need for a class if we only need one of two possible options, e.g. in the case of finding misspelled words such as *differance* we could use `differ(a|e)nce` instead of `differ[ea]nce`.

- The parenthesis in the previous example create an inner-scope that allows for a pattern to have a class-like selection in the middle of a string (*alternation*). If they were not used, the previous example would look for instances of `differa` or `ence` instead.

- Classes can match multiple singular characters, allowing for a broad range of possible options. Alternations are used only when we want to reference a limited number of patterns within an existing pattern (alternations can be regular expressions, e.g. `^(From|Subject|Date):` or ...

```
F...(an|ish|pino|ese) =>
> Fijian
> Filipino
> Finnish
> Faroese
X French
```

