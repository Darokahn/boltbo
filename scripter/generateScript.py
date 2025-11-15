import os
import cv2

spriteWidth = 48
stageWidth = 128

alignOfImage = 2
alignOfPixel = 1

mode = "desktop"

files = [["stages", file] for file in sorted(os.listdir("assets/stages"))] + [["sprites", file] for file in sorted(os.listdir("assets/sprites"))]
imageBlob = []

def intToBytes(num, length):
    b = bytearray(length)
    for i in range(length):
        b[i] = (num >> (8 * i) & 0xff)
    return b

for idnum, file in enumerate(files):
    while(len(imageBlob) & (~(alignOfImage - 1)) != len(imageBlob)):
        imageBlob.append(0)
    imageType, filename = file
    img = cv2.imread(f"assets/{imageType}/{filename}", cv2.IMREAD_UNCHANGED)
    hasAlpha = False
    alpha = None
    if (img.shape[2] == 4):
        hasAlpha = True
    height, width = img.shape[:2]
    scale = stageWidth / width
    if (imageType == "sprites"):
        scale = spriteWidth / width
    width, height = (int(width * scale), int(height * scale))
    img = cv2.resize(img, (width, height), interpolation=cv2.INTER_AREA)
    expectedDifference = 2 + 2 + 2 + 256 + (width * height * 4)
    imageBlob.extend(intToBytes(idnum, 2))
    imageBlob.extend(intToBytes(width, 2))
    imageBlob.extend(intToBytes(height, 2))
    name = bytearray(256)
    for i, char in enumerate(bytes(filename, "utf-8")):
        name[i] = char
    imageBlob.extend(name)
    buffer = img.tobytes()
    bufferIter = iter(buffer)
    for i in range(len(buffer) // (3 + hasAlpha)):
        b, g, r = next(bufferIter), next(bufferIter), next(bufferIter)
        a = 255
        if (hasAlpha):
            a = next(bufferIter)
        color = bytearray([r, g, b, a])
        imageBlob.extend(color)
imageBlob.extend(intToBytes(-1, 2))

array = "unsigned char imageBlob[] = {" + ", ".join((str(l) for l in imageBlob)) + "};"
print(array)
