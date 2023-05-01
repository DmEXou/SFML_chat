#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <cassert>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

void send(sf::TcpSocket* chat_socet) {
	sf::Packet pack;
	std::string message;
	while (true) {
		std::getline(std::cin, message);
		pack << message;
		if (chat_socet->send(pack) != sf::Socket::Status::Done) {
			break;
		}
		pack.clear();
		message.clear();
	}
}

void receive(sf::TcpSocket* chat_socet) {
	sf::Packet pack;
	std::string message;
	while (true) {
		if (chat_socet->receive(pack) != sf::Socket::Status::Done) {
			break;
		}
		pack >> message;
		std::cout << message <<std::endl;
		pack.clear();
		message.clear();
	}
}

int main() {
	std::string person_name;
	std::cout << "Enter Name = ";
	std::cin >> person_name;
	sf::IpAddress server_ip("192.168.0.124");
	sf::TcpSocket socket_new_connect;
	if (socket_new_connect.connect(server_ip, 2000) != sf::Socket::Status::Done) {
		std::cout << "Connect failed\n";
		return 0;
	}
	sf::Packet pack;
	pack << person_name;
	socket_new_connect.send(pack); // send name
	pack.clear();
	socket_new_connect.receive(pack);
	unsigned short free_port = 0;
	pack >> free_port; // rece port
	socket_new_connect.disconnect();

	sf::TcpSocket chat_socet;
	if (chat_socet.connect(server_ip, free_port, sf::seconds(5)) != sf::Socket::Status::Done) {
		std::cout << "Connect failed\n";
		return 0;
	}

	sf::Thread th_send(&send, &chat_socet);
	sf::Thread th_receive(&receive, &chat_socet);
	th_send.launch();
	th_receive.launch();

	//std::string message;
	//sf::Packet pack_a;
	//while (true) {
	//	std::getline(std::cin, message);
	//	pack_a << message;
	//	if (chat_socet.send(pack_a) != sf::Socket::Status::Done) {
	//		break;
	//	}

	//	pack_a.clear();
	//	message.clear();
	//}

	return 0;
}