all:
	g++ SFML_client.cpp -o client -std=c++17 -lsfml-system -lsfml-network -lpthread
