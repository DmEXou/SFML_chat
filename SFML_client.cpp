#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <mutex>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

using namespace std::string_literals;

void send(sf::TcpSocket* chat_socet) {
	while (true) {
		sf::Packet pack;
		std::string message;
		int flag = 0;

		std::getline(std::cin, message);
		
		if (message == ""s) continue;
		if (message[1] == 'n') {
			flag = 1;
		}
		if (message[1] == 'c'){
			flag = 2;
		}

		pack << flag << message;
		if (chat_socet->send(pack) != sf::Socket::Status::Done) {
			break;
		}
		if (flag == 2)
			break;
	}
}

void receive(sf::TcpSocket* chat_socet) {
	while (true) {
		sf::Packet pack;
		std::string message;
		if (chat_socet->receive(pack) != sf::Socket::Status::Done) {
			break;
		}
		pack >> message;
		std::cout << std::endl << message << std::endl;
	}
}

int main() {

	sf::TcpSocket socket_new_connect;
	//sf::IpAddress server_ip("127.0.0.1");
	sf::IpAddress server_ip("45.10.246.155");
	if (socket_new_connect.connect(server_ip, 3000, sf::seconds(5)) != sf::Socket::Status::Done) {
		std::cout << "Connect failed\n";
		return 0;
	}
	else {
		std::cout << "Connected!!! " << socket_new_connect.getLocalPort() << " " << socket_new_connect.getRemotePort() << std::endl;
	}
	//std::thread th_1(&send, &socket_new_connect);
	//std::thread th_2(&receive, &socket_new_connect);
	//th_1.detach();
	//th_2.join();
	sf::Thread th_send(&send, &socket_new_connect);
	sf::Thread th_receive(&receive, &socket_new_connect);
	th_send.launch();
	th_receive.launch();
	return 0;
}
