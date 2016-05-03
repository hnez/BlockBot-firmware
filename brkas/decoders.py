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

    def __init__ (self, name):
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

class MemAddr(object):
    names= 'Mot1 Mot2 DIn DOut Timer Ram1 Ram2 Ram3'.split()
    numtoname= dict(enumerate(names))
    nametonum= dict((a[1], a[0]) for a in numtoname.items())

    def __init__ (self, name):
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
        return self.numtoname[self.memnum]


class ProtoBase(object):
    def __len__(self):
        return 1

    def place(self, position):
        self.position= position

class ProtoNoOperand(ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, bytes):
            if line == self.opcode:
                return

        if isinstance(line, str):
            if line == self.mnemonic:
                return

        raise (DecodeException('low',
                               'Does not match {}'.format(self.mnemonic)))

    def __bytes__(self):
        return bytes(self.opcode)

    def __str__(self):
        return self.mnemonic

class ProtoSingleRegister(ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, bytes):
            if (line[0] & 0xfc) == self.opcode:
                self.inst= line
                return

        if isinstance(line, str):
            sln= line.split(' ')

            if sln[0] == self.mnemonic:
                if (len(sln) != 2):
                    errtxt= '{} expects one register as argument. But got {}'
                    errtext= errtxt.format(self.mnemonic, len(sln))

                    raise (DecodeException('medium', errtxt))

                self.reg= Register(sln[1])

                return

        raise (DecodeException('low',
                               'Does not match {}'.format(self.mnemonic)))

    def __bytes__(self):
        return bytes([self.opcode | int(self.reg)])

    def __str__(self):
        return '{} {}'.format(self.mnemonic, self.reg)

class ProtoJumps(ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, bytes):
            if (line[0] & 0xf0) == self.opcode:
                self.inst= line
                return

        if isinstance(line, str):
            sln= line.split(' ')

            if sln[0] == self.mnemonic:
                if (len(sln) != 2):
                    errtxt= '{} expects one relative jump address. But got {}'
                    errtext= errtxt.format(self.mnemonic, len(sln))

                    raise (DecodeException('medium', errtxt))

                self.program= program
                self.jump= sln[1]
                return

        raise (DecodeException('low',
                               'Does not match {}'.format(self.mnemonic)))
    def __bytes__(self):
        if self.jump not in self.program.labels:
            text= 'label {} could not be found'.format(self.jump)
            raise EncodeException('high', text)

        label= self.program.labels[self.jump]

        direction, length= label.get_offset(self.position)

        if (direction != self.direction):
            dmap= {'pos': 'is not behind the current position',
                   'neg': 'is not before the current position'}

            text= '{} selected but {} {}'.format(self.mnemonic, self.jump, dmap[self.direction])
            raise EncodeException('high', text)

        if (length > 16):
            text= 'label {} is {} bytes away but jumps can only span 16 bytes'
            raise EncodeException('high', text.format(self.jump, length))

        return bytes([self.opcode | (length-1)])

    def __str__(self):
        return '{} {}'.format(self.mnemonic, self.jump)


class ProtoTwoRegisters(ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, bytes):
            if (line[0] & 0xf0) == self.opcode:
                self.inst= line
                return

        if isinstance(line, str):
            sln= line.split(' ')

            if sln[0] == self.mnemonic:
                if (len(sln) != 3):
                    errtxt= '{} expects two registers as arguments. But got {}'
                    errtext= errtxt.format(self.mnemonic, len(sln))

                    raise (DecodeException('medium', errtxt))

                self.regx= Register(sln[1])
                self.regy= Register(sln[2])
                return

        raise (DecodeException('low',
                               'Does not match {}'.format(self.mnemonic)))

    def __bytes__(self):
        return bytes([self.opcode | int(self.regx) << 2 | int(self.regy)])

    def __str__(self):
        return '{} {} {}'.format(self.mnemonic, self.regx, self.regy)


class ProtoMemReg (ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, bytes):
            if (line[0] & 0xe0) == self.opcode:
                self.inst= line
                return

        if isinstance(line, str):
            sln= line.split(' ')

            if sln[0] == self.mnemonic:
                if (len(sln) != 3):
                    errtxt= '{} expects two arguments. But got {}'
                    errtext= errtxt.format(self.mnemonic, len(sln))

                    raise (DecodeException('medium', errtxt))

                self.memaddr= MemAddr(sln[1])
                self.reg= Register(sln[2])
                return

        raise (DecodeException('low',
                               'Does not match {}'.format(self.mnemonic)))

    def __bytes__(self):
        return bytes([self.opcode | int(self.memaddr) << 2 | int(self.reg)])

    def __str__(self):
        return '{} {} {}'.format(self.mnemonic, self.memaddr, self.reg)

class ProtoConstant(ProtoBase):
    def __init__(self, program, line):
        if isinstance(line, bytes):
            self.value= line[0]
            return

        if isinstance(line, str):
            self.value= self.parse(line)

    def __bytes__(self):
        return bytes([self.value])

    def __str__(self):
        return hex(self.value)


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


class CmdSOV (ProtoNoOperand):
    description= 'Skip if last arithmetic instruction generated an over- or underflow'
    opcode= 0b00000100
    mnemonic= 'SOV'

class CmdSPU (ProtoNoOperand):
    description= 'Push current PC and RC to stack'
    opcode=0b000000101
    mnemonic= 'SPU'

class CmdSPO (ProtoNoOperand):
    description= 'Pop RC and fill PC buffer from stack'
    opcode=0b000000110
    mnemonic= 'SPO'

class CmdSPJ (ProtoNoOperand):
    description= 'Perform the jump in the PC buffer'
    opcode=0b000000111
    mnemonic= 'SPJ'

class CmdLD (ProtoSingleRegister):
    description= 'Load next Byte into register XX and skip execution over it'
    opcode=0b00001000
    mnemonic= 'LD'

class CmdDEC (ProtoSingleRegister):
    description= 'Decrement register XX by one'
    opcode=0b00010000
    mnemonic= 'DEC'

class CmdINC (ProtoSingleRegister):
    description= 'Increment register XX by one'
    opcode=0b00010100
    mnemonic= 'INC'

class CmdNOT (ProtoSingleRegister):
    description= 'Bitwise NOT of Register XX'
    opcode=0b00011000
    mnemonic= 'NOT'

class CmdSRR (ProtoSingleRegister):
    description= 'Shift register right by one bit'
    opcode=0b00011100
    mnemonic= 'SRR'

class CmdJFW (ProtoJumps):
    description= 'Jump forward NNNN instructions'
    opcode=0b00100000
    mnemonic= 'JFW'
    direction= 'pos'

class CmdJBW (ProtoJumps):
    description= 'Jump backwards NNNN instructions'
    opcode=0b00110000
    mnemonic= 'JBW'
    direction= 'neg'

class CmdMOV (ProtoTwoRegisters):
    description= 'Copy register content of YY to XX'
    opcode=0b01000000
    mnemonic= 'MOV'

class CmdOR (ProtoTwoRegisters):
    description= 'Store logic or of XX and YY in XX'
    opcode=0b01010000
    mnemonic= 'OR'

class CmdAND (ProtoTwoRegisters):
    description= 'Store logic and of XX and YY in XX'
    opcode=0b01100000
    mnemonic= 'AND'

class CmdXOR (ProtoTwoRegisters):
    description= 'Store logic exclusive or of XX and YY in XX'
    opcode=0b01110000
    mnemonic= 'XOR'

class CmdADD (ProtoTwoRegisters):
    description= 'Store sum of XX and YY in XX'
    opcode=0b10000000
    mnemonic= 'ADD'

class CmdSUB (ProtoTwoRegisters):
    description= 'Store difference of XX and YY in XX'
    opcode=0b10010000
    mnemonic= 'SUB'

class CmdSEQ (ProtoTwoRegisters):
    description= 'Skip next instruction if equal'
    opcode=0b10100000
    mnemonic= 'SEQ'

class CmdSNE (ProtoTwoRegisters):
    description= 'Skip next instruction if not equal'
    opcode=0b10110000
    mnemonic= 'SNE'

class CmdLDA (ProtoMemReg):
    description= 'Load io address NNN into register XX'
    opcode=0b1100000
    mnemonic= 'LDA'

class CmdSTA (ProtoMemReg):
    description= 'Store Register XX into io address NNN'
    opcode=0b1110000
    mnemonic= 'STA'

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
