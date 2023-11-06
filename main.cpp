#include "main.h"

int main() {
	sf::TcpSocket socket;
	sf::Socket::Status status = socket.connect("192.168.0.5", 53000);
	if (status != sf::Socket::Done) {
		// error...
		std::cout << "Error 1";
	}
	sf::TcpListener listener;

	// bind the listener to a port
	if (listener.listen(53000) != sf::Socket::Done) {
		// error...
		std::cout << "Error 2";
	}
	sf::TcpSocket client;
	if (listener.accept(client) != sf::Socket::Done) {
		// error...
		std::cout << "Error 3";
	}
	char data[100];

	// TCP socket:
	if (socket.send(data, 100) != sf::Socket::Done) {
		// error...
		std::cout << "Error 4";
	}
	std::size_t received;

	// TCP socket:
	if (socket.receive(data, 100, received) != sf::Socket::Done) {
		// error...
		std::cout << "Error 5";
	}
	std::cout << "Received " << received << " bytes" << std::endl;

	return 0;
}
