# -*- coding: utf-8 -*-
import struct, sys #TODO: Change x,y
labels = {}

def parse(text):
  print 'Parsing ...'

  def splitline(line):#-------------------------------------------
    ret = {}

    splt = line.split(':')
    #separate labels, keep rest intact
    if len(splt)==2:
      line=splt[1]
      ret['l']=splt[0].upper()
      labels[splt[0].upper()] = 0 #TODO
    elif len(splt)>2:
      raise SyntaxError('More than one lable in a line')


    splt = line.split(' ')
    splt = [s for s in splt if '' != s] #remove whitespaces between ops, args
    #separate arguments, keep opcode intact
    if len(splt)==1:
      ret['a1']=0 #needed for easy compilation
      ret['a2']=0
    if len(splt)==2:
      line=splt[0]
      ret['a1']=splt[1].upper()
      ret['a2']=0
    elif len(splt)==3:
      line=splt[0]
      ret['a1']=splt[1].upper()
      ret['a2']=splt[2].upper()
    elif len(splt)>3:
      raise SyntaxError('An operation got more than 2 arguments')

    if(line):
      ret['o']=line.upper()
    else:
      ret['o']=0

    return ret
#------------------------------------------------------------------

  lines = text.split('\n') #parse into lines

  lines = [l for l in lines if '' != l] #remove multiple newlines

  lines = [x.lstrip() for x in lines] #strip leading whitespaces

  lines = [x.split('//')[0]  for x in lines] #strip comments

  lines = [x.rstrip() for x in lines] #strip trailing whitespaces

  print lines#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  #split the line syntax ([label:]opcode [argument])
  lines = [splitline(x) for x in lines]

  print lines#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  return lines



def xycompile(ops):
    opcodes = {'NOP':0x00, 'LD':0x04,
               'NOT':0x08,

               'INC':0x10, 'DEC':0x14,

               'JFW':0x20,'JBW':0x30,

               'LDA':0x40,'STA':0x60,

               'MOV': 0x80,'OR' :0x90,
               'AND': 0xA0,'XOR': 0xB0,

               'ADD':0xC0,'SUB':0xD0,
               'SEQ': 0xE0,'SNE':0xF0,
               0:0} #if no opcode

    registers = {'RZ':0x00, 'RA':0x01,
                 'RB':0x02, 'RC':0x03}
    mem_addrs = {'MOT1':0x00, 'MOT2':0x01,
                 'DIN':0x02, 'DOUT':0x03,
                 'T1':0x4, 'T2':0x5,
                 'T3':0x6, 'T4':0x7}

    def makeargs(op):#-----------------------------
      args = [0, 0]

      for i, arg in enumerate([op['a1'],op['a2']]):
        try:
          args[i] = int(arg)
          continue
        except ValueError:
          pass

        if arg in registers:
          args[i] = registers[arg]
          continue
        if arg in mem_addrs:
          args[i] = mem_addrs[arg]
          continue
        if arg in labels:
            #args[i] = #TODO Linker #TODO TODO TODO TODO TODO TODO TODO TODO
            print output
        raise SyntaxError('Argument '+arg+' not found')

      return args[0], args[1]
#---------------------------------------------------
    output = bytearray()
    ops = [alias(op) for op in ops]

    print 'Assembling ...'

    for i, op in enumerate(ops):
      if 'o' in op:
        a1, a2 = makeargs(op)

        #shift to right position
        if op['o'] == 

        opcode = 0x0
        try:
            opcode |= opcodes[op['o']]
        except IndexError:
            raise SyntaxError('Command '+op['o']+' does not exist')

        opcode |= a1
        opcode |= a2

        #note where a command was placed, used for
        #translation of the jump labels
        ops[i]['p']=len(output)
        output.append(opcode)
    print '---------------------------------------------------------'
    print output#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    exit(1)



        #if (at in [argtypes['CONST']]):
        #  if a is None:
        #    #operand is a potentially unknown label
        #    #place update note and set a to prevent confusion
        #    pinput[i]['u']=len(output)
        #    a=0

         # #!l -> int32_t in network byte order
         # binary=struct.pack('!l',a)
         # output+=binary

    #   print ("Linking...")
    #   #link
    #   for op in pinput:
    #     if ('u' in op):
    #       label=op['a']
    #       res=tuple(filter(lambda x: 'l' in x and x['l'] == label, pinput))[0]
    #       a=res['p']
    #       pos=op['u']
      #
    #       #!l -> int32_t in network byte order
    #       #replace placeholder with address
    #       binary=struct.pack('!l',a)
    #       output[pos:pos+4]=binary
      #
    #   #print ('\n'.join(str(x) for x in pinput))
    #   #print (' '.join('{:02x}'.format(x) for x in output))
      #
    #   return (output)




def alias(cmd):
    if 'o' in cmd:
        if cmd['o']=='NEG':
            cmd['a2'] = cmd['a1']
            cmd['a1'] = 'RZ'
            cmd['o'] = 'SUB'
    return cmd

def main (argv):
    if (len(argv) != 3):
      print ('{} in.x out.y'.format(argv[0]))
      exit(1)

    with open(argv[1], 'r') as i:
        instr = i.read()

    CMDs = parse(instr)
    machinecode = xycompile(CMDs)

    print 'Writing Output...'
    with open(argv[2], 'wb') as o:
        o.write(machinecode)

if __name__ == '__main__':
  main(sys.argv)
