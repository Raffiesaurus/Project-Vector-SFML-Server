#include "main.h"

void parse_config_file(const std::string& filename, std::string& ip, int& port) {
	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string key;
		if (std::getline(iss, key, '=')) {
			std::string value;
			if (std::getline(iss, value)) {
				// Do something with the key-value pair.
				if (key == "ip") {
					ip = value;
				} else if (key == "port") {
					port = stoi(value);
				}
			}
		}
	}
}

struct PlayerConnectionDetails {
	unsigned short port;
	sf::IpAddress ipAddress;
};

int main() {

	PlayerConnectionDetails player1;
	PlayerConnectionDetails player2;

	int playersConnected = 0;

	std::string ip = {};
	int port = 0;
	parse_config_file("net_config.txt", ip, port);

	sf::TcpListener listener;
	sf::UdpSocket udpSocket;

	// bind the listener to a port
	if (listener.listen(port) != sf::Socket::Done) {
		// error...
	} else {
		std::cout << "TCP Listening with port " << port << std::endl;
	}

	if (udpSocket.bind(port) != sf::Socket::Done) {
		// error...
	} else {
		std::cout << "UDP Listening with port " << port << std::endl;
	}

	sf::TcpSocket tcpClient1;
	sf::TcpSocket tcpClient2;

	while (playersConnected < 2) {

		if (tcpClient1.getLocalPort() == 0) {
			if (listener.accept(tcpClient1) != sf::Socket::Done) {
				// error...
			} else {
				std::cout << "First client connected with deets : " << tcpClient1.getLocalPort() << " " << tcpClient1.getRemotePort() << " " << tcpClient1.getRemoteAddress() << std::endl;
				player1.ipAddress = tcpClient1.getRemoteAddress();
				player1.port = tcpClient1.getRemotePort();
				playersConnected++;
			}
		} else {
			if (listener.accept(tcpClient2) != sf::Socket::Done) {
				// error...
			} else {
				std::cout << "First client connected with deets : " << tcpClient2.getLocalPort() << " " << tcpClient2.getRemotePort() << " " << tcpClient2.getRemoteAddress() << std::endl;
				player2.ipAddress = tcpClient2.getRemoteAddress();
				player2.port = tcpClient2.getRemotePort();
				playersConnected++;
			}
		}
		/*if () {
			playersConnected--;
		}*/
	}

	std::cout << "\n\n\n\n Connection deets: \n\n";
	std::cout << "Player 1: \n";
	std::cout << "Ip: " << player1.ipAddress << std::endl;
	std::cout << "Port: " << player1.port << std::endl << std::endl << std::endl;
	std::cout << "Player 2: \n";
	std::cout << "Ip: " << player2.ipAddress;
	std::cout << "Port: " << player2.port;

	char startData[2] = "1";

	// TCP socket:
	if (tcpClient1.send(startData, 2) != sf::Socket::Done) {
		// error...
	}

	if (tcpClient2.send(startData, 2) != sf::Socket::Done) {
		// error...
	}


	while (1) {
	}
	//sf::TcpListener listener;

	//// bind the listener to a port
	//if (listener.listen(53000) != sf::Socket::Done) {
	//	// error...
	//	std::cout << "Error 2";
	//}
	//sf::TcpSocket client;
	//if (listener.accept(client) != sf::Socket::Done) {
	//	// error...
	//	std::cout << "Error 3";
	//}

	//sf::UdpSocket udpSocket;

	//// bind the socket to a port
	//if (udpSocket.bind(54000) != sf::Socket::Done) {
	//	// error...
	//}


	//char data[100];

	//// TCP socket:
	//if (tcpSocket.send(data, 100) != sf::Socket::Done) {
	//	// error...
	//	std::cout << "Error 4";
	//}
	//std::size_t received;

	//// TCP socket:
	//if (tcpSocket.receive(data, 100, received) != sf::Socket::Done) {
	//	// error...
	//	std::cout << "Error 5";
	//}
	//std::cout << "Received " << received << " bytes" << std::endl;

	return 0;
}
