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

struct PositionData {
	float x;
	float y;
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
	udpSocket.setBlocking(false);

	// bind the listener to a port
	if (listener.listen(port) != sf::Socket::Done) {
		std::cerr << "Error listening on TCP port " << port << std::endl;
		return 1;
	} else {
		std::cout << "TCP Listening with port " << port << std::endl;
	}

	if (udpSocket.bind(port) != sf::Socket::Done) {
		std::cerr << "Error binding UDP socket to port " << port << std::endl;
		return 1;
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
				std::cout << "First client connected." << std::endl;
				player1.ipAddress = tcpClient1.getRemoteAddress();
				player1.port = tcpClient1.getRemotePort();
				playersConnected++;
			}
		} else {
			if (listener.accept(tcpClient2) != sf::Socket::Done) {
				// error...
			} else {
				std::cout << "Second client connected." << std::endl;
				player2.ipAddress = tcpClient2.getRemoteAddress();
				player2.port = tcpClient2.getRemotePort();
				playersConnected++;
			}
		}
	}

	std::cout << "\n\n\nConnection deets: \n\n";
	std::cout << "Player 1: \n";
	std::cout << "Ip: " << player1.ipAddress << std::endl;
	std::cout << "Port: " << player1.port << std::endl << std::endl << std::endl;
	std::cout << "Player 2: \n";
	std::cout << "Ip: " << player2.ipAddress << std::endl;
	std::cout << "Port: " << player2.port << std::endl;

	char startData[2] = "1";

	if (tcpClient1.send(startData, 2) != sf::Socket::Done) {
		std::cerr << "Error sending data to Player 1." << std::endl;
	}

	if (tcpClient2.send(startData, 2) != sf::Socket::Done) {
		std::cerr << "Error sending data to Player 2." << std::endl;
	}

	std::cout << "\n\nGame has begun.\n" << std::endl;

	//sf::SocketSelector selector;
	//selector.add(udpSocket);
	sf::Packet packet1;
	sf::Packet packet2;
	while (1) {

		//if (selector.wait()) {
		if (udpSocket.receive(packet1, player1.ipAddress, player1.port) == sf::Socket::Done) {
			PositionData player1Data;
			packet1 >> player1Data.x >> player1Data.y;
			packet1.clear();
			std::cout << "\nData received from player 1: " << player1Data.x << "  " << player1Data.y;
		}

		if (udpSocket.receive(packet2, player2.ipAddress, player2.port) == sf::Socket::Done) {
			PositionData player2Data;
			packet2 >> player2Data.x >> player2Data.y;
			packet2.clear();
			std::cout << "\nData received from player 2: " << player2Data.x << "  " << player2Data.y;
		}
		//}

		/*sf::Packet packet1;
		if (udpSocket.receive(packet1, player1.ipAddress, player1.port) != sf::Socket::Done) {

		} else {
			PositionData player1Data;
			packet1 >> player1Data.x >> player1Data.y;
			std::cout << "\nData received from player 1: " << player1Data.x << "  " << player1Data.y;
		}

		sf::Packet packet2;
		if (udpSocket.receive(packet2, player2.ipAddress, player2.port) != sf::Socket::Done) {

		} else {
			PositionData player2Data;
			packet2 >> player2Data.x >> player2Data.y;
			std::cout << "\nData received from player 2: " << player2Data.x << "  " << player2Data.y;
		}*/

		//sf::Packet packet1;
		//udpSocket.receive(packet1, player1.ipAddress, player1.port);
		////if (packet1.getDataSize() > 0) {
		//PositionData player1Data;
		//packet1 >> player1Data.x >> player1Data.y;
		//std::cout << "\nData received from player 1: " << player1Data.x << "  " << player1Data.y;
		///*} else {
		//	std::cout << "\n Packet 1 Empty";
		//}*/

		//sf::Packet packet2;
		//udpSocket.receive(packet2, player2.ipAddress, player2.port);
		////if (packet2.getDataSize() > 0) {
		//PositionData player2Data;
		//packet2 >> player2Data.x >> player2Data.y;
		//std::cout << "\nData received from player 2: " << player2Data.x << "  " << player2Data.y;
		///*} else {
		//	std::cout << "\n Packet 2 Empty";
		//}*/
	}

	return 0;
}
