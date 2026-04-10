This homework is a command-line text replacement program. I built it so it
reads an input file, looks for the text you give with `-s`, and writes the
result into an output file using `-r`. It also supports `-l` for choosing a
line range, so the replacement only happens between the start and end lines
you specify. Outside that range, the file is copied exactly as it is.

I also added support for wildcard searching with `-w`, which lets the search
text work as a prefix or suffix pattern when it contains one `*` at either the
beginning or the end. Along with the main replacement logic, I made sure the
program checks for missing arguments, invalid line ranges, duplicate options,
and invalid wildcard patterns, so it behaves more safely and gives the
expected exit codes when something is wrong. The tests in the project helped
confirm that the simple search cases and wildcard cases both work the way they
should.