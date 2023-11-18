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
	int mHealth;
	int oHealth;
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


bool CheckPointInCircle(float cx, float cy, float px, float py, float r) {
	return (pow((px - cx), 2) + pow((py - cy), 2)) <= pow(r, 2);
}


void CheckBulletCollision(PacketData* player1, PacketData* player2) {

	/*if (player1->bulletPosX == 10000 && player1->bulletPosY == 10000) {
		player1->canDamage = true;
	}

	if (player2->bulletPosX == 10000 && player2->bulletPosY == 10000) {
		player2->canDamage = true;
	}

	if (player1->canDamage) {
		if (CheckPointInCircle(player2->spritePosX, player2->spritePosY, player1->bulletPosX, player1->bulletPosY, 20)) {
			player1->canDamage = false;
			player2->health -= 20;
		}
	}
	if (player2->canDamage) {
		if (CheckPointInCircle(player1->spritePosX, player1->spritePosY, player2->bulletPosX, player2->bulletPosY, 20)) {
			player2->canDamage = false;
			player1->health -= 20;
		}
	}*/
}

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
				char data[4];
				std::size_t received;

				if (tcpClient1.receive(data, 4, received) != sf::Socket::Done) {
					// error...
				}
				player1.currentData.mHealth = std::stoi(data);
				if (tcpClient1.send("0", 2) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 1." << std::endl;
				}
				playersConnected++;
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
				}
				player2.currentData.mHealth = std::stoi(data);
				if (tcpClient2.send("1", 2) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 2." << std::endl;
				}
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

	char startData[2] = "0";

	if (tcpClient1.send(startData, 2) != sf::Socket::Done) {
		std::cerr << "Error sending data to Player 1." << std::endl;
	}

	if (tcpClient2.send(startData, 2) != sf::Socket::Done) {
		std::cerr << "Error sending data to Player 2." << std::endl;
	}

	std::cout << "\n\nGame has begun.\n" << std::endl;

	sf::Packet packet;
	while (1) {
		sf::IpAddress ipAddr;
		unsigned short port;
		//std::cout << player1.currentData.mHealth << " " << player2.currentData.mHealth << std::endl;
		if (udpSocket.receive(packet, ipAddr, port) == sf::Socket::Done) {
			if (packet.getDataSize() > 20) {
				PacketData recvData;
				packet >> recvData.playerNumber >> recvData.spritePosX >> recvData.spritePosY >> recvData.bulletPosX >> recvData.bulletPosY >> recvData.rotationAngle;
				//std::cout << data.playerNumber << " " << data.bulletPosX << " " << data.bulletPosY << " " << data.rotationAngle << " " << data.spritePosX << " " << data.spritePosY << std::endl;
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
				if (ipAddr == player1.ipAddress && port == player1.port) {
					player2.currentData.mHealth -= 20;
				} else if (ipAddr == player2.ipAddress && port == player2.port) {
					player1.currentData.mHealth -= 20;
				}
			}
		}
		//CheckBulletCollision(&player1.currentData, &player2.currentData);
	}
	return 0;
}