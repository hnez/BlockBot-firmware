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
        elif name.isdecimal() and int(name) in self.numtoreg:
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
        self.label= label

    def __int__(self):
        command= self.command
        labels= command.program.labels

        if self.label not in labels:
            text= 'label {} could not be found.'.format(self.jump)
            text+= 'Make sure you typed it correctly and in the right case.'
            raise EncodeException('high', text)

        label= labels[self.label]

        direction, length= label.get_offset(command.position)

        if (direction != command.direction):
            dmap= {'pos': 'is not behind the current position. Use JBW instead',
                   'neg': 'is not before the current position. Use JFW instead'}

            text= '{} selected but {} {}'.format(self.mnemonic, self.jump, dmap[self.direction])
            raise EncodeException('high', text)

        if (length > self.maximum_jumplength):
            text= 'Label {} is {} bytes away but jumps can only span {} bytes.'
            text+= 'Consider putting a intermediate jumppoint in the middle of the spanned codeblock.'
            text= text.format(self.jump, length, self.maximum_jumplength)
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
        elif name.isdecimal() and int(name) in self.numtoname:
            self.regnum= int(name)
        else:
            errtxt= 'Expected one of "{}" '.format(', '.join(self.names))
            errtxt+= 'as memory slot name. Got "{}"'.format(name)
            raise(DecodeException('medium', errtxt))

    def __int__(self):
        return self.memnum

    def __str__(self):
        return '"{}"'.format(self.numtoname[self.memnum])


class ProtoBase(object):
    def __len__(self):
        return 1

    def place(self, position):
        self.position= position

class ProtoCmd (ProtoBase):
    specfmt= '{: ^13}| {:11}| {}'
    disasmfmt= '    {:16} // {}'
    parmmap= [('XX', 'reg1', Register),
              ('YY', 'reg2', Register),
              ('NNN', 'mem', MemAddr),
              ('NNNN', 'jump', Jump)]
    prototoname= dict((a[0], a[1]) for a in parmmap)

    def match_prototype(self, line):
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

    def __init__(self, program, line):
        if isinstance(line, int):
            if (line & self.opcodemask() == self.opcode):
                self.instruction= line
                return

        if isinstance(line, str):
            parms= self.match_prototype(line)

            self.program= program
            self.args= dict(
                (
                    c[1],
                    c[2](self, parms[c[0]])
                ) for c in self.parmmap if c[0] in parms
            )

    def cmdtype(self):
        pt= self.prototype.split(' ')

        if len(pt) == 1:
            return 'NoArgs'

        if len(pt) == 2:
            if pt[1] == 'XX':
                return 'SingleReg'

            if pt[1] == 'NNNN':
                return 'Jump'

        if len(pt) == 3:
            if pt[1] == 'XX' and pt[2] == 'YY':
                return 'TwoRegs'

            if pt[1] == 'NNN' and pt[2] == 'XX':
                return 'RegMem'

        text= 'Prototype "{}" has unkown type'.format(self.prototype)
        raise DecodeException('high', text)

    def opcodemask(self):
        masks= {'NoArgs': 0xff,
                'SingleReg': 0xfc,
                'Jump': 0xf0,
                'TwoRegs': 0xf0,
                'RegMem': 0xe0}

        return masks[self.cmdtype]

    @classmethod
    def specification_line(cls):
        encoding= list('{:08b}'.format(cls.opcode))

        prototype= getattr(cls, 'alfrom', None) or cls.prototype
        pt= prototype.split(' ')

        if ('XX' in pt):
            encoding[6:8]=list('XX')

        if ('YY' in pt):
            encoding[4:6]=list('YY')

        if ('NNN' in pt):
            encoding[3:6]=list('NNN')

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
    description= 'Load io address NNN into register XX'
    opcode=0b1100000
    prototype= 'LDA NNN XX'

class CmdSTA (ProtoCmd):
    description= 'Store Register XX into io address NNN'
    opcode=0b1110000
    prototype= 'STA NNN XX'

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

class CmdConstant (ProtoConstant):
    def parse(self, line):
        try:
            return int(line, base=0)
        except ValueError:
            errtxt= '{} is not a number'.format(line)
            raise DecodeException('low', errtxt)
