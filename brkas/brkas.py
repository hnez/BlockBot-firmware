#!/usr/bin/env python3

from exception import Exception

class DecodeException(Exception):
    def __init__(self, quality, text):
        super(DecodeException, self).__init__('({})'.format(quality), text)
        
        self.quality= qualtity
        self.text= text

    def __cmp__(self, other):
        qualities={'low': 1, 'medium' : 2, 'high' : 3}

        return (qualities[self.quality] - qualities[other.quality])

class CmdNoOperand(object):
    def __init__(self, line):
        if line is bytes:
            if line == self.opcode:
                return

        if line is str:
            if line == self.mnemonic:
                return

        raise (DecodeException('low',
                               'Does not match {}'.format(self.mnemonic)))

    def place(self, position):
        pass

    def __len__(self):
        return 1

    def __bytes__(self):
        return self.opcode

    def __str__(self):
        return self.mnemonic

    
