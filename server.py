#bin/python
import argparse
parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('--fn', dest='filename',action="store", type=str, help="filename")
parser.add_argument('--baud', dest='baud',action="store",default='921600', type=int, help="baud")
args = parser.parse_args()

import numpy
import serial
from serial.tools import list_ports
import time
from PIL import Image
from twisted.internet import reactor
import cv2

width = 32
height = 32



class Player(object):
        def __init__(self, imagePath, frameOutClosure):
                self.imagePath = imagePath
                self.out = frameOutClosure
                self.fps = 30
                self.currentFrame = 0
                self.images = []
                self.reloadImage()


        def reloadImage(self):
                print(self.imagePath)
                self.images = []
                vidcap = cv2.VideoCapture(self.imagePath)
                success, image = vidcap.read()
                self.images.append(self.resizeImage(image))
                count = 0
                while success:
                        success, image = vidcap.read()
                        if success:
                                self.images.append(self.resizeImage(image))
                                print('Read a new frame: ', count)
                                count += 1
                print(len(self.images))
        def resizeImage(self, image):
                converted = cv2.cvtColor(image,cv2.COLOR_BGR2RGB)
                i = Image.fromarray(converted).convert('HSV')  
                print('image size(WxH): ', i.size) 
                out = i.resize([width, height])
                print('image size(WxH): ', out.size) 
                imgArr = numpy.asarray(out)
                lineNum = 0
                for line in range(0, height):
                        if (line % 2 != 0):
                                start = line * width
                                end = start + width
                                imgArr[line] = imgArr[line][::-1] 
                return imgArr
        def step(self):
                # print('Displaying Frame: ', self.currentFrame)
                frame = self.images[self.currentFrame]
                self.currentFrame+=1
                if self.currentFrame >= len(self.images):
                        self.currentFrame = 0
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
                reactor.callLater(.016, loop)
                # exit()
				
        loop()
        reactor.run()