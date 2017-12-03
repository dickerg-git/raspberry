#! /usr/bin/python3.4 -tt
# Copyright 2017 RGD. All Rights Reserved.

import sys
import binascii

# This is standard TEMPLATE that defines the main() function.
def main():

   if( len(sys.argv) == 3 ):
       print( "Dumping: ", sys.argv[2] )
   else: print( "Use: filedump.py words filename" )

   per_line = int(sys.argv[1])
   linechunk = (int(sys.argv[1]) * 4)
   print(per_line, linechunk)

   # open the file for reading
   readfile = open( sys.argv[2], "rb")
   count = 0

   # Get a file chunk of length linechunk...
   #for linecount in range(100):
   while True:
       readline = readfile.read(linechunk)
       if len( readline ) < per_line: break
       print("----------")
       print( readline )

       # Dump each file segment

       offset = 0
       print( hex(count), ": ", end="")
       for j in range(per_line):
           #print( binascii.hexlify( readline[offset:offset+4].encode() ), end="")
           hexval = int.from_bytes(readline[offset:offset+4], byteorder='little')
           print( "0x{0:08X}".format(hexval), end=" " )
           #print( '0x%08X'%(hexval), end=" ")
           offset += 4
       print()
       count = count + linechunk

   readfile.close()






# This is standard TEMPLATE that calls the main() function.
if __name__ == '__main__':
    main()
