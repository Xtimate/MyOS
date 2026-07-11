import socket
import struct

INTERFACE = "tap0"
SERVER_IP = "192.168.100.1"
OFFERED_IP = "192.168.100.50"
SUBNET_MASK = "255.255.255.0"
LEASE_TIME = 86400  # 24 hours, in seconds


def build_reply(xid, chaddr, msg_type, yiaddr):
    op = 2  # BOOTREPLY
    htype = 1
    hlen = 6
    hops = 0
    secs = 0
    flags = 0x8000
    ciaddr = b"\x00\x00\x00\x00"
    yiaddr_b = socket.inet_aton(yiaddr)
    siaddr = socket.inet_aton(SERVER_IP)
    giaddr = b"\x00\x00\x00\x00"
    chaddr_padded = chaddr + b"\x00" * (16 - len(chaddr))
    sname = b"\x00" * 64
    file_ = b"\x00" * 128
    magic = struct.pack("!I", 0x63825363)

    options = b""
    options += bytes([53, 1, msg_type])  # message type
    options += bytes([1, 4]) + socket.inet_aton(SUBNET_MASK)  # subnet mask
    options += bytes([3, 4]) + socket.inet_aton(SERVER_IP)  # router
    options += bytes([54, 4]) + socket.inet_aton(SERVER_IP)  # server identifier
    options += bytes([51, 4]) + struct.pack("!I", LEASE_TIME)  # lease time
    options += bytes([255])  # end

    packet = struct.pack("!BBBBIHH", op, htype, hlen, hops, xid, secs, flags)
    packet += (
        ciaddr
        + yiaddr_b
        + siaddr
        + giaddr
        + chaddr_padded
        + sname
        + file_
        + magic
        + options
    )
    # pad to at least 300 bytes total, common DHCP minimum
    if len(packet) < 300:
        packet += b"\x00" * (300 - len(packet))
    return packet


def get_option(options, opt_type):
    i = 0
    while i < len(options):
        t = options[i]
        if t == 255:
            break
        length = options[i + 1]
        value = options[i + 2 : i + 2 + length]
        if t == opt_type:
            return value
        i += 2 + length
    return None


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.SOL_SOCKET, 25, INTERFACE.encode())  # SO_BINDTODEVICE
    sock.bind(("0.0.0.0", 67))

    print(f"DHCP test server listening on {INTERFACE}...")

    while True:
        data, addr = sock.recvfrom(1024)
        xid = struct.unpack("!I", data[4:8])[0]
        chaddr = data[28:34]  # first 6 bytes of chaddr (MAC)
        options = data[240:]

        msg_type = get_option(options, 53)
        if not msg_type:
            continue

        if msg_type[0] == 1:  # DISCOVER
            print(f"DISCOVER from {chaddr.hex(':')}, xid={hex(xid)}")
            reply = build_reply(xid, chaddr, 2, OFFERED_IP)  # OFFER
            sock.sendto(reply, ("255.255.255.255", 68))
            print(f"Sent OFFER: {OFFERED_IP}")

        elif msg_type[0] == 3:  # REQUEST
            print(f"REQUEST from {chaddr.hex(':')}, xid={hex(xid)}")
            reply = build_reply(xid, chaddr, 5, OFFERED_IP)  # ACK
            sock.sendto(reply, ("255.255.255.255", 68))
            print(f"Sent ACK: {OFFERED_IP}")


if __name__ == "__main__":
    main()
