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
					port = std::stoi(value);
				}
			}
		}
	}
}

struct PacketData {
	int playerNumber;
	float spritePosX;
	float spritePosY;
	float bulletPosX;
	float bulletPosY;
	float rotationAngle;
	int mHealth = 100;
	int oHealth = 100;
};


struct PlayerConnectionDetails {
	unsigned short port = 0;
	sf::IpAddress ipAddress;
	PacketData currentData;
};

//sf::Packet& operator <<(sf::Packet& packet, const PacketData& data) {
//	return packet << data.playerNumber << data.spritePosX << data.spritePosY << data.bulletPosX << data.bulletPosY << data.rotationAngle << data.health;
//}
//
//sf::Packet& operator >>(sf::Packet& packet, PacketData& data) {
//	return packet >> data.playerNumber >> data.spritePosX >> data.spritePosY >> data.bulletPosX >> data.bulletPosY >> data.rotationAngle >> data.health;
//}
//
//std::ostream& operator<<(std::ostream& os, const PacketData& data) {
//	os << data.playerNumber << " " << data.bulletPosX << " " << data.bulletPosY << " " << data.health << " " << data.rotationAngle << " " << data.spritePosX << " " << data.spritePosY << std::endl;
//	return os;
//}

bool CheckGameOver(PlayerConnectionDetails* player1, PlayerConnectionDetails* player2, sf::TcpSocket* tcpClient1, sf::TcpSocket* tcpClient2, int* playerCount) {
	if (player1->currentData.mHealth == 0) {
		if (tcpClient1->send("lose", 2) != sf::Socket::Done) {
			std::cerr << "Error sending data to Player 1." << std::endl;
		}
		if (tcpClient2->send("win", 2) != sf::Socket::Done) {
			std::cerr << "Error sending data to Player 2." << std::endl;
		}
		tcpClient1->disconnect();
		tcpClient2->disconnect();
		playerCount = 0;
	}

	if (player2->currentData.mHealth == 0) {
		if (tcpClient2->send("lose", 2) != sf::Socket::Done) {
			std::cerr << "Error sending data to Player 2." << std::endl;
		}
		if (tcpClient1->send("win", 2) != sf::Socket::Done) {
			std::cerr << "Error sending data to Player 1." << std::endl;
		}
		tcpClient1->disconnect();
		tcpClient2->disconnect();
		playerCount = 0;
	}

	return false;
}

void CheckDisconnect(sf::TcpSocket* tcpClient1, sf::TcpSocket* tcpClient2, int* playerCount) {
	bool checkResult = false;

	if (tcpClient1->send("check", 6) == sf::Socket::Disconnected) {
		checkResult = true;
		playerCount = 0;
		if (tcpClient2->send("win", 2) != sf::Socket::Done) {
			std::cerr << "Error sending data to Player 2." << std::endl;
		}
	}

	if (!checkResult && tcpClient2->send("check", 6) == sf::Socket::Disconnected) {
		checkResult = true;
		playerCount = 0;
		if (tcpClient1->send("win", 2) != sf::Socket::Done) {
			std::cerr << "Error sending data to Player 1." << std::endl;
		}
	}
}


int main() {

	PlayerConnectionDetails player1;
	PlayerConnectionDetails player2;

	int playersConnected = 0;
	int port = 0;

	bool gameBegun = false;
	bool gameEnded = false;

	char startData[2] = "0";
	std::string ip = {};

	parse_config_file("net_config.txt", ip, port);

	sf::TcpListener listener;

	sf::TcpSocket tcpClient1;
	sf::TcpSocket tcpClient2;

	sf::UdpSocket udpSocket;

	sf::Packet packet;

	auto lastCheckTime = std::chrono::system_clock::now();

	udpSocket.setBlocking(false);
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

	std::cout << "\n\n\nConnection details: \n\n";
	std::cout << "Player 1: \n";
	std::cout << "Ip: " << player1.ipAddress << std::endl;
	std::cout << "Port: " << player1.port << std::endl << std::endl << std::endl;
	std::cout << "Player 2: \n";
	std::cout << "Ip: " << player2.ipAddress << std::endl;
	std::cout << "Port: " << player2.port << std::endl;

	std::cout << "\n\nGame has begun.\n" << std::endl;

	while (1) {
		if (playersConnected < 2 && !gameBegun && !gameEnded) {
			std::cout << "First initialisation, accepting players.\n";
			if (tcpClient1.getLocalPort() == 0) {
				if (listener.accept(tcpClient1) != sf::Socket::Done) {
					// error...
				} else {
					std::cout << "First client connected." << std::endl;
					player1.ipAddress = tcpClient1.getRemoteAddress();
					player1.port = tcpClient1.getRemotePort();
					char data[4];
					std::size_t received;

					if (tcpClient1.receive(data, 4, received) != sf::Socket::Done) {
						// error...
					} else {
						player1.currentData.mHealth = std::stoi(data);
						if (tcpClient1.send("0", 2) != sf::Socket::Done) {
							std::cerr << "Error sending data to Player 1." << std::endl;
						}
						playersConnected++;
					}
				}
			} else {
				if (listener.accept(tcpClient2) != sf::Socket::Done) {
					// error...
				} else {
					std::cout << "Second client connected." << std::endl;
					player2.ipAddress = tcpClient2.getRemoteAddress();
					player2.port = tcpClient2.getRemotePort();
					char data[4];
					std::size_t received;

					if (tcpClient2.receive(data, 4, received) != sf::Socket::Done) {
						// error...
					} else {
						player2.currentData.mHealth = std::stoi(data);
						if (tcpClient2.send("1", 2) != sf::Socket::Done) {
							std::cerr << "Error sending data to Player 2." << std::endl;
						}
						playersConnected++;
					}
				}
			}
			//gameBegun = (playersConnected == 2);
			if (playersConnected == 2) {
				gameBegun = true;
				char startData[2] = "0";

				if (tcpClient1.send(startData, 2) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 1." << std::endl;
				}

				if (tcpClient2.send(startData, 2) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 2." << std::endl;
				}
			}
		} else if (playersConnected == 2 && gameBegun && !gameEnded) {
			//std::cout << "\nRunning the game.";
			auto now = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = now - lastCheckTime;
			if (elapsed_seconds.count() > 2) {
				CheckDisconnect(&tcpClient1, &tcpClient2, &playersConnected);
			}

			gameEnded = CheckGameOver(&player1, &player2, &tcpClient1, &tcpClient2, &playersConnected);

			sf::IpAddress ipAddr;
			unsigned short port;
			if (udpSocket.receive(packet, ipAddr, port) == sf::Socket::Done) {
				if (packet.getDataSize() > 20) {
					PacketData recvData;
					packet >> recvData.playerNumber >> recvData.spritePosX >> recvData.spritePosY >> recvData.bulletPosX >> recvData.bulletPosY >> recvData.rotationAngle;
					packet.clear();
					if (ipAddr == player1.ipAddress && port == player1.port) {
						sf::Packet sendPacket;
						recvData.mHealth = player1.currentData.mHealth;
						recvData.oHealth = player2.currentData.mHealth;
						player1.currentData = recvData;
						PacketData sendData = recvData;
						sendData.mHealth = player2.currentData.mHealth;
						sendData.oHealth = player1.currentData.mHealth;
						sendPacket << sendData.playerNumber << sendData.spritePosX << sendData.spritePosY << sendData.bulletPosX << sendData.bulletPosY << sendData.rotationAngle << sendData.mHealth << sendData.oHealth;
						udpSocket.send(sendPacket, player2.ipAddress, player2.port);
						sendPacket.clear();
					} else if (ipAddr == player2.ipAddress && port == player2.port) {
						sf::Packet sendPacket;
						recvData.mHealth = player2.currentData.mHealth;
						recvData.oHealth = player1.currentData.mHealth;
						player2.currentData = recvData;
						PacketData sendData = recvData;
						sendData.mHealth = player1.currentData.mHealth;
						sendData.oHealth = player2.currentData.mHealth;
						sendPacket << sendData.playerNumber << sendData.spritePosX << sendData.spritePosY << sendData.bulletPosX << sendData.bulletPosY << sendData.rotationAngle << sendData.mHealth << sendData.oHealth;;
						udpSocket.send(sendPacket, player1.ipAddress, player1.port);
						sendPacket.clear();
					}
				} else {
					std::cout << packet.getDataSize();
					packet.clear();
					if (ipAddr == player1.ipAddress && port == player1.port) {
						player2.currentData.mHealth -= 20;
						std::cout << player2.currentData.mHealth << std::endl;
					} else if (ipAddr == player2.ipAddress && port == player2.port) {
						player1.currentData.mHealth -= 20;
						std::cout << player1.currentData.mHealth << std::endl;
					}
				}
			}
		} else if (playersConnected != 0 && !gameEnded) {
			std::cout << "Someone disconnected";
		} else if (playersConnected != 0 && gameEnded) {
			std::cout << "Someone died";
		}
	}
	return 0;
}

//3538