#!/usr/bin/env python3

from decoders import DecodeException, decoders

class DecodeExeCollection(Exception):
    def __init__(self, linenum=None):
        self.linenum= linenum
        self.errors=[]

    def append(self, exception):
        self.errors.append(exception)

    def __str__(self):
        errs= 'Error parsing line {} '.format(self.linenum)
        errs+='see below for errors the separate parsing modules produced\n'

        errs+= '\n'.join(str(e) for e in sorted(self.errors))

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

        exe= DecodeExeCollection(linenum)

        for dc in decoders:
            try:
                return (dc(self, ln))
            except DecodeException as e:
                exe.append(e)

        raise exe

    def _decode_bytecode(self, op):
        addr, inst= op

        exe= DecodeExeCollection(addr)

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


    @classmethod
    def to_specification(cls):
        ops= filter(lambda o: getattr(o, 'opcode', False), decoders)
        ops= sorted(ops, key= lambda a: a.opcode)

        spec= ops[0].specfmt.format('Instr bits', 'Name', 'Description') + '\n'
        spec+= '\n'.join(map(lambda o: o.specification_line(), ops))

        return (spec)

    def to_text(self):
        return ('\n'.join(map(str, self.ops)))

    def to_bytecode(self):
        bc= bytes()
        for o in map(bytes, self.ops):
            bc+=o

        return (bc)

p=Program()
p.from_textfile('tst.brkas')
u=Program()
