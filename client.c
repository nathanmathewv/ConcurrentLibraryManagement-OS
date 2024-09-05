#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "header.h"


int main() {
    int client_fd; //socket file descriptor
    struct sockaddr_in server_address;
    printf("Client started\n");
    printf("Waiting for server\n");

    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    //setting the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //connect to the server
    if(connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    int choice;
    //display admin user menu according to the user type
    printf("Welcome to the library management system\n");
    
    while(1) {
        char buffer[BUFFER_SIZE];
        printf("Enter your choice :\n");
        printf("0. Exit\n");
        printf("1. Admin\n");
        printf("2. User\n");
        scanf("%d", &choice);
        //send the choice to the server
        handleWriteError(write(client_fd, &choice, sizeof(int)));

        //response from server
        int res;
        handleReadError(read(client_fd, &res,sizeof(int)));

        if(res == 0)  exit(1);

        printf("Enter your user/admin ID\n");
        int userID;
        scanf("%d", &userID);
        handleWriteError(write(client_fd, &userID, sizeof(int)));

        printf("Enter your password\n");
        char password[100];
        scanf("%s", password);
        handleWriteError(write(client_fd, password, strlen(password)));

        memset(buffer, 0, sizeof(buffer));
        handleReadError(read(client_fd, buffer, sizeof(buffer)));

        if(strcmp(buffer, "User not found") == 0) {
            printf("User not found\n");
            continue;
        }
        else if(strcmp(buffer, "Admin not found") == 0) {
            printf("Admin not found\n");
            continue;
        }
        else if(strcmp(buffer, "User found") == 0) {
            printf("User found\n");
        }
        else if(strcmp(buffer, "Admin found") == 0) {
            printf("Admin found\n");
        }

        //Read the menu
        memset(buffer, 0, sizeof(buffer)); 
        handleReadError(read(client_fd, buffer, sizeof(buffer)));
        printf("%s", buffer);

        while(1){
            //read the option
            int option;
            scanf("%d", &option);
            handleWriteError(write(client_fd, &option, sizeof(int)));

            if(option == 0) {
                //exit the menu
                printf("Exiting the menu\n");
                close(client_fd);
                exit(1);
            }

            if(choice == 1) {
                //admin menu
                if(option == 1) {
                    //see all the books available
                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option == 2) {
                    //add a book
                    Book book;
                    int id, year, copies;
                    char title[100], author[100];
                    //get the book details
                    printf("Enter the Book title\n");
                    scanf("%s",title);
                    printf("Enter the author name\n");
                    scanf("%s",author);
                    printf("Enter the book id\n");
                    scanf("%d", &id);
                    printf("Enter the year of publication\n");
                    scanf("%d", &year);
                    printf("Enter the number of copies\n");
                    scanf("%d", &copies);
                    //fill the book details
                    book.bookID=id;
                    strcpy(book.title, title);
                    strcpy(book.author, author);
                    book.year=year;
                    book.copies=copies;
                    //send the book details to the server
                    handleWriteError(write(client_fd, &book, sizeof(Book)));
                    //read the response from the server
                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd,buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }            
                else if(option == 3) {
                    //remove a book
                    int id;
                    printf("Enter the book id\n");

                    scanf("%d", &id);
                    handleWriteError(write(client_fd, &id, sizeof(int)));

                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option == 4) {
                    //modify a book
                    int id, copies;
                    printf("Enter the book id\n");
                    scanf("%d", &id);
                    handleWriteError(write(client_fd, &id, sizeof(int)));

                    printf("Enter the new number of copies\n");
                    scanf("%d", &copies);
                    handleWriteError(write(client_fd, &copies, sizeof(int)));

                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer, sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option==5) {
                    //check the availability of a book
                    int id;
                    printf("Enter the book id\n");
                    scanf("%d", &id);
                    handleWriteError(write(client_fd, &id, sizeof(int)));
                    char res[BUFFER_SIZE];
                    handleReadError(read(client_fd,res,sizeof(res)));
                    printf("%s\n", res);
                }
                else if(option==6) {
                    //to see the users
                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option==7) {
                    //to add a user
                    int userID;
                    char name[100], password[100];

                    printf("Enter the user ID\n");
                    scanf("%d", &userID);
                    handleWriteError(write(client_fd, &userID, sizeof(int)));
                    printf("Enter the user name\n");
                    scanf("%s", name);
                    handleWriteError(write(client_fd, name, strlen(name)));
                    printf("Enter the password\n");
                    scanf("%s", password);
                    handleWriteError(write(client_fd, password, strlen(password)));

                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                    
                }
                else if(option==8) {
                    //to remove a user
                    int userID;
                    printf("Enter the user ID\n");
                    scanf("%d", &userID);
                    handleWriteError(write(client_fd, &userID, sizeof(int)));

                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option==9) {
                    //to see the menu again
                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else {
                    //invalid option
                    printf("Invalid option\n");
                }
            }
            else if(choice == 2) {
                //user menu
                if(option == 1) {
                    //see all the books borrowed
                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd,buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option == 2) {
                    //return a book
                    int id;
                    printf("Enter the book id\n");
                    scanf("%d", &id);
                    //send the book id to the server
                    handleWriteError(write(client_fd, &id, sizeof(int)));
                    //read the response from the server
                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option == 3) {
                    //borrow a book
                    int id;
                    printf("Enter the book id\n");
                    scanf("%d", &id);
                    handleWriteError(write(client_fd, &id, sizeof(int)));

                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd, buffer, sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option == 4) {
                    //check the availability of a book
                    int id;
                    printf("Enter the book id\n");
                    scanf("%d", &id);
                    handleWriteError(write(client_fd, &id, sizeof(int)));

                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd,buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else if(option == 5) {
                    //to see the menu again
                    memset(buffer, 0, sizeof(buffer));
                    handleReadError(read(client_fd,buffer,sizeof(buffer)));
                    printf("%s\n", buffer);
                }
                else {
                    //invalid option
                    printf("Invalid option\n");
                }
            }
            else {
                //invalid choice
                memset(buffer, 0, sizeof(buffer));
                handleReadError(read(client_fd, buffer, sizeof(buffer)));
                printf("%s\n", buffer);
            }
        
    }
    //close the socket
    close(client_fd);
    return 0; 
}
}
