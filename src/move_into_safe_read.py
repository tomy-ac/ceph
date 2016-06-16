#!env python

import sys

lines_in_file = [line.rstrip('\n') for line in sys.stdin.readlines()]

to_cut = []
cutting = False

def deindent(line):
    count = 0
    while len(line):
        if line[0] == ' ':
            count += 1
            line = line[1:]
        elif line[0] == '\t':
            count += 8
            line = line[1:]
        else:
            break
    if count >= 2:
        count -= 2
    line = (' '*(count % 8)) + line
    line = ('\t'*(count / 8)) + line
    return line.rstrip()

for line in lines_in_file:
    if line == '----->':
        assert not cutting
        cutting = True
    elif line == '-----<':
        assert cutting
        cutting = False
    elif cutting:
        to_cut.append(line)

assert not cutting
for line in lines_in_file:
    if line == '----->':
        assert not cutting
        cutting = True
    elif line == '-----<':
        assert cutting
        cutting = False
    elif line == '+++++':
        assert not cutting
        for l in to_cut:
            print deindent(l)
    elif not cutting:
        print line
