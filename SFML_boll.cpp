#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

using namespace std::string_literals;

class Person {
public:
    Person(const unsigned short port, const std::string& name) noexcept
        :port_(port)
        ,name_(name)
    {
        if (listener_.listen(port_) != sf::Socket::Status::Done) {
            std::cout << "listen error\n";
            throw("listen error");
        }
        if (listener_.accept(socket_) != sf::Socket::Status::Done) {
            std::cout << "accept error\n";
            throw("accept error");
        }
        std::cout << "Person " << name << " crieted\n";
    }

    unsigned short get_port() const {
        return port_;
    }

    const std::string get_name() const {
        return name_;
    }

    sf::TcpListener* person_listener() {
        return &listener_;
    }

    sf::TcpSocket* person_socket() {
        return &socket_;
    }

    ~Person() {
        std::cout << "Person destroyed\n";
    }

private:
    unsigned short port_;
    std::string name_;
    sf::TcpListener listener_;
    sf::TcpSocket socket_;
};

class Clients_list {
    using my_tuple = std::tuple<std::string, sf::IpAddress, unsigned short> ;
public:
    Clients_list() {
        std::map<unsigned short, bool> tmp;
        for (unsigned short i = 2001; i <= 2100; ++i) {
            tmp[i] = false;
        }
        list_port_.swap(tmp);
    }

    void client_add() {

        sf::TcpSocket add_socket;
        sf::TcpListener add_listener;
        sf::Packet pack;

        if (add_listener.listen(2000) != sf::Socket::Status::Done) {
            std::cout << "error listen\n";
        }

        if (add_listener.accept(add_socket) != sf::Socket::Done) {
            std::cout << "error accept\n";
        }
        if (add_socket.receive(pack) != sf::Socket::Done) {
            std::cout << "error receive\n";
        }
        pack >> person_name_;

        person_IP_ = add_socket.getRemoteAddress();
        
        for (auto& k_val : list_port_) {
            if (k_val.second == false) {
                free_port_ = k_val.first;
                k_val.second = true;
                break;
            }
        }
        pack.clear();
        pack << free_port_;
        add_socket.send(pack);
        add_socket.disconnect();
    }

    my_tuple get_param_person() {
        return { person_name_, person_IP_, free_port_ };
    }

    std::vector<Person*>& access_arr_person() {
        return arr_person;
    }

private:
    std::map<unsigned short, bool> list_port_;
    std::string person_name_;
    sf::IpAddress person_IP_;
    unsigned short free_port_ = 0;
    std::vector<Person*> arr_person;
};

void add_new_chat_client(Clients_list& list) {
    while (true) {
        list.client_add();
        const auto [name, ip, port] = list.get_param_person();
        Person* person = new Person(port, name);
        list.access_arr_person().push_back(person);

    }
}

void send_and_receive_msg(Person* person, std::vector<Person*>& arr_person) {
    while (true) {
        
        sf::Packet pack;
        if (person->person_socket()->receive(pack) != sf::Socket::Status::Done) {//
            std::cout << person->get_name() << " disconected\n";
            //

            arr_person.erase(std::find(arr_person.begin(), arr_person.end(), person));
            delete(person);

            //
            break;
        }

        std::string msg;
        pack >> msg;
        for (auto person_ptr : arr_person) {
            pack.clear();
            std::string msg_re = person->get_name() + " - "s + msg;
            pack << msg_re;
            if (person_ptr != person) {
                person_ptr->person_socket()->send(pack);
            }
        }
        std::cout << person->get_name() << " - " << msg << std::endl;
    }
}

void chat(Clients_list& list) {
    //std::vector<std::thread*> thread_list;
    int counter = 0;
    while (true) {
        if ((list.access_arr_person().size() > 0) && (counter != list.access_arr_person().size())) {//
            counter = list.access_arr_person().size();
            std::thread* th = new std::thread(&send_and_receive_msg, list.access_arr_person().back(), std::ref(list.access_arr_person()));
            std::cout << "thread N " << th->get_id() << std::endl;
            th->detach();
        }
    }
}

int main() {

    //sf::Font font;
    //if (!font.loadFromFile("arial.ttf"))
    //    return EXIT_FAILURE;

    //const unsigned int vm_coordx = 1200, vm_coordy = 800;
    //sf::RenderWindow window(sf::VideoMode(vm_coordx, vm_coordy), L"Простой игровой ЧАТ");

    std::string message("test");

    Clients_list list;
    sf::Thread th(&add_new_chat_client, std::ref(list));
    th.launch();
    chat(std::ref(list));

    //while (window.isOpen()) {
    //    sf::Event event;
    //    while (window.pollEvent(event)) {
    //        if (event.type == sf::Event::Closed)
    //            window.close();
    //    }
    //    window.clear();
    //    sf::Text txt_name;
    //    txt_name.setFont(font);
    //    txt_name.setString(message);
    //    txt_name.setCharacterSize(30);
    //    txt_name.setPosition(200, 200);
    //    window.draw(txt_name);

    //    window.display();
    //}
    //return EXIT_SUCCESS;
    return 0;
}