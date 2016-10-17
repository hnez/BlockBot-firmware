#!/usr/bin/env python3

import serial
import struct

class Brick(object):
    def __init__(self, flasher, addr):
        self.flasher= flasher
        self.addr= addr

    def read_page(self, page_num):
        ret= self.flasher.xfer_pkg(self.addr, self.flasher.OP_READ, page_num, None)

        (op, addr, page_num, data) = ret

        return (data)

    def write_page(self, page_num, data):
        ret= self.flasher.xfer_pkg(self.addr, self.flasher.OP_WRITE, page_num, data)

    def dump(self):
        flash= bytes()

        for pagenr in range(8192//64):
            flash+= self.read_page(pagenr)

        return(flash)

class Flasher(object):
    OP_NOP= 0
    OP_ENUMERATE= 1
    OP_WRITE= 2
    OP_READ= 3
    OP_RESTART= 4

    msg_fmt= struct.Struct('<BBH64s')

    def __init__(self, port):
        self.port= port
        self.ser= serial.Serial(port, 9600)
        self.ser.reset_input_buffer()

        self.bricks= list()

        chain_len= self.enum_chain()

        print ('Enumerated {} Bricks'.format(chain_len))

        for addr in range(1, chain_len+1):
            self.bricks.append(Brick(self, addr))

    def xfer_pkg(self, addr, op, page_num, data):
        if data is None:
            data=bytes(64)

        packet= self.msg_fmt.pack(addr, op, page_num, data)

        self.ser.write(packet)

        ans_b= self.ser.read(self.msg_fmt.size)
        ans_t= self.msg_fmt.unpack(ans_b)

        if (ans_t[0]!=addr or ans_t[1]!=op or ans_t[2]!=page_num):
            raise(Exception('Header mangled in transfer {}'.format(ans_t)))

        return(ans_t)

    def enum_chain(self):
        (addr, op, page_num, data)= self.xfer_pkg(0, self.OP_ENUMERATE, 0, None)

        return (data[0])
