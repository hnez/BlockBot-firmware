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

class DEException(Exception):
    def __init__(self, quality, text):
        super(DEException, self).__init__('({})'.format(quality), text)

        self.quality= quality
        self.text= text

    def __lt__(self, other):
        qualities={'low': 1, 'medium' : 2, 'high' : 3}

        return (qualities[self.quality] > qualities[other.quality])

    def __str__(self):
        return ('({}) {}'.format(self.quality, self.text))

class DecodeException(DEException):
    pass

class EncodeException(DEException):
    pass

class Register(object):
    names='RZ RA RB RC'.split()
    numtoreg= dict(enumerate(names))
    regtonum= dict((a[1], a[0]) for a in numtoreg.items())

    def __init__ (self, command, name):
        if name in self.regtonum:
            self.regnum= self.regtonum[name]
        elif (isinstance(name, int) or name.isdecimal()) and int(name) in self.numtoreg:
            self.regnum= int(name)
        else:
            errtxt= 'Expected one of "{}" '.format(', '.join(self.names))
            errtxt+= 'as register name. Got "{}"'.format(name)
            raise(DecodeException('medium', errtxt))

    def __int__(self):
        return self.regnum

    def __str__(self):
        return self.numtoreg[self.regnum]

class Jump(object):
    maximum_jumplength= 16

    def __init__(self, command, label):
        self.command= command
        self.label= label if not isinstance(label, int) else 'anon' +str(label)

    def __int__(self):
        command= self.command
        labels= command.program.labels
        pt= self.command.prototype.split()

        if self.label not in labels:
            text= 'label {} could not be found.'.format(self.jump)
            text+= 'Make sure you typed it correctly and in the right case.'
            raise EncodeException('high', text)

        label= labels[self.label]

        direction, length= label.get_offset(command.position)

        if (direction != command.direction):
            dmap= {'pos': 'is not behind the current position. Use JBW instead',
                   'neg': 'is not before the current position. Use JFW instead'}

            text= '{} selected but {} {}'.format(pt[0], self.jump, dmap[self.direction])
            raise EncodeException('high', text)

        if (length > self.maximum_jumplength):
            text= 'Label {} is {} bytes away but jumps can only span {} bytes.'
            text+= 'Consider putting an intermediate jumppoint in the middle of the spanned codeblock.\n'
            text+= '  SKP      // Allways skip next instruction in linear execution\n'
            text+= 'impoint:   // Jump to this label instead\n'
            text+= '  {} {} // Perform desired jump\n'
            text= text.format(self.label, length, self.maximum_jumplength, pt[0], self.label)
            raise EncodeException('high', text)

        return (length - 1)

    def __str__(self):
        return (self.label)

class MemAddr(object):
    names= 'Mot1 Mot2 DIn DOut Timer Ram1 Ram2 Ram3'.split()
    numtoname= dict(enumerate(names))
    nametonum= dict((a[1], a[0]) for a in numtoname.items())

    def __init__ (self, command, name):
        if name in self.nametonum:
            self.memnum= self.nametonum[name]
        elif (isinstance(name, int) or name.isdecimal()) and int(name) in self.numtoname:
            self.memnum= int(name)
        else:
            errtxt= 'Expected one of "{}" '.format(', '.join(self.names))
            errtxt+= 'as memory slot name. Got "{}"'.format(name)
            raise(DecodeException('medium', errtxt))

    def __int__(self):
        return self.memnum

    def __str__(self):
        return str(self.numtoname[self.memnum])


class ProtoBase(object):
    def __len__(self):
        return 1

    def place(self, position):
        self.position= position

class ProtoDirective(ProtoBase):
    def match_name(self, line):
        name = line.split(' ')[0][1:]

        if (self.name != name):
            raise DecodeException('low', '{} does not match {}'.format(self.name, name))

    def __init__(self, program, line):
        self.program= program

        if isinstance(line, bytes):
            raise DecodeException('low', 'Directives are not present in bytecode')

        if isinstance(line, str):
            self.match_name(line)

            self.parse_parameters(line.split(' ')[1:])

            return

        raise Exception('Incompatible input type')

class DirectiveBrickName (ProtoDirective):
    name= 'brkname'

    def parse_parameters(self, parms):
        self.program.meta['Name']= ' '.join(parms)

    def __len__(self):
        return 0

    def __bytes__(self):
        return (bytes([]))

    def __str__(self):
        return ('.brkname' + self.program.meta['Name'])


class DirectiveParameter (ProtoDirective):
    name= 'brkparm'

    def parse_parameters(self, parms):
        if len(parms) != 2:
            text= '{} takes two parameters (parm_num default) but received {}'

            DecodeException('high', text.format(self.name, len(parms)))

        try:
            self.parm_num, self.default= tuple(map(int, parms))

        except ValueError:
            text= '{} takes two integer parameters (parm_num default)'

            DecodeException('high', text.format(self.name))

        if 'Parms' not in self.program.meta:
            self.program.meta['Parms']= dict()

        if self.parm_num not in self.program.meta['Parms']:
            self.program.meta['Parms'][self.parm_num]= list()

        self.program.meta['Parms'][self.parm_num].append(self)

    def __len__(self):
        return 0

    def __bytes__(self):
        return (bytes([self.default]))

    def __str__(self):
        return ('.{} {} {}',  self.name, self.parm_num, self.default)

class ProtoCmd (ProtoBase):
    specfmt= '{: ^13}| {:11}| {}'
    disasmfmt= '    {:16} // {}'
    parmmap= [('XX', 'reg1', Register),
              ('YY', 'reg2', Register),
              ('AAA', 'mem', MemAddr),
              ('NNNN', 'jump', Jump)]
    prototoname= dict((a[0], a[1]) for a in parmmap)

    def text_match_prototype(self, line):
        sln= line.split(' ')
        pt= self.prototype.split(' ')

        if (sln[0].upper() != pt[0]):
            text= '{} does not match {}'.format(sln[0], pt[0])
            raise DecodeException('low', text)

        if (sln[0] != pt[0]):
            text= '{} does not match {}. Please note, that brkas is case sensitve.'
            text+= 'use {} instead of {}'
            text=text.format(sln[0], pt[0], sln[0].upper(), sln[0])
            raise DecodeException('high', text)

        if len(sln) != len(pt):
            text= 'You supplied {} arguments but {} expects {} ({})'
            text= text.format(len(sln)-1, pt[0] ,len(pt)-1, self.prototype)

            raise DecodeException('high', text)

        parms= dict((tk, sln[pos]) for (pos, tk) in enumerate(pt))

        return (parms)

    def bytecode_match_prototype(self, bc):
        pt= self.prototype.split(' ')
        parms={}

        if (bc & self.opcodemask() != self.opcode):
            text='{}&{} does not match{}'.format(bc, self.opcodemask(), self.opcode)
            raise DecodeException('low', text)

        if ('YY' in pt):
            parms['reg1']= Register(self, (bc & 0x0c) >> 2)
            parms['reg2']= Register(self, bc & 0x03)
        elif ('XX' in pt):
            parms['reg1']= Register(self, bc & 0x03)

        if ('AAA' in pt):
            parms['mem']= MemAddr(self, (bc & 0x1c) >> 2)

        if ('NNNN' in pt):
            parms['jump']= Jump(self, bc & 0x0f)

        return parms

    def __init__(self, program, line):
        self.program= program

        if isinstance(line, bytes):
            parms= self.bytecode_match_prototype(line[0])

            self.args= parms

            return

        if isinstance(line, str):
            parms= self.text_match_prototype(line)

            self.args= dict(
                (
                    c[1],
                    c[2](self, parms[c[0]])
                ) for c in self.parmmap if c[0] in parms
            )

            return

        raise Exception('Incompatible input type')

    def opcodemask(self):
        pt= self.prototype.split()

        argbits= len(''.join(pt[1:]))

        mask= 0xff^((1 << argbits)-1)

        return (mask)

    @classmethod
    def specification_line(cls):
        encoding= list('{:08b}'.format(cls.opcode))

        prototype= getattr(cls, 'alfrom', None) or cls.prototype
        pt= prototype.split(' ')

        if ('XX' in pt):
            encoding[6:8]=list('XX')

        if ('YY' in pt):
            encoding[4:6]=list('YY')

        if ('AAA' in pt):
            encoding[3:6]=list('AAA')

        if ('NNNN' in pt):
            encoding[4:6]=list('NNNN')

        estr= ' '.join(encoding[2*a] + encoding[2*a+1]
                       for a in range(4))

        padproto= ' '.join(s.ljust(3) for s in pt)

        spec= cls.specfmt.format(estr, padproto, cls.description)

        return (spec)

    def __bytes__(self):
        instr= self.opcode

        if 'reg2' in self.args:
            instr|= int(self.args['reg2']) | (int(self.args['reg1']) << 2)
        elif 'reg1' in self.args:
            instr|= int(self.args['reg1'])

        if 'mem' in self.args:
            instr|= int(self.args['reg1']) | (int(self.args['mem']) << 2)

        if 'jump' in self.args:
            instr |= int(self.args['jump'])

        return (bytes([instr]))

    def __str__(self):
        pt= self.prototype.split()
        ds= self.description.split()

        args= self.args
        prne= self.prototoname

        spt= list(str(args[prne[p]]) if p in prne else p
                      for p in pt)

        sds= list(str(args[prne[d]]) if d in prne else d
                      for d in ds)

        text= self.disasmfmt.format(' '.join(spt),
                                    ' '.join(sds))

        return (text)


class ProtoConstant(ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, bytes):
            self.value= line[0]
            return

        if isinstance(line, str):
            self.value= self.parse(line)

    def __bytes__(self):
        return (bytes([self.value]))

    def __str__(self):
        return ('    ' + hex(self.value))


class ProtoAlias(ProtoBase):
    def __init__ (self, program, line):
        if not isinstance(line, str):
            raise DecodeException('low', 'Aliases only work on text')

        sln= line.split()
        frm= self.alfrom.split(' ')
        to= self.alto.split(' ')

        if (frm[0] != sln[0]):
            text= '{} is not an alias for {}'.format(sln[0], frm[0])
            raise DecodeException('low', text)

        if len(frm) != len(sln):
            text= 'Alias {} expects {} arguments but got {}'
            text= text.format(frm[0], len(frm), len(sln))

            raise DecodeException('medium', text)

        parms= {}
        for (pos, tk) in enumerate(frm):
            parms[tk] = sln[pos]

        cmd= ' '.join(parms[tk] if tk in parms else tk for tk in to)

        super(ProtoAlias, self).__init__(program, cmd)

class CmdLabel (ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, str):
            if line.endswith(':'):
                self.label= line.split(':')[0]
                self.program= program
                program.labels[self.label]=self

                return

        raise DecodeException('low', 'line is not a label')

    def get_offset(self, pos):
        diff= self.position -pos

        return (('neg', -diff) if diff < 0 else ('pos', diff))

    def __len__(self):
        return 0

    def __bytes__(self):
        return bytes([])

    def __str__(self):
        return self.label + ':'


class CmdSOV (ProtoCmd):
    description= 'Skip if last arithmetic instruction generated an over- or underflow'
    opcode= 0b00000100
    prototype= 'SOV'

class CmdSPU (ProtoCmd):
    description= 'Push current PC and RC to stack'
    opcode=0b000000101
    prototype= 'SPU'

class CmdSPO (ProtoCmd):
    description= 'Pop RC and fill PC buffer from stack'
    opcode=0b000000110
    prototype= 'SPO'

class CmdSPJ (ProtoCmd):
    description= 'Perform the jump in the PC buffer'
    opcode=0b000000111
    prototype= 'SPJ'

class CmdLD (ProtoCmd):
    description= 'Load next Byte into register XX and skip execution over it'
    opcode=0b00001000
    prototype= 'LD XX'

class CmdDEC (ProtoCmd):
    description= 'Decrement register XX by one'
    opcode=0b00010000
    prototype= 'DEC XX'

class CmdINC (ProtoCmd):
    description= 'Increment register XX by one'
    opcode=0b00010100
    prototype= 'INC XX'

class CmdNOT (ProtoCmd):
    description= 'Bitwise NOT of Register XX'
    opcode=0b00011000
    prototype= 'NOT XX'

class CmdSRR (ProtoCmd):
    description= 'Shift register right by one bit'
    opcode=0b00011100
    prototype= 'SRR XX'

class CmdJFW (ProtoCmd):
    description= 'Jump forward to label NNNN'
    opcode=0b00100000
    prototype= 'JFW NNNN'
    direction= 'pos'

class CmdJBW (ProtoCmd):
    description= 'Jump backwards to label NNNN'
    opcode=0b00110000
    prototype= 'JBW NNNN'
    direction= 'neg'

class CmdMOV (ProtoCmd):
    description= 'Copy register content of YY to XX'
    opcode=0b01000000
    prototype= 'MOV XX YY'

class CmdOR (ProtoCmd):
    description= 'Store logic or of XX and YY in XX'
    opcode=0b01010000
    prototype= 'OR XX YY'

class CmdAND (ProtoCmd):
    description= 'Store logic and of XX and YY in XX'
    opcode=0b01100000
    prototype= 'AND XX YY'

class CmdXOR (ProtoCmd):
    description= 'Store logic exclusive or of XX and YY in XX'
    opcode=0b01110000
    prototype= 'XOR XX YY'

class CmdADD (ProtoCmd):
    description= 'Store sum of XX and YY in XX'
    opcode=0b10000000
    prototype= 'ADD XX YY'

class CmdSUB (ProtoCmd):
    description= 'Store difference of XX and YY in XX'
    opcode=0b10010000
    prototype= 'SUB XX YY'

class CmdSEQ (ProtoCmd):
    description= 'Skip next instruction if equal'
    opcode=0b10100000
    prototype= 'SEQ XX YY'

class CmdSNE (ProtoCmd):
    description= 'Skip next instruction if not equal'
    opcode=0b10110000
    prototype= 'SNE XX YY'

class CmdLDA (ProtoCmd):
    description= 'Load io address AAA into register XX'
    opcode=0b11000000
    prototype= 'LDA AAA XX'

class CmdSTA (ProtoCmd):
    description= 'Store Register XX into io address AAA'
    opcode=0b11100000
    prototype= 'STA AAA XX'

class AliasNEG (ProtoAlias, CmdSUB):
    description= 'Gives the 2-complement of a number'
    alfrom= 'NEG XX'
    alto= 'SUB RZ XX'

class AliasNOP (ProtoAlias, CmdSNE):
    description= 'No operation'
    alfrom='NOP'
    alto='SNE RZ RZ'

class AliasSRL (ProtoAlias, CmdADD):
    description= 'Shift register left by one bit'
    alfrom= 'SRL XX'
    alto= 'ADD XX XX'

class AliasSKP (ProtoAlias, CmdSEQ):
    description= 'Allways skip next instruction'
    alfrom= 'SKP'
    alto= 'SEQ RZ RZ'


class ConstNumber (ProtoConstant):
    def parse(self, line):
        try:
            return int(line, base=0)
        except ValueError:
            errtxt= '{} is not a number'.format(line)
            raise DecodeException('low', errtxt)

def list_decoders():
    decoders=[]
    globpairs= globals().items()

    for pr in ['Directive', 'Cmd', 'Alias', 'Const']:
        decoders.extend(d[1] for d in globpairs
                        if d[0].startswith(pr))

    return decoders

decoders= list_decoders()
