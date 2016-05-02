# -*- coding: utf-8 -*-
import sys #TODO: Change x,y
labels = {}


def parse(text):
  print 'Parsing ...'


  def splitline(line):#-------------------------------------------
    ret = {}
    ret['b'] = 0
    try:
      ret['b'] = int(line)
      ret['o'] = 0
      ret['a1'] = 0
      ret['a2'] = 0
      return ret
    except ValueError:
      pass


    splt = line.split(':')
    #separate labels, keep rest intact
    #if len(splt) == 1 -> op
    if len(splt) == 2: #label
      if splt[1]!='':
          raise SyntaxError('OP in same line with label')

      ret['l'] = splt[0].upper()
      ret['o'] = 'LABEL'
      ret['a1'] = '0'
      ret['a2'] = '0'
      ret['e'] = '0'
      return ret

    elif len(splt)>2:
      raise SyntaxError('More than one lable in a line')


    splt = line.split(' ')
    splt = [s for s in splt if '' != s] #remove whitespaces between ops, args
    #separate arguments, keep opcode intact
    if len(splt) in (0,1):
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
      raise SyntaxError('An operation ('+splt[0]+') got more than 2 arguments')

    if(line):
      ret['o'] = line.upper()
    else:
      ret['o'] = 0

    return ret
#------------------------------------------------------------------

  lines = text.split('\n') #parse into lines

  lines = [l for l in lines if '' != l] #remove multiple newlines

  lines = [x.lstrip() for x in lines] #strip leading whitespaces

  lines = [x.split('//')[0]  for x in lines] #strip comments

  lines = [x.rstrip() for x in lines] #strip trailing whitespaces

  #split the line syntax ([label:]opcode [argument])
  lines = [splitline(x) for x in lines]

  return lines



def xycompile(ops):
    opcodes = {'NOP':0x00, 'LD':0x04,


               'INC':0x10, 'DEC':0x14,
               'NOT':0x08,

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
                 'TIMER':0x4, 'RAM1':0x5,
                 'RAM2':0x6, 'RAM3':0x7}

    def makeargs(op):#-----------------------------
      args = [0, 0]

      if op['o'] in ('JFW','JBW'): #linking
          return args[0], args[1]

      for i, arg in enumerate([op['a1'], op['a2']]):
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

        raise SyntaxError('Argument '+arg+' not found')

      return args[0], args[1]
#---------------------------------------------------
    output = bytearray()
    ops = [alias(op) for op in ops]

    print 'Assembling ...'

    for i, op in enumerate(ops):
      if op['o'] != 'LABEL':#------------------
        arg1, arg2 = makeargs(op) #local because of shift

        #shift to right position
        if op['o'] in ('LDA', 'STA'):
            arg1 <<= 3
        if op['o'] in ('MOV', 'OR', 'AND', 'XOR', 'ADD', 'SUB', 'SEQ', 'SNE'):
            arg1 <<= 2

        byte = 0
        try:
            byte |= opcodes[op['o']]
        except IndexError:
            if op['0'] !=0: #label
              raise SyntaxError('Command '+op['o']+' does not exist')

        byte |= arg1
        byte |= arg2
        byte |= op['b'] #special case

      #Also for Labels
      #note where a command was placed, used for
      #translation of the jump labels
      ops[i]['p']=len(output)
      output.append(byte)
#---------------------------------------------------------------------

    print ("Linking...")
    for op in ops:
      if op['o'] in ('JFW','JBW'): #linking
        for label in ops:
          if 'l' in label:
            if label['l'] == op['a1']:

              output[op['p']] |= op['p'] - label['p']
              break

    for a in output:
        print bin(a)
    return (output)


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
