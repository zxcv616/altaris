import nmap


def scan_network(target):
    nm = nmap.PortScanner()
    nm.scan(target, '1-1024')

    for host in nm.all_hosts():
        print(f'Host: {host}')
        print(f'Status: {nm[host].state()}')
        print(f'Open ports: {", ".join(map(str, nm[host].get("tcp", {}).keys()))}')
        print()
        for proto in nm[host].all_protocols():
            ports = nm[host][proto].keys()
            for port in ports:
                print(f'Protocol: {proto}')
                print(f'Port: {port}')
                print(f'State: {nm[host][proto][port]["state"]}')
                print(f'Service: {nm[host][proto][port].get("service", "")}')
                print()

if __name__ == '__main__':
    target_ip = input("Enter IP")
    scan_network(target_ip)

