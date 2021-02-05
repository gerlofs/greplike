Most of the notes made are taken from reading the following resources:

- *Mastering Regular Expressions* by Jeffrey Friedl

- Rob Pike's section on Regular Expressions in *Beautiful Code*.

### Notes on regular expressions and regex engines:

*Regex* is a meta-programming-language used in tools and programming languages to handle tasks which involve pattern matching strings; such as finding duplicated instances of words within a code file,
or finding and removing tags from a HTML page. The *Regex* patterns are strung-together from standard text and special characters that modifies the subsequent or antecedent characters, 
or changes the limits applied by the parser (engine) - for example, `^.*$` is a pattern of compounded special characters that _literally_ (in *regular expression* terms) means "match any character
that appears between the beginning and end of a line zero or more times", thus broadly matching all the text on a line.

