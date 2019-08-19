import cv2
import numpy as np
import sys
import urllib.request
host = "192.168.137.243:81/stream"
if len(sys.argv)>1:
    host = sys.argv[1]

hoststr = 'http://' + host
print('Streaming ' + hoststr)

stream=urllib.request.urlopen(hoststr)

bytes = b''
while True:
    bytes+=stream.read(1024)
    a = bytes.find(b'\xff\xd8')
    b = bytes.find(b'\xff\xd9')
    if a!=-1 and b!=-1:
        jpg = bytes[a:b+2]
        bytes= bytes[b+2:]
        if not jpg:
            continue
        i = cv2.imdecode(np.fromstring(jpg, dtype=np.uint8), 1)
        cv2.imshow(hoststr,i)
        if cv2.waitKey(1) ==27:
            exit(0)
			
