#! env python3

from scipy.io import wavfile
import scipy.signal as sps
import sys

RATE=6300
BITS=12

def output(file):
    rate, data = wavfile.read(file)
    nsamples = round(len(data) * float(RATE) / rate)
    data = sps.resample(data, nsamples)
    
    # Convert to mono by averaging channels
    data1 = []
    maxval = 0;
    for pair in data:
        val = (pair[0] + pair[1])/2
        if abs(val) > maxval:
            maxval = val
        data1.append(val)

    # Output max 0.6V (1.2V P-P)
    atten = 0.6/3.3*((1 << BITS)/maxval)

    filename = (file.split("/")[-1]).split('.',1)[0]
    print("\nconst Sound sound_%s = {" % filename)
    print("    %u, 2, %u, {" % (len(data1), RATE))
    n = 0
    m = 0
    str = ""
    for sample in data1:
        if n == 0:
            str += "        "

        str += "0x%04x" % (int)((sample*atten) + (1 << (BITS-1)))

        m += 1
        if m < len(data1):
            str += ", "
        
        n += 1
        if n == 10:
            str += "\n"
            n = 0

    print(str)
    print("    }")
    print("};")

        
print("#include \"audio/audio.h\"")

for arg in sys.argv[1:]:
    output(arg)
