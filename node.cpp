#include <map>
#include <cmath>
#include <mutex>
#include <message.cpp>
using namespace std;
volatile int barrier = 0;
mutex glb_lock;
class node
{
public:
    node();
    ~node();

    mutex modify_lock;
    mutex timer_lock;
};

// get_port(int id)
// get the port number for a node according to the node id
// input: id -- id of the node
// return: port of the node
string get_port(int id) {
    return to_string(id+10000);
}

// send_message(message & m)
// send message to the designated destination
// input: m -- message to be sent
// return: sock_fd -- the socket's file descriptor
int send_message(message & m) {

    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* TCP */

    s = getaddrinfo(NULL, get_port(m.dest).c_str(), &hints, &result);

    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }

    connect(sock_fd, result->ai_addr, result->ai_addrlen);

    write(sock_fd, &m, sizeof(message));
    increment_count(m.command);

    freeaddrinfo(result);
    return sock_fd;
}

// server_thread(int node_id)
// main execution for each node
// input: node_id -- identifier for the node
// return: none
void server_thread(int node_id, node & data)
{
    int connection_count=0;
    std::vector<int> descriptors;
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, get_port(node_id).c_str(), &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }

    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    if ( ::bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0 ) {
        perror("bind()");
        exit(1);
    }

    if ( listen(sock_fd, 10) != 0 ) {
        perror("listen()");
        exit(1);
    }

    freeaddrinfo(result);
    glb_lock.lock();
    barrier+=1;
    glb_lock.unlock();
    while(connection_count!=9)
    {
        int fd=accept(sock_fd, NULL, NULL);
        descriptors.push_back(fd);
        connection_count+=1;
    }
    while(1)
    {
        
    }
    close(sock_fd);
    return;
}

// client_thread(int node_id)
// main execution for each node
// input: node_id -- identifier for the node
// return: none
void client_thread(int node_id)
{
    while(barrier<9){}
    int connection_count=0;
    std::vector<int> descriptors;
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    while(connection_count!=9)
    {
        struct addrinfo hints, *result;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        s = getaddrinfo(NULL, get_port(connection_count).c_str(), &hints, &result);
        if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(1);
        }

        connect(sock_fd, result->ai_addr, result->ai_addrlen);

        int fd=connect(sock_fd, result->ai_addr, result->ai_addrlen);
        descriptors.push_back(fd);

        freeaddrinfo(result);
    }
    while(1)
    {

    }
    close(sock_fd);
    return;
}

// checker_thread(int node_id)
// main execution for each node
// input: node_id -- identifier for the node
// return: none
void timer_thread(int node_id)
{
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, get_port(node_id).c_str(), &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }

    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    if ( ::bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0 ) {
        perror("bind()");
        exit(1);
    }

    if ( listen(sock_fd, 10) != 0 ) {
        perror("listen()");
        exit(1);
    }

    freeaddrinfo(result);

    

    close(sock_fd);
    return;
}

void node_thread(int node_id)
{
    node data;
    thread server_thread(node_id,data);
}