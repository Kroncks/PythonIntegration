import socket
import threading
import netifaces
import sys

UDP_PORT = 5000
TCP_PORT = 12345
MAX_CLIENTS = 4

clients = []
client_ids = {}
available_ids = [0, 1, 2, 3]
clients_lock = threading.Lock()

def get_local_ip():
    interfaces = netifaces.interfaces()
    for iface in interfaces:
        ifaddresses = netifaces.ifaddresses(iface)
        if netifaces.AF_INET in ifaddresses:
            for link in ifaddresses[netifaces.AF_INET]:
                ip = link.get('addr')
                if ip and not ip.startswith("127."):
                    return ip
    return "127.0.0.1"

def handle_client(conn, addr):
    print(f"[TCP] Nouveau client connecté depuis {addr}")
    with clients_lock:
        client_id = available_ids.pop(0)
        clients.append(conn)
        client_ids[conn] = client_id

    try:
        conn.sendall(f"ID:{client_id}".encode())
        while True:
            data = conn.recv(1024)
            if not data:
                break
            print(f"[TCP] Message du client {client_id} : {data.decode()}")
    except Exception as e:
        print(f"[TCP] Erreur avec le client {client_id}: {e}")
    finally:
        with clients_lock:
            if conn in clients:
                clients.remove(conn)
                available_ids.append(client_ids[conn])
                available_ids.sort()
                del client_ids[conn]
        conn.close()
        print(f"[TCP] Client {client_id} déconnecté")

def start_udp_discovery():
    local_ip = get_local_ip()
    print(f"[UDP] IP locale du serveur : {local_ip}")
    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    udp_sock.bind(("", UDP_PORT))
    print("[UDP] Serveur prêt à répondre aux PING...")
    while True:
        try:
            data, addr = udp_sock.recvfrom(1024)
            if data.decode().strip() == "PING":
                udp_sock.sendto(f"PONG {local_ip}".encode(), addr)
        except Exception as e:
            print(f"[UDP] Erreur : {e}")

def start_tcp_server():
    tcp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    tcp_sock.bind(("", TCP_PORT))
    tcp_sock.listen()
    print(f"[TCP] Serveur TCP en écoute sur le port {TCP_PORT}...")

    while True:
        conn, addr = tcp_sock.accept()
        with clients_lock:
            if len(clients) >= MAX_CLIENTS or not available_ids:
                print(f"[TCP] Connexion refusée (max {MAX_CLIENTS} atteint) : {addr}")
                conn.sendall(b"FULL")
                conn.close()
                continue
        threading.Thread(target=handle_client, args=(conn, addr)).start()

if __name__ == "__main__":
    print("[Serveur Python] Démarrage du serveur...")
    udp_thread = threading.Thread(target=start_udp_discovery, daemon=True)
    udp_thread.start()
    start_tcp_server()
