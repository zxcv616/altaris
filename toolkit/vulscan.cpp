#include <iostream>
#include <cstdlib>
using namespace std;

void scanNetwork(const string& target) {

    string command = "nmap -p 1-1024" + target;
    cout << "Running command: " << command << endl;
    int result = system(command.c_str());

    if (result != 0) {
        cerr << "Error running scan" << endl;
    }
}

int main() {
    string target_ip;
    cout << "Enter IP" << endl;
    cin >> target_ip;

    scanNetwork(target_ip);
    return 0;
}