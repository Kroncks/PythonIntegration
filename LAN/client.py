# client.py
import socket, netifaces, time, sys

UDP_PORT = 5000

def get_broadcast_ip():
    for iface in netifaces.interfaces():
        addrs = netifaces.ifaddresses(iface)
        if netifaces.AF_INET in addrs:
            for info in addrs[netifaces.AF_INET]:
                ip = info.get('addr')
                bc = info.get('broadcast')
                if ip and bc and not ip.startswith("127."):
                    print(f"[client.py] Interface détectée : {iface} -> IP: {ip}, Broadcast: {bc}", file=sys.stderr)
                    return bc
    print("[client.py] Aucune interface avec broadcast détectée", file=sys.stderr)
    return None

def detect():
    broadcast = get_broadcast_ip()
    print("[VERSION DEBUG] ip broadcast fixed 192.168.4.255:5000 on Boby  ⚠️", file=sys.stderr)
    print("- ignored broadcast ip : ", broadcast, file=sys.stderr)
    broadcast = "192.168.4.255"
    if not broadcast:
        print("[client.py] Erreur : impossible d'obtenir une IP de broadcast.", file=sys.stderr)
        sys.exit(1)

    print(f"[client.py] Envoi d'un ping vers {broadcast}:{UDP_PORT}", file=sys.stderr)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    sock.settimeout(5)

    try:
        sock.sendto(b"PING", (broadcast, UDP_PORT))
        data, addr = sock.recvfrom(1024)
        if data.decode().startswith("PONG"):
            print(f"[client.py] PONG reçu de {addr[0]}", file=sys.stderr)
            print(data.decode().split()[1])  # stdout vers C
        else:
            print("[client.py] Réponse inattendue", file=sys.stderr)
            sys.exit(1)
    except socket.timeout:
        print("[client.py] Timeout : aucune réponse du serveur", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"[client.py] Erreur de communication : {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    detect()
