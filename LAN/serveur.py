import os
import socket
import threading
from time import sleep

import netifaces
import sys

UDP_PORT = 5000
TCP_PORT = 12345

NB_PLAYERS_FILE = '../LAN/tmp/NB_PLAYERS.txt'
try:
    with open(NB_PLAYERS_FILE, 'r') as f:
        MAX_CLIENTS = int(f.read().strip())
        print(" [Serveur Python] MAX_CLIENTS =",MAX_CLIENTS)
except Exception as e:
    print(f" [Serveur Python] Erreur lors de la lecture du nombre de joueurs : {e}")
    print(" [Serveur Python] Client max = 4 (défault)")
    MAX_CLIENTS = 4

clients = []
client_ids = {}
available_ids = [0, 1, 2, 3]
clients_lock = threading.Lock()

STATUT_FILE = '../LAN/tmp/serveur_statut.txt'

stop_event = threading.Event()

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
    print(f" [TCP] Nouveau client connecté depuis {addr}")
    with clients_lock:
        client_id = available_ids.pop(0)
        clients.append(conn)
        client_ids[conn] = client_id

        # Vérifie si le serveur est complet
        if len(clients) == MAX_CLIENTS:
            client_zero_conn = next((c for c in clients if client_ids[c] == 0), None)
            if client_zero_conn:
                try:
                    client_zero_conn.sendall(b"READY\n")
                    print(" [TCP] READY envoyé au client 0.")
                except Exception as e:
                    print(f" [TCP] Impossible d’envoyer READY au client 0 : {e}")

    try:
        conn.sendall(f"ID:{client_id}".encode())

        while not stop_event.is_set():
            data = conn.recv(1024)
            if not data:
                break
            message = data.decode().strip()
            print(f" [TCP] Message du client {client_id} : {message}")

            # Si le client 0 demande à démarrer la partie
            if message == "START?":
                if client_id == 0:
                    with clients_lock:
                        if len(clients) == MAX_CLIENTS:
                            print(" [TCP] Client 0 a demandé le démarrage. Envoi de 'START' à tous les clients.")
                            for c in clients:
                                target_id = client_ids.get(c, '?')
                                try:
                                    c.sendall(b"START\n")
                                    print(f" [TCP] START envoyé à client {target_id}")
                                except Exception as e:
                                    print(f" [TCP] Erreur lors de l'envoi de START à client {target_id} : {e}")
                        else:
                            print(" [TCP] Démarrage refusé : tous les clients ne sont pas encore connectés.")
                else:
                    print(f" [TCP] Client {client_id} a tenté un START?, mais seul le client 0 est autorisé.")

            # Si le message est "STOP", arrêter le serveur
            elif message.upper() == "STOP":
                print(" [TCP] Message STOP reçu. Arrêt du serveur demandé.")
                stop_event.set()
                break

            # Sinon, diffusion aux autres clients
            else:
                with clients_lock:
                    for other_conn in clients:
                        if other_conn != conn:
                            try:
                                other_conn.sendall(f"{client_id}: {message}".encode())
                            except Exception as e:
                                print(f" [TCP] Erreur lors de l'envoi à un autre client : {e}")

    except Exception as e:
        print(f" [TCP] Erreur avec le client {client_id}: {e}")
    finally:
        with clients_lock:
            if conn in clients:
                clients.remove(conn)
                available_ids.append(client_ids[conn])
                available_ids.sort()
                del client_ids[conn]
        try:
            conn.close()
        except Exception as e:
            print(f" [TCP] Erreur lors de la fermeture de la connexion : {e}")
        print(f" [TCP] Client {client_id} déconnecté")

def start_udp_discovery():
    local_ip = get_local_ip()
    print(f" [UDP] IP locale du serveur : {local_ip}")
    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    udp_sock.bind(("", UDP_PORT))
    print(" [UDP] Serveur prêt à répondre aux PING...")
    while not stop_event.is_set():
        try:
            udp_sock.settimeout(1.0)
            data, addr = udp_sock.recvfrom(1024)
            if data.decode().strip() == "PING":
                udp_sock.sendto(f"PONG {local_ip}".encode(), addr)
        except socket.timeout:
            continue
        except Exception as e:
            print(f" [UDP] Erreur : {e}")
    udp_sock.close()

def start_tcp_server():
    tcp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    tcp_sock.bind(("", TCP_PORT))
    tcp_sock.listen()
    print(f" [TCP] Serveur TCP en écoute sur le port {TCP_PORT}...")

    with open(STATUT_FILE, 'w') as f:
        f.write('ready')
        print(" [Serveur Python] ready dans ", STATUT_FILE)

    while not stop_event.is_set():
        try:
            tcp_sock.settimeout(1.0)
            conn, addr = tcp_sock.accept()
        except socket.timeout:
            continue

        with clients_lock:
            if len(clients) >= MAX_CLIENTS or not available_ids:
                print(f" [TCP] Connexion refusée (max {MAX_CLIENTS} atteint) : {addr}")
                conn.sendall(b"FULL\n")
                conn.close()
                continue
        threading.Thread(target=handle_client, args=(conn, addr)).start()

    print(" [Serveur Python] Fermeture du serveur : déconnexion des clients...")
    with clients_lock:
        for conn in clients[:]:
            try:
                conn.sendall(b"STOP\n")
            except Exception as e:
                print(f" [TCP] Impossible d'envoyer STOP : {e}")
            try:
                conn.close()
            except Exception as e:
                print(f" [TCP] Erreur lors de la fermeture de la connexion : {e}")
        clients.clear()

    if os.path.exists(STATUT_FILE):
        os.remove(STATUT_FILE)
    print(" [Serveur Python] Serveur arrêté proprement.")

if __name__ == "__main__":
    print(" [Serveur Python] Démarrage du serveur...")
    udp_thread = threading.Thread(target=start_udp_discovery, daemon=True)
    udp_thread.start()
    start_tcp_server()
