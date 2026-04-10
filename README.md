# CSE 220 Homework 6

## Student Information

- Name: Aninda Saprotiv Roy
- SBU Username: aninroy
- SBUID: 116485388
- Course: CSE 220 (Spring 2026)

## Assignment Summary for homework 6

This project implements command-line parsing and file-based search/replace behavior in C.

The main solution is implemented in `hw6.c` and supports:

- Required options: `-s` and `-r`
- Optional options: `-l` and `-w`
- Input/output file processing
- Simple replacement mode
- Wildcard replacement mode
- Error handling with required priority and return codes from `hw6.h`

## Files Included

- `hw6.c`: Main implementation to be graded
- `hw6.h`: Assignment constants and error-code macros
- `student_tests.c`: Additional student-written Criterion tests
- `unit_tests.c`, `unit_tests.h`: Provided test harness (not modified)
- `Makefile`: Build and test commands
- `README.md`: This file

## How to Build

Build binaries with:

```bash
make all
```

Or compile directly:

```bash
gcc -std=gnu11 -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -pedantic -g hw6.c -o hw6
```

## How to Run

General usage:

```bash
./hw6 -s <search_text> -r <replace_text> [-l start,end] [-w] <infile> <outfile>
```

Example:

```bash
./hw6 -s hello -r world -l 4,10 input.txt output.txt
```

## Testing

Run provided tests:

```bash
make test
```

Run student tests (if compiled separately):

```bash
gcc -std=gnu11 -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wundef -pedantic -g student_tests.c -lcriterion -o student_tests
./student_tests -j1
```

## Submission Notes

- Ensure your name, SBU username, and SBUID are filled in this file.
- Ensure final zip filename follows the required format:

```text
SPR26CSE220-<SBUusername>-<SBUID#>.zip
```

- Include required source files and Makefile.
- Do not include generated binaries or temporary test output folders.
I also made some test cases in student_tests.c
