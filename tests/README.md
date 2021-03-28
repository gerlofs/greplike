Tests are performed by bash scripts, running the greplike program with a series of expression inputs. 

1. The **invalid** input test case.

Check various input expressions don't cause segfaults and other nefarious happenings.

Check invalid filenames are avoided (files that don't exist, null bytes, etc).

Check invalid options are parsed and flagged (this isn't _really_ needed as they can be avoided but it's useful for the user to know).

2. The **valid** input test case.

Check a list of valid expressions against a single file.

Check a list of valid expressions against multiple files.

3. The **single file** tests.

Check a list of expressions on one of three files.

Each should return a success code.

4. The **multi-file** test.

Check a list of expressions against all three files.

Each should return a success code.

5. **Timing** test case.

Run a list of valid expressions against a list of files using both grep and greplike, report the times of both commands using `time ...`.
