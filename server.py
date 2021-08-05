# bin/python
import argparse
import numpy
import serial
from serial.tools import list_ports
import time
from PIL import Image
from twisted.internet import reactor
import math
import sys
import cv2

parser = argparse.ArgumentParser(description='Process some integers.')
parser.add_argument('--fn', dest='filename', action="store", type=str, help="filename")
parser.add_argument('--baud', dest='baud', action="store", default='921600', type=int, help="baud")
args = parser.parse_args()

width = 32
height = 32

GAMMA = 2.6

# This is a set of color values where error diffusion dithering won't be
# applied (is still calculated for subsequent pixels, but not applied).
# Here it's set up for primary colors, so these always appear unfettered
# in output images.
PASSTHROUGH = ((0, 0, 0),
               (255, 0, 0),
               (255, 255, 0),
               (0, 255, 0),
               (0, 255, 255),
               (0, 0, 255),
               (255, 0, 255),
               (255, 255, 255))

REDUCE = False  # Assume 24-bit BMP out unless otherwise specified


class Player(object):
    def __init__(self, imagePath, frameOutClosure):
        self.imagePath = imagePath
        self.out = frameOutClosure
        self.fps = 30
        self.currentFrame = 0
        self.images = []
        self.reloadImage()

    def process(self, img, output_8_bit=False, passthrough=PASSTHROUGH):
        """Given a color image filename, load image and apply gamma correction
                and error-diffusion dithering while quantizing to 565 color
                resolution. If output_8_bit is True, image is reduced to 8-bit
                paletted mode after quantization/dithering. If passthrough (a list
                of 3-tuple RGB values) is provided, dithering won't be applied to
                colors in the provided list, they'll be quantized only (allows areas
                of the image to remain clean and dither-free). Writes a BMP file
                with the same name plus '-processed' and .BMP extension.
                """
        err_next_pixel = (0, 0, 0)  # Error diffused to the right
        err_next_row = [(0, 0, 0) for _ in range(img.size[0])]  # " diffused down
        for row in range(img.size[1]):
            for column in range(img.size[0]):
                pixel = img.getpixel((column, row))
                want = (math.pow(pixel[0] / 255.0, GAMMA) * 31.0,  # Gamma and
                        math.pow(pixel[1] / 255.0, GAMMA) * 63.0,  # quantize
                        math.pow(pixel[2] / 255.0, GAMMA) * 31.0)  # to 565 res
                if pixel in passthrough:  # In passthrough list?
                    got = (pixel[0] >> 3,  # Take color literally,
                           pixel[1] >> 2,  # though quantized
                           pixel[2] >> 3)
                else:
                    got = (min(max(int(err_next_pixel[0] * 0.5 +  # Diffuse
                                       err_next_row[column][0] * 0.25 +  # from
                                       want[0] + 0.5), 0), 31),  # prior XY
                           min(max(int(err_next_pixel[1] * 0.5 +
                                       err_next_row[column][1] * 0.25 +
                                       want[1] + 0.5), 0), 63),
                           min(max(int(err_next_pixel[2] * 0.5 +
                                       err_next_row[column][2] * 0.25 +
                                       want[2] + 0.5), 0), 31))
                err_next_pixel = (want[0] - got[0],
                                  want[1] - got[1],
                                  want[2] - got[2])
                err_next_row[column] = err_next_pixel
                rgb565 = ((got[0] << 3) | (got[0] >> 2),  # Quantized result
                          (got[1] << 2) | (got[1] >> 4),  # after dither
                          (got[2] << 3) | (got[2] >> 2))
                img.putpixel((column, row), rgb565)  # Put pixel back in image

        if output_8_bit:
            img = img.convert('P', palette=Image.ADAPTIVE)

        return img

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
        converted = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        i = Image.fromarray(converted).convert('RGB')
        print('image size(WxH): ', i.size)
        out = i.resize([width, height])
        print('image size(WxH): ', out.size)
        out = self.process(out)

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
        self.currentFrame += 1
        self.out(frame[::-1])
        if self.currentFrame >= len(self.images):
            self.currentFrame = 0
            time.sleep(3)


if __name__ == "__main__":
    filename = args.filename
    ports = [port[0] for port in list_ports.comports()]
    print("Using serial port ", ports[-1])
    port = serial.Serial(ports[-1], baudrate=args.baud, timeout=0, writeTimeout=1)


    def frameOut(colors):
        # print(len(colors))
        port.write(str.encode('*'))
        port.write(bytearray(colors.copy(order='C')))


    player = Player(filename, frameOut)


    def loop():
        player.step()
        reactor.callLater(.010, loop)


    loop()
    reactor.run()
