#bin/python
import argparse
parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('--fn', dest='filename',action="store", type=str, help="filename")
parser.add_argument('--baud', dest='baud',action="store",default='115200', type=int, help="baud")
args = parser.parse_args()

import numpy
import serial
from serial.tools import list_ports
import time
from PIL import Image
from twisted.internet import reactor

width = 32
height = 32



class Player(object):
        def __init__(self, imagePath, frameOutClosure):
                self.imagePath = imagePath
                self.out = frameOutClosure
                self.fps = 30
                self.reloadImage()

        def reloadImage(self):
                print(self.imagePath)
                i = Image.open(self.imagePath).convert('HSV')   

                print('image size(WxH): ', i.size) 
                out = i.resize([width, height], Image.BOX)
                print('image size(WxH): ', out.size) 
                self.image = numpy.asarray(out)
                print(self.image.shape)

        def step(self):
                now = time.time()
                lineNum = 0
                frame = self.image

                for line in range(0, height):
                        if (line % 2 != 0):
                                start = line * width
                                end = start + width
                                frame[line] = frame[line][::-1]
        
                self.out(frame[::-1])

if __name__ == "__main__":
        filename = args.filename
        ports = [port[0] for port in list_ports.comports()]
        print("Using serial port ", ports[-1])
        port = serial.Serial(ports[-1],baudrate=args.baud, timeout=0, writeTimeout=1)

        def frameOut(colors):
                #print(len(colors))
                port.write(str.encode('*'))
                port.write(bytearray(colors.copy(order='C')))
        
        player = Player(filename, frameOut)

        def loop():
                player.step()
                # reactor.callLater(5, loop)
                # exit()
				
        loop()
        reactor.run()