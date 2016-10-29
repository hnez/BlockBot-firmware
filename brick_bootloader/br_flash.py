#!/usr/bin/env python3

'''
    Copyright 2016 Leonard Göhrs

    This file is part of Chain flasher.

    Chain flasher is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Chain flasher is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Chain flasher.  If not, see <http://www.gnu.org/licenses/>.
'''

import serial
import struct
import time
import itertools

class AvrMcu(object):
    devices= {
        '1e930b' : {'name' : 'ATtiny85', 'page_size' : 64, 'flash_size' : 8192},
        '1e9206' : {'name' : 'ATtiny45', 'page_size' : 64, 'flash_size' : 4096},
        '1e9108' : {'name' : 'ATtiny25', 'page_size' : 32, 'flash_size' : 2048}
    }

    def __init__(self, descriptor):
        bin_sig= (descriptor[8]<<16) | (descriptor[9]<<8) | (descriptor[10])
        self.signature= '{:06x}'.format(bin_sig)

        self.bootloader_start= descriptor[16] | (descriptor[17]<<8)

    def __getitem__(self, k):
        dev_info= self.devices[self.signature]

        if k in dev_info:
            return dev_info[k]

        if k in ('signature', 'bootloader_start'):
            return (getattr(self, k))

    def __str__(self):
        dev_info= self.devices[self.signature]

        text= '{} with bootloader start address {:x}'

        return (text.format(dev_info['name'], self.bootloader_start))

    def encode_rjmp(self, pos, dst):
        rel_addr= dst-pos-1

        if rel_addr>=0x0800:
            return (self.encode_rjmp(pos, dst-0x1000))

        addr_compl= rel_addr if (rel_addr >= 0) else (0x1000 + rel_addr)

        instr= 0xc000 | addr_compl

        return (instr)

    def decode_rjmp(self, pos, instr):
        addr_compl= instr & 0x0fff

        addr= ('TODO') if (addr_compl & 0x0800) else (addr_compl + 1)

        return(pos+addr)

    def relocate_rjmp(self, oldpos, newpos, instr):
        target= self.decode_rjmp(oldpos, instr)

        return(self.encode_rjmp(newpos, target))

class Brick(object):
    def __init__(self, flasher, addr):
        self.flasher= flasher
        self.addr= addr

        self.mcu= self.get_devinfo()

        print('Enumerated device {}: {}'.format(addr, self.mcu))

    def get_devinfo(self):
        ret= self.flasher.xfer_pkg(self.addr, self.flasher.OP_DEVINFO)

        (op, addr, page_num, data) = ret

        return (AvrMcu(data))

    def read_page(self, page_num):
        ret= self.flasher.xfer_pkg(self.addr, self.flasher.OP_READ, page_num)

        (op, addr, page_num, data) = ret

        return (data)

    def write_page(self, page_num, data):
        if page_num==0:
            data= bytearray(data)

            bl_addr= self.mcu['bootloader_start']

            rs_vect_pos= (1 - 1)
            wd_vect_pos= (13 - 1)

            # Move the original reset vector to the watchdog vector
            # Place jump to bootloader into reset vector
            watchdog_vect= self.mcu.relocate_rjmp(rs_vect_pos, wd_vect_pos, data[0] | (data[1]<<8))
            reset_vect= self.mcu.encode_rjmp(rs_vect_pos, bl_addr)

            data[rs_vect_pos*2]= reset_vect & 0xff
            data[rs_vect_pos*2+1]= reset_vect >> 8

            data[wd_vect_pos*2]= watchdog_vect & 0xff
            data[wd_vect_pos*2+1]= watchdog_vect >> 8

        ret= self.flasher.xfer_pkg(self.addr, self.flasher.OP_WRITE, page_num, data)

    def dump_binary(self, fname):
        fs= self.mcu['flash_size']
        ps= self.mcu['page_size']

        with open(fname, 'wb') as fd:
            print('Dev {} dumping:'.format(self.addr))

            for page_num in range(fs//ps):
                page= self.read_page(page_num)

                fd.write(page)

                sys.stdout.write('#')
                sys.stdout.flush()

            print('\nDone')

    def flash_binary(self, fname):
        fs= self.mcu['flash_size']
        bs= self.mcu['bootloader_start']
        ps= self.mcu['page_size']

        with open(fname, 'rb') as fd:
            print('Dev {} flashing:'.format(self.addr))

            for page_num in range(bs//ps):
                page= fd.read(ps)

                if not page:
                    break

                page+= bytes(0xff for i in range(len(page)))

                self.write_page(page_num, page)

                sys.stdout.write('#')
                sys.stdout.flush()

            print('\nDone')

class Flasher(object):
    OP_NOP= 0
    OP_ENUMERATE= 1
    OP_DEVINFO= 2
    OP_WRITE= 3
    OP_READ= 4

    msg_fmt= struct.Struct('<BBH64s')

    def __init__(self, port):
        self.port= port
        self.ser= serial.Serial(port, 9600)

        self.reset()

        self.ser.reset_input_buffer()

        self.bricks= list()

        chain_len= self.enum_chain()

        for addr in range(1, chain_len+1):
            self.bricks.append(Brick(self, addr))

    def reset(self):
        self.ser.dtr= True

        time.sleep(5)

        self.ser.dtr= False

        time.sleep(0.5)

    def xfer_pkg(self, addr, op, page_num=0, data=None):
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

def main(args):
    ai= iter(args)

    flash_target= None
    dump_target= None

    for arg in ai:
        if arg == '-d':
            dev= next(ai)

        if arg == 'flash':
            flash_target= list(ai)

        if arg == 'dump':
            dump_target= list(ai)

        if arg== 'flash_all':
            flash_target= itertools.repeat(next(ai))

    fl= Flasher(dev)

    if flash_target:
        for (brick, binary) in zip(fl.bricks, flash_target):
            brick.flash_binary(binary)

    if dump_target:
        for (brick, target) in zip(fl.bricks, dump_target):
            brick.dump_binary(target)


if __name__ == '__main__':
    import sys

    main(sys.argv)
