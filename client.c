#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 1234
#define MAXDATASIZE 100

char sendbuf[1024];
char recvbuf[1024];
char name[100];
int fd;
int board[9];

void usage(){
    printf("add a new user\t\t input: 1 new_user_name\n");
    printf("Refresh all players\t input: 2\n");
    printf("Invite other player\t input: 3 Your_name Other's_name\n");
    printf("Logout\t\t\t input: exit\n\n");
}

char alp(int a){
    if (a==0)
        return ' ';
    else if (a==1)
        return 'O';
    else 
        return 'X';
}

void print_board(int *board){
    printf("┌─────┬─────┬─────┐        ┌─────┬─────┬─────┐\n");
    printf("│     │     │     │        │     │     │     │\n");
    printf("│  0  │  1  │  2  │        │  %c  │  %c  │  %c  │\n", alp(board[0]),alp(board[1]),alp(board[2]));
    printf("│     │     │     │        │     │     │     │\n");
    printf("├─────┼─────┼─────┤        ├─────┼─────┼─────┤\n");
    printf("│     │     │     │        │     │     │     │\n");
    printf("│  3  │  4  │  5  │        │  %c  │  %c  │  %c  │\n", alp(board[3]),alp(board[4]),alp(board[5]));
    printf("│     │     │     │        │     │     │     │\n");
    printf("├─────┼─────┼─────┤        ├─────┼─────┼─────┤\n");
    printf("│     │     │     │        │     │     │     │\n");
    printf("│  6  │  7  │  8  │        │  %c  │  %c  │  %c  │\n", alp(board[6]),alp(board[7]),alp(board[8]));
    printf("│     │     │     │        │     │     │     │\n");
    printf("└─────┴─────┴─────┘        └─────┴─────┴─────┘\n");
}

int choose_user_turn(int *board){
    int i=0;
    int inviter=0, invitee=0;
    for(i=0; i<9; i++){
        if(board[i] == 1){
            inviter++;
        }
        else if(board[i] == 2){
            invitee++;
        }
    }
    if(inviter > invitee)
        return 2;
    else
        return 1;
}

// modify chess board, and fill "sendbuf" with package format.
void write_on_board(int *board, int location){
    print_board(board);
    int user_choice = choose_user_turn(board);
    // Record which location on board is selected by inviter.
    // int location;
    // scanf("%d", &location);
    board[location] = user_choice;
    sprintf(sendbuf, "7  %d %d %d %d %d %d %d %d %d\n", board[0], \
        board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8]);
}



// Only handle message from server to client.
void pthread_recv(void* ptr)
{
    int instruction;
    while(1)
    {
        memset(sendbuf,0,sizeof(sendbuf));
        instruction = 0;
        // recvbuf is filled by server's fd.
        if ((recv(fd,recvbuf,MAXDATASIZE,0)) == -1)
        {
            printf("recv() error\n");
            exit(1);
        }
        sscanf (recvbuf,"%d",&instruction);
        switch (instruction)
        {
            case 2: {
                printf("%s\n", &recvbuf[2]); // Print the message behind the instruction.
                break;
            }
            case 4: {
                char inviter[100];
                sscanf(recvbuf,"%d %s",&instruction, inviter);
                printf("%s\n", &recvbuf[2]); // Print the message behind the instruction.
                printf("If accept, input:5 yes %s\n", inviter);
                printf("If not, input:5 no %s\n", inviter);
                break;
            }
            case 6: {
                printf("Game Start!\n");
                printf("Blank space is 0\n");
                printf("Inviter is player 1\n");
                printf("Invitee is player 2\n");
                printf("Inviter go first.\n");
                printf("Please input:-0~-8\n");
                print_board(board);
                break;
            }
            case 8: {
                // int board[9];
                char msg[100];
                sscanf (recvbuf,"%d %d%d%d%d%d%d%d%d%d %s",&instruction, \
                    &board[0],&board[1],&board[2],&board[3],&board[4],&board[5],&board[6], \
                        &board[7],&board[8], msg);
                print_board(board);
                printf("%s\n", msg);
                printf("Please input:-0~8\n");
                break;
            }
            
            default:
                //printf("Input Format error!\n");
                break;
        }

        memset(recvbuf,0,sizeof(recvbuf));
    }
}



int main(int argc, char *argv[])
{
    int  numbytes;
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in server;


    if (argc !=2)
    {
        printf("Usage: %s <IP Address>\n",argv[0]);
        exit(1);
    }


    if ((he=gethostbyname(argv[1]))==NULL)
    {
        printf("gethostbyname() error\n");
        exit(1);
    }

    if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        printf("socket() error\n");
        exit(1);
    }

    bzero(&server,sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr *)he->h_addr);
    if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1)
    {
        printf("connect() error\n");
        exit(1);
    }

    // First, Add User.
    printf("connect success\n");
    char str[]=" have come in\n";
    printf("Please ENTER your user name：");
    fgets(name,sizeof(name),stdin);
    char package[100];
    strcat(package, "1 ");
    strcat(package, name);
    send(fd, package, (strlen(package)),0);

    // usage
    usage();

    // Only handle message from server to client. (Goto pthread_recv finction)
    pthread_t tid;
    pthread_create(&tid, NULL, (void*)pthread_recv, NULL);
    // Only handle message from client to server.
    while(1){
        memset(sendbuf,0,sizeof(sendbuf));
        fgets(sendbuf,sizeof(sendbuf),stdin);   // Input instructions
        int location;
        if(sendbuf[0] == '-'){
            sscanf(&sendbuf[1], "%d", &location);
            write_on_board(board, location);
        }

        // logout
        if(strcmp(sendbuf,"exit\n")==0){          
            memset(sendbuf,0,sizeof(sendbuf));
            printf("You have Quit.\n");
            return 0;
        }
        send(fd,sendbuf,(strlen(sendbuf)),0);   // Send instructions to server
    }
    pthread_join(tid,NULL);
    close(fd);
}
