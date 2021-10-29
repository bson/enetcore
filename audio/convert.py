#! env python3

from scipy.io import wavfile
import scipy.signal as sps
import sys

RATE  = 6300
BITS  = 12
VDD   = 3.3
VAVG  = 0.3                       # Target average

FS    = 2**BITS

def output(file):
    rate, data = wavfile.read(file)
    nsamples = round(len(data) * float(RATE) / rate)
    data = sps.resample(data, nsamples)
    
    # Convert to mono by averaging channels
    data1 = []
    sum = 0
    for pair in data:
        val = (pair[0] + pair[1])/2
        sum += abs(val)
        data1.append(val)

    avg = sum/len(data)

    # Scaling factor to create
    atten = VAVG/VDD * (FS/2)/avg

    filename = (file.split("/")[-1]).split('.',1)[0]
    print("\nextern const Sound %s = {" % filename)
    print("    %u, 2, %u, {" % (len(data1), RATE))
    n = 0
    m = 0
    str = ""
    for sample in data1:
        if n == 0:
            str += "        "

        s = sample * atten + (FS/2)
        s = min(s, FS-1)
        s = max(s, 0)

        str += "0x%04x" % (int)(s)

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

        
print("#include \"audio/audio.h\"\n\nnamespace sound {")

for arg in sys.argv[1:]:
    output(arg)

print("} // ns sound")
