#include "main.h"
constexpr int TCP_MESSAGE_SIZE = 7;
constexpr int DC_CHECK_INTERVAL_SECONDS = 2;

struct PacketData {
	int playerNumber = 0;
	float spritePosX = 0;
	float spritePosY = 0;
	float bulletPosX = 0;
	float bulletPosY = 0;
	float rotationAngle;
	int mHealth = 100;
	int oHealth = 100;
};

struct PlayerConnectionDetails {
	unsigned short port = 0;
	sf::IpAddress ipAddress;
	PacketData currentData;
};

enum NetworkEvent {
	PlayerOne = 111,
	PlayerTwo = 222,
	Win = 333,
	Lose = 444,
	PlayOn = 555,
	GameStart = 666,
	Hit = 777,
	Check = 888,
	Error = -111,
};

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

sf::Packet& operator <<(sf::Packet& packet, const PacketData& data) {
	return packet << data.playerNumber << data.spritePosX << data.spritePosY << data.bulletPosX << data.bulletPosY << data.rotationAngle << data.mHealth << data.oHealth;;
}

sf::Packet& operator >>(sf::Packet& packet, PacketData& data) {
	return packet >> data.playerNumber >> data.spritePosX >> data.spritePosY >> data.bulletPosX >> data.bulletPosY >> data.rotationAngle;
}

std::ostream& operator<<(std::ostream& os, const PacketData& data) {
	return os << data.playerNumber << data.spritePosX << data.spritePosY << data.bulletPosX << data.bulletPosY << data.rotationAngle << data.mHealth << data.oHealth;
}

int main() {

	PlayerConnectionDetails player1;
	PlayerConnectionDetails player2;

	int playersConnected = 0;
	int port = 0;

	bool gameBegun = false;
	bool gameEnded = false;

	std::string ip = {};
	std::string netEventData;
	char const* netEventMessage;

	sf::TcpListener listener;

	sf::TcpSocket tcpClient1;
	sf::TcpSocket tcpClient2;

	sf::UdpSocket udpSocket;

	sf::Packet packet;


	auto lastCheckTime = std::chrono::system_clock::now();

	udpSocket.setBlocking(false);

	parse_config_file("net_config.txt", ip, port);
	std::cout << "Server IP: " << ip << " Port: " << port << std::endl;
	if (listener.listen(port, ip) != sf::Socket::Done) {
		std::cerr << "Error listening on TCP port " << port << std::endl;
		char input = getchar();
		return 1;
	} else {
		std::cout << "TCP Listening with port " << port << std::endl;
	}

	if (udpSocket.bind(port) != sf::Socket::Done) {
		std::cerr << "Error binding UDP socket to port " << port << std::endl;
		char input = getchar();
		return 1;
	} else {
		std::cout << "UDP Listening with port " << port << std::endl;
	}


	while (1) {
		if (playersConnected < 2 && !gameBegun && !gameEnded) {
			std::cout << "Initialisation, accepting players. Current connected count: " << playersConnected << std::endl;
			if (tcpClient1.getLocalPort() == 0) {
				if (listener.accept(tcpClient1) != sf::Socket::Done) {
					std::cout << "Error accepting client 1.\n";
				} else {
					std::cout << "First client connected." << std::endl;
					player1.ipAddress = tcpClient1.getRemoteAddress();
					player1.port = tcpClient1.getRemotePort();
					char data[TCP_MESSAGE_SIZE];
					std::size_t received;

					if (tcpClient1.receive(data, TCP_MESSAGE_SIZE, received) != sf::Socket::Done) {
						std::cout << "Error receiving client 1's message.\n";
					} else {
						player1.currentData.mHealth = std::stoi(data);
						netEventData = std::to_string(NetworkEvent::PlayerOne);
						netEventMessage = netEventData.c_str();
						if (tcpClient1.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
							std::cerr << "Error sending data to Player 1." << std::endl;
						}
						playersConnected++;
					}
				}
			} else {
				if (listener.accept(tcpClient2) != sf::Socket::Done) {
					std::cout << "Error accepting client 2.\n";
				} else {
					std::cout << "Second client connected." << std::endl;
					player2.ipAddress = tcpClient2.getRemoteAddress();
					player2.port = tcpClient2.getRemotePort();
					char data[TCP_MESSAGE_SIZE];
					std::size_t received;

					if (tcpClient2.receive(data, TCP_MESSAGE_SIZE, received) != sf::Socket::Done) {
						std::cout << "Error receiving client 2's message.\n";
					} else {
						player2.currentData.mHealth = std::stoi(data);
						netEventData = std::to_string(NetworkEvent::PlayerTwo);
						netEventMessage = netEventData.c_str();
						if (tcpClient2.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
							std::cerr << "Error sending data to Player 2." << std::endl;
						}
						playersConnected++;
					}
				}
			}
			if (playersConnected == 2) {
				gameBegun = true;

				std::cout << "\nConnection details: \n";
				std::cout << "Player 1: \n";
				std::cout << "Ip: " << player1.ipAddress << std::endl;
				std::cout << "Port: " << player1.port << std::endl << std::endl << std::endl;
				std::cout << "Player 2: \n";
				std::cout << "Ip: " << player2.ipAddress << std::endl;
				std::cout << "Port: " << player2.port << std::endl;

				std::cout << "\n\nGame has begun.\n\n";

				netEventData = std::to_string(NetworkEvent::GameStart);
				netEventMessage = netEventData.c_str();
				if (tcpClient1.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 1." << std::endl;
				}

				if (tcpClient2.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 2." << std::endl;
				}
			}
		} else if (playersConnected == 2 && gameBegun && !gameEnded) {
			auto now = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = now - lastCheckTime;
			if (elapsed_seconds.count() > DC_CHECK_INTERVAL_SECONDS) {
				lastCheckTime = now;

				netEventData = std::to_string(NetworkEvent::Check);
				netEventMessage = netEventData.c_str();

				if (tcpClient1.send(netEventMessage, TCP_MESSAGE_SIZE) == sf::Socket::Disconnected) {
					netEventData = std::to_string(NetworkEvent::Win);
					netEventMessage = netEventData.c_str();
					if (tcpClient2.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
						std::cerr << "Error sending data to Player 2." << std::endl;
					}
					playersConnected = 0;
				}

				netEventData = std::to_string(NetworkEvent::Check);
				netEventMessage = netEventData.c_str();
				if (tcpClient2.send(netEventMessage, TCP_MESSAGE_SIZE) == sf::Socket::Disconnected) {
					netEventData = std::to_string(NetworkEvent::Win);
					netEventMessage = netEventData.c_str();
					if (tcpClient1.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
						std::cerr << "Error sending data to Player 1." << std::endl;
					}
					playersConnected = 0;
				}
			}

			if (player1.currentData.mHealth == 0) {
				gameEnded = true;
				netEventData = std::to_string(NetworkEvent::Lose);
				netEventMessage = netEventData.c_str();
				if (tcpClient1.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 1." << std::endl;
				}
				netEventData = std::to_string(NetworkEvent::Win);
				netEventMessage = netEventData.c_str();
				if (tcpClient2.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 2." << std::endl;
				}
				tcpClient1.disconnect();
				tcpClient2.disconnect();
				playersConnected = 0;
			} else if (player2.currentData.mHealth == 0) {
				gameEnded = true;
				netEventData = std::to_string(NetworkEvent::Lose);
				netEventMessage = netEventData.c_str(); 
				if (tcpClient2.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 2." << std::endl;
				}
				netEventData = std::to_string(NetworkEvent::Win);
				netEventMessage = netEventData.c_str();
				if (tcpClient1.send(netEventMessage, TCP_MESSAGE_SIZE) != sf::Socket::Done) {
					std::cerr << "Error sending data to Player 1." << std::endl;
				}
				tcpClient1.disconnect();
				tcpClient2.disconnect();
				playersConnected = 0;
			}

			sf::IpAddress ipAddr;
			unsigned short port;
			if (udpSocket.receive(packet, ipAddr, port) == sf::Socket::Done) {
				if (packet.getDataSize() > 20) {
					PacketData recvData;
					packet >> recvData;
					packet.clear();
					if (ipAddr == player1.ipAddress && port == player1.port) {
						sf::Packet sendPacket;
						recvData.mHealth = player1.currentData.mHealth;
						recvData.oHealth = player2.currentData.mHealth;
						player1.currentData = recvData;
						PacketData sendData = recvData;
						sendData.mHealth = player2.currentData.mHealth;
						sendData.oHealth = player1.currentData.mHealth;
						sendPacket << sendData;
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
						sendPacket << sendData;
						udpSocket.send(sendPacket, player1.ipAddress, player1.port);
						sendPacket.clear();
					}
				} else {
					packet.clear();
					if (ipAddr == player1.ipAddress && port == player1.port) {
						player2.currentData.mHealth -= 20;
					} else if (ipAddr == player2.ipAddress && port == player2.port) {
						player1.currentData.mHealth -= 20;
					}
				}
			}
		} else if (playersConnected != 2 && gameBegun && !gameEnded) {
			std::cout << "Someone disconnected. Resetting server. \n";
			tcpClient1.disconnect();
			tcpClient2.disconnect();
			playersConnected = 0;
			gameBegun = false;
			gameEnded = false;
		} else if (gameBegun && gameEnded) {
			std::cout << "Someone died. Game Over. Resetting server. \n";
			tcpClient1.disconnect();
			tcpClient2.disconnect();
			playersConnected = 0;
			gameBegun = false;
			gameEnded = false;
		}
	}
	return 0;
}