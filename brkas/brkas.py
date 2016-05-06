#!/usr/bin/env python3

'''
    Copyright 2016 Leonard Göhrs

    This file is part of BrkAs.

    BrkAs is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BrkAs is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BrkAs.  If not, see <http://www.gnu.org/licenses/>.
'''

# This software started out to be a clean and modular assembler
# but ended up being quite the opposite
# (a unclean and hackish dissassembler).
# Some work would be required to get it into a nice shape,
# work like making this not the only comment in the whole sourcecode.
# But the assembler and disassembler work for now and features can
# still be added and removed without digging in to much crud
# so i will postpone those cleanups to 'later'.

import sys
from decoders import DecodeException, EncodeException, decoders


class DecodeExeCollection(Exception):
    numerrors= 5

    def __init__(self, line='', linenum=None):
        self.line= line
        self.linenum= linenum
        self.errors=[]

    def append(self, exception):
        self.errors.append(exception)

    def __str__(self):
        errs= 'Error parsing line {}\n'.format(self.linenum)
        errs+= ' {:4} | {}\n'.format(self.linenum, self.line)
        errs+='See below for the top {} '.format(self.numerrors)
        errs+='errors the separate parsing modules produced\n'

        errs+= '\n'.join('{:3} | {}'.format(e[0]+1,e[1])
                         for e in enumerate(sorted(self.errors)[:self.numerrors]))
        errs+= '\n...'

        return (errs)

class Program(object):
    def __init__(self):
        self.labels={}
        self.ops=[]

    def _lines_from_text(self, text):
        lines= text.split('\n')

        def rm_comment(ln):
            sep= ln.find('//')

            if sep >= 0:
                ln= ln[:sep]

            return (ln.strip())

        def not_empty(ln):
            return (ln if ln != '' else False)

        lines= list(map(rm_comment, lines))
        lines= list(filter(not_empty, lines))

        return (lines)

    def _decode_textline(self, line):
        linenum, ln= line

        exe= DecodeExeCollection(ln, linenum)

        for dc in decoders:
            try:
                return (dc(self, ln))
            except DecodeException as e:
                exe.append(e)

        raise exe

    def _decode_bytecode(self, op):
        addr, inst= op

        exe= DecodeExeCollection(hex(inst), addr)

        for dc in decoders:
            try:
                return (dc(self, inst))
            except DecodeException as e:
                exe.append(e)

        raise exe

    def from_text(self, text):
        lines= self._lines_from_text(text)

        self.ops.extend(map(self._decode_textline, enumerate(lines)))

        offset=0
        for op in self.ops:
            op.place(offset)
            offset+=len(op)

    def from_textfile(self, path):
        fd= open(path, encoding='utf-8')
        text=fd.read()
        fd.close()

        self.from_text(text)

    def from_bytecode(self, bc):
        self.ops.extend(self._decode_bytecode((b[0], bytes([b[1]])))
                        for b in  enumerate(bc))

    def from_bytecodefile(self, path):
        fd= open(path, 'rb')
        bytecode=fd.read()
        fd.close()

        self.from_bytecode(bytecode)

    def to_text(self):
        return ('\n'.join(map(str, self.ops)) + '\n')

    def to_textfile(self, path):
        fd= open(path, 'w', encoding='utf-8')
        fd.write(self.to_text())
        fd.close()

    def to_bytecode(self):
        bc= bytes()
        for o in map(bytes, self.ops):
            bc+=o

        return (bc)

    def to_bytecodefile(self, path):
        fd= open(path, 'wb')
        fd.write(self.to_bytecode())
        fd.close()

    @classmethod
    def to_specification(cls):
        opal= filter(lambda o: 'opcode' in o.__dict__, decoders)
        ops= filter(lambda o: 'alfrom' not in o.__dict__, opal)
        aliases= filter(lambda o: 'alfrom' in o.__dict__, opal)

        ops= sorted(ops, key= lambda a: a.opcode)

        spec= ops[0].specfmt.format('Instr bits', 'Name', 'Description') + '\n'
        spec+= '\n'.join(map(lambda o: o.specification_line(), ops))
        spec+= '\n'

        return (spec)

    @classmethod
    def to_specificationfile(cls, path):
        fd= open(path, 'w', encoding='utf-8')
        fd.write(cls.to_specification())
        fd.close()

def main(args):
    if (len(args) == 0 or len(args) % 2 != 0):
        lines= []
        lines.append('Brkas expects parameters as command filename pairs.')
        lines.append('Available commands are:')
        lines.append(' source_in sourcefile.brkas         - Assembly source file')
        lines.append(' bytecode_in bytecode.bc            - Bytecode output file')
        lines.append(' source_out sourcefile.brkas        - Disassembly file')
        lines.append(' bytecode_out bytecode.bc           - Assembly file')
        lines.append(' specification_out specification.md - Bytecode specification')
        lines.append('')
        lines.append('example:')
        lines.append(' brkas source_in tst.brkas bytecode_out tst.bc')

        sys.stderr.write('\n'.join(lines) + '\n')

        return

    parms= dict(zip(args[0::2],args[1::2]))
    prog= Program()

    try:
        if ('source_in' in parms):
            prog.from_textfile(parms['source_in'])

        if ('bytecode_in' in parms):
            prog.from_bytecodefile(parms['bytecode_in'])

        if ('source_out' in parms):
            prog.to_textfile(parms['source_out'])

        if ('bytecode_out' in parms):
            prog.to_bytecodefile(parms['bytecode_out'])

        if ('specification_out' in parms):
            prog.to_specificationfile(parms['specification_out'])

    except DecodeExeCollection as e:
        sys.stderr.write(str(e) + '\n')

if __name__ == '__main__':
    main(sys.argv[1:])
