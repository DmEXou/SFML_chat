﻿#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <list>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

using namespace std::string_literals;

class Person {
public:
    void set_person_name(const std::string& name) noexcept {
        _name = name;
    }

    std::string get_person_name() const noexcept {
        return _name;
    }

    void set_person_rem_port(const unsigned short port) noexcept {
        _remote_port = port;
    }

    unsigned short get_person_rem_port() const noexcept {
        return _remote_port;
    }

private:
    //float _x;
    //float _y;
    std::string _name;
    unsigned short _remote_port;
};

class Socket_list {
public:
    Socket_list() {
        if (_listener.listen(3000) != sf::Socket::Status::Done) {
            std::cout << "<<Castom Error>> Error listen listener in Socket_list\n"s;
            std::logic_error e("<<Castom Error>> Error listen listener in Socket_list");
            throw(&e);
        }
    }

    void add_new_client_rec() {
        while (true) {
            add_person();
        }
    }

    size_t get_size() const {
        return _socket_list.size();
    }

    std::list <std::unique_ptr<sf::TcpSocket>>& get_list() {
        return std::ref(_socket_list);
    }

    void servis_pack(int type, const std::string& str, const sf::TcpSocket& socket) {
        if (type == 1) { // switch
            std::string serv = str.substr(6, str.size() - 6);
            auto person_it = std::find_if(_person_list.begin(), _person_list.end(), [&socket](auto& unq_person) {
                return socket.getRemotePort() == unq_person.get()->get_person_rem_port();
                });
            person_it->get()->set_person_name(serv);
        }
    }

    void get_msg(sf::TcpSocket& socket) {
        sf::Packet pack;
        while (true) {
            auto port_disconnected = socket.getRemotePort();
            std::string tern_msg;
            if (socket.receive(pack) != sf::Socket::Status::Done) {
                auto list_soc_it = std::find_if(_socket_list.begin(), _socket_list.end(), [&socket](auto& un_p_socket) {
                    return un_p_socket.get()->getRemotePort() == socket.getRemotePort();
                    });
                auto list_pers_it = std::find_if(_person_list.begin(), _person_list.end(), [&socket](auto& un_p_person) {
                    return un_p_person.get()->get_person_rem_port() == socket.getRemotePort();
                    });
                if (list_soc_it != std::end(_socket_list) && list_pers_it != std::end(_person_list)) {
                    _person_list.erase(list_pers_it);
                    _socket_list.erase(list_soc_it);
                }
                else {
                    std::cout << "<<Castom Error>> Invalid erase attempt.\n";
                    std::logic_error e("<<Castom Error>> Invalid erase attempt.");
                    throw(e);
                }
                std::cout << port_disconnected << " disconnected\n";
                break;
            }
            int type_pack;
            pack >> type_pack;
            if (type_pack != 0) {
                std::string str;
                pack >> str;
                servis_pack(type_pack, str, socket);
            }
            else {
                auto person_it = std::find_if(_person_list.begin(), _person_list.end(), [&socket](auto& unq_person) {
                    return socket.getRemotePort() == unq_person.get()->get_person_rem_port();
                    });
                std::string tmp_msg;
                tern_msg += person_it->get()->get_person_name();
                pack >> tmp_msg;
                tern_msg += ": " + tmp_msg;
                pack.clear();
                pack << tern_msg;
                for (auto& an_socket : _socket_list) {
                    if ((an_socket.get()->getRemotePort() != socket.getRemotePort()) && (an_socket.get()->getRemotePort() != 0)) {
                        std::cout << "send " << an_socket.get()->getRemotePort() << std::endl;
                        an_socket.get()->send(pack);
                    }
                }
            }
            std::cout << tern_msg << std::endl;
            pack.clear();
        }
    }

private:
    void add_person() {
        std::unique_ptr<sf::TcpSocket> u_ptr = std::make_unique<sf::TcpSocket>();
        _socket_list.emplace_back(std::move(u_ptr));

        if (_listener.accept(*_socket_list.back().get()) != sf::Socket::Done) {
            std::cout << "Error accept listener in Socket_list\n"s;
        }
        std::cout << "list size = "s << get_size() << " LocalPort = "s << _socket_list.back().get()->getLocalPort()
            << " RemotePort = " << _socket_list.back().get()->getRemotePort() << std::endl;

        std::unique_ptr<Person> person = std::make_unique<Person>(); //Формирование Клиента
        person.get()->set_person_rem_port(_socket_list.back().get()->getRemotePort()); //Присваиваем RemotePort Клиенту
        person.get()->set_person_name(std::to_string(_socket_list.back().get()->getRemotePort())); //Присваеваем Имя Клиенту аналагично Порту
        _person_list.emplace_back(std::move(person));//Добовляем нового клиента в список клиентов.

        std::thread th_get_msg(&Socket_list::get_msg, this, std::ref(*_socket_list.back().get()));
        th_get_msg.detach();
    }

private:
    std::list <std::unique_ptr<Person>> _person_list;
    std::list <std::unique_ptr<sf::TcpSocket>> _socket_list;
    sf::TcpListener _listener;
};

int main() {
    Socket_list list;

    std::thread th_add_client(&Socket_list::add_new_client_rec, &list);
    th_add_client.detach();
    while (true) {
        std::string str;
        std::cin >> str;
        if (str == "quit"s) {
            return 0;
        }
    }
    return 0;
}