import socket, struct


def query(name):
    hdr = struct.pack(">HHHHHH", 0x1234, 0x0100, 1, 0, 0, 0)
    q = b"".join(bytes([len(p)]) + p.encode() for p in name.split(".")) + b"\x00"
    return hdr + q + struct.pack(">HH", 1, 1)  # TYPE_A, CLASS_IN


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.settimeout(3)
s.sendto(query("koolDomain.cum"), ("127.0.0.1", 5353))
resp, _ = s.recvfrom(512)
print("IP:", ".".join(map(str, resp[-4:])))
