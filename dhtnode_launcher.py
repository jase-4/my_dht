import subprocess
import time

processes = []

def start_bootstrap_node(ip_address, port):
    proc = subprocess.Popen(['./build/dht_node', 'boot', ip_address, str(port)])
    processes.append(proc)

def start_dht_node(ip_address, port, boot_ip_address, boot_port):
    proc = subprocess.Popen(['./build/dht_node', 'node', ip_address, str(port), boot_ip_address, str(boot_port)])
    processes.append(proc)

def stop_all_nodes():
    for proc in processes:
        proc.terminate()  
        try:
            proc.wait(timeout=5) 
        except subprocess.TimeoutExpired:
            proc.kill()

def main():
    try:
        bootstrap_ip = "127.0.0.1"  
        bootstrap_port = 8084

        start_bootstrap_node(bootstrap_ip, bootstrap_port)
        time.sleep(1)

        # number of nodes
        num_nodes = 20
        for i in range(1, num_nodes + 1):
            node_ip = "127.0.0.1"
            node_port = 8085 + i 
            boot_ip = bootstrap_ip
            boot_port = bootstrap_port

            start_dht_node(node_ip, node_port, boot_ip, boot_port)
            time.sleep(0.5) 

        print("All nodes started. Press Ctrl+C to stop.")

        while True:
            time.sleep(1)

    except KeyboardInterrupt:
        print("\nStopping all nodes...")
        stop_all_nodes()

if __name__ == "__main__":
    main()
