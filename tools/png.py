import math
import numpy as np
from PIL import Image

data = np.fromfile('out.bin', dtype=np.ubyte)

# B&W
size = int(math.sqrt(data.shape[0]))
nbytes = size * size
reshaped = data[0:nbytes].reshape((size, size))
img = Image.fromarray(reshaped, '1')
img.save('out_bw-1bit.png')
img = Image.fromarray(reshaped, 'L')
img.save('out_bw-8bit.png')

# RGB
size = int(math.sqrt(data.shape[0] / 3))
nbytes = size * size * 3
reshaped = data[0:nbytes].reshape((size, size, 3))
img = Image.fromarray(reshaped, 'RGB')
img.save('out_rgb.png')

