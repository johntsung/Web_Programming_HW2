all:
	gcc server_v1.c -o server -lpthread
	gcc client_v2.c -o client -lpthread

clean:
	rm server
	rm client
