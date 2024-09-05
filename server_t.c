#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<fcntl.h>
#include "header.h"

#define MAX_USERS 30
pthread_mutex_t book_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t user_lock = PTHREAD_MUTEX_INITIALIZER;

#define ADMIN 1998 // Admin ID
#define ADMIN_PASSWORD "admin123" // Admin Password

struct flock lock(int fd, int type, int whence, int start, int len) {
    // Locking
    printf("Locking\n");
    struct flock lock;
    lock.l_type = type;
    lock.l_whence = whence;
    lock.l_start = start;
    lock.l_len = len;
    lock.l_pid = getpid();

    fcntl(fd, F_SETLKW, &lock);
    
    return lock;
}

void unlock(int fd, struct flock lock) {
    // Unlocking
    printf("Unlocking\n");
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
}

void displayUserMenu(int clientSock) {
    // Displaying the user menu
    char buffer[BUFFER_SIZE] = {"\nMENU\n0. Exit\n1. To see all the books borrowed\n2. To return a book\n3. To borrow a book\n4. To check the availability of the book\n5. To see the menu again\nPlease enter your choice :\n"};
    // Sending the menu to the client
    handleWriteError(write(clientSock, buffer, strlen(buffer)));
}

void displayAdminMenu(int clientSock) {
    // Displaying the admin menu
    char buffer[BUFFER_SIZE] = {"\nMENU\n0. Exit\n1. To see all the books\n2. To add a book\n3. To remove a book\n4. To modify a book\n5. To check the availability of the book\n6. To see all the users\n7. To add a user\n8. To remove a user\n9. To see the menu again\nPlease enter your choice :\n"};

    handleWriteError(write(clientSock, buffer, strlen(buffer)));
}

Book* returnBooks(){
    // Reading the books from the file
    int fd = open("books.dat", O_CREAT | O_RDWR, 0666);
    pthread_mutex_lock(&book_lock);
    struct flock lockv = lock(fd, F_RDLCK, SEEK_SET, 0, 0);
    lseek(fd, 0, SEEK_SET);
    Book *books = malloc(sizeof(Book) * MAX_BOOKS);
    for (int i = 0; i < MAX_BOOKS; i++) {
        books[i].bookID = 0;
    }
    int i = 0;
    printf("Reading books\n");
    Book *temp = malloc(sizeof(Book));
    while(read(fd, temp, sizeof(Book)) > 0){
        if(temp->bookID == 0) continue;
        books[i] = *temp;
        printf("Book ID: %d\n", books[i].bookID);
        i++;
    }
    unlock(fd, lockv);
    pthread_mutex_unlock(&book_lock);
    close(fd);
    return books;
}

User* returnUsers() {
    // Reading the users from the file
    int fd = open("users.dat", O_CREAT | O_RDWR, 0666);
    pthread_mutex_lock(&user_lock);
    struct flock lockv = lock(fd, F_RDLCK, SEEK_SET, 0, 0);
    lseek(fd, 0, SEEK_SET);
    User *users = malloc(sizeof(User) * MAX_USERS);
    printf("Reading users\n");
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].userID = 0;
    }
    int i = 0;
    User *temp = malloc(sizeof(User));
    while(read(fd, temp, sizeof(User)) > 0){
        if(temp->userID == 0) continue;
        users[i] = *temp;
        i++;
    }
    unlock(fd, lockv);
    pthread_mutex_unlock(&user_lock);
    close(fd);
    return users;
}

void checkBookAvailability(int clientSock) {
    // To check the availability of the book
    char buffer[BUFFER_SIZE] = {0};
    int bookID;

    handleReadError(read(clientSock, &bookID, sizeof(int)));

    int bookfd = open("books.dat", O_CREAT | O_RDWR, 0666);
    struct flock lockb = lock(bookfd, F_RDLCK, SEEK_SET, 0, 0);
    lseek(bookfd, 0, SEEK_SET);

    Book *book = malloc(sizeof(Book));
    int flag = 0;
    // Reading the books from the file
    for(int i = 0; i<MAX_BOOKS; i++){
        if(read(bookfd, book, sizeof(Book)) == 0) break;
        if(book->bookID == bookID){
            char buf[50];
            if(book->copies)    
                strcpy(buf,"Book available\n");

            else 
                strcpy(buf,"Book not available\n");
            // Sending the book details to the client
            sprintf(buffer, "Book ID: %d\tTitle: %s\tAuthor: %s\tYear: %d\tCopies Left: %d\n%s\n", 
            book->bookID, book->title, book->author, book->year, book->copies, buf);
            handleWriteError(write(clientSock, buffer, strlen(buffer)));
            // Book found
            flag = 1;
            break;
        }
    }
    // If book not found
    if(flag == 0)   handleWriteError(write(clientSock, "Book not found\n", 15));
    // Unlocking
    unlock(bookfd, lockb);
    close(bookfd);
}

void userFunctions(int userID, int clientSock) {
    // User functions

    while(1){
        int choice;
        handleReadError(read(clientSock, &choice, sizeof(int)));
        printf("Menu Option: %d\n", choice);

        if (choice == 0) {
            // Exiting
            handleWriteError(write(clientSock, "Exiting...\n", 17));
            return;
        }

        else if (choice == 1) {
            // To see all the books borrowed
            printf("Seeing all the books borrowed of a user\n");
            char buffer[BUFFER_SIZE] = "Books borrowed:\n";
            User *users = returnUsers();
            Book *books = returnBooks();

            for(int i = 0; i<MAX_USERS; i++){
                if(users[i].userID == userID){
                    for(int j = 0; j<MAX_BOOKS; j++){
                        if(users[i].borrowedBooks[j] != 0){
                            for(int k = 0; k<MAX_BOOKS; k++){
                                if(books[k].bookID == users[i].borrowedBooks[j]){
                                    char temp[500];
                                    sprintf(temp, "Book ID: %d\tTitle: %s\tAuthor: %s\tYear: %d\n", 
                                    books[k].bookID, books[k].title, books[k].author, books[k].year);
                                    strcat(buffer, temp);
                                }
                            }
                        }
                    }
                }
            }
            handleWriteError(write(clientSock, buffer, strlen(buffer)));
        }

        else if (choice == 2) {
            // To return a book
            printf("Returning a book\n");
            int bookID, flag = 0;
            handleReadError(read(clientSock, &bookID, sizeof(int)));
            User *users = returnUsers();

            int bookfd = open("books.dat", O_CREAT | O_RDWR, 0666);
            int userfd = open("users.dat", O_CREAT | O_RDWR, 0666);
            pthread_mutex_lock(&user_lock);
            struct flock lockb = lock(bookfd, F_WRLCK, SEEK_SET, 0, 0);
            struct flock locku = lock(userfd, F_WRLCK, SEEK_SET, 0, 0);

            for(int i = 0; i<MAX_USERS; i++){
                if(users[i].userID == userID){
                    for(int j = 0; j<MAX_BOOKS; j++){
                        if(users[i].borrowedBooks[j] == bookID){
                            // Book found
                                for(int k=j; k<MAX_BOOKS-1; k++){
                                    // Shifting the borrowed books
                                users[i].borrowedBooks[k] = users[i].borrowedBooks[k+1];
                            }
                            // Decreasing the number of borrowed books
                            lseek(userfd, i*sizeof(User), SEEK_SET);
                            handleWriteError(write(userfd, &users[i], sizeof(User)));
                            // Increasing the number of copies of the book
                            Book *book = malloc(sizeof(Book));
                            for(int k = 0; k<MAX_BOOKS; k++){
                                if(read(bookfd, book, sizeof(Book)) == 0) break;
                                if(book->bookID == bookID){
                                    book->copies++;
                                    lseek(bookfd, k*sizeof(Book), SEEK_SET);
                                    handleWriteError(write(bookfd, book, sizeof(Book)));
                                    break;
                                }
                            }
                            // Sending the message to the client
                            handleWriteError(write(clientSock, "Book returned\n", 14));
                            printf("Book returned\n");
                            flag = 1;
                            break;
                        }
                    }
                }
            }
            // If book not found
            if(flag == 0){
                handleWriteError(write(clientSock, "Book not found\n", 15));
            }
            unlock(bookfd, lockb);
            unlock(userfd, locku);
            pthread_mutex_unlock(&user_lock);
            close(bookfd);
            close(userfd);
        }

        else if (choice == 3) {
            // To borrow a book
            printf("Borrowing a book\n");
            int bookID;
            handleReadError(read(clientSock, &bookID, sizeof(int)));
            User *users = returnUsers();

            int bookfd = open("books.dat", O_CREAT | O_RDWR, 0666);
            int userfd = open("users.dat", O_CREAT | O_RDWR, 0666);
            pthread_mutex_lock(&user_lock);
            struct flock lockb = lock(bookfd, F_WRLCK, SEEK_SET, 0, 0);
            struct flock locku = lock(userfd, F_WRLCK, SEEK_SET, 0, 0);

            for(int i = 0; i<MAX_USERS; i++){
                if(users[i].userID == userID){
                    for(int j = 0; j<MAX_BOOKS; j++){
                        if(users[i].borrowedBooks[j]==0){
                            Book *book = malloc(sizeof(Book));
                            // Searching for the book
                            for(int k = 0; k<MAX_BOOKS; k++){
                                // If book not found
                                if(read(bookfd, book, sizeof(Book)) == 0) break;
                                // If book found
                                if(book->bookID == bookID){
                                    // If book already borrowed
                                    if(book->copies == 0){
                                        // If book not available
                                        handleWriteError(write(clientSock, "Book not available\n", 19));
                                        break;
                                    }
                                    // If book available
                                    book->copies--;
                                    lseek(bookfd, k*sizeof(Book), SEEK_SET);
                                    handleWriteError(write(bookfd, book, sizeof(Book)));
                                    // Borrowing the book
                                    users[i].borrowedBooks[j] = bookID;
                                    lseek(userfd, i*sizeof(User), SEEK_SET);
                                    handleWriteError(write(userfd, &users[i], sizeof(User)));
                                    // Sending the message to the client
                                    handleWriteError(write(clientSock, "Book borrowed\n", 14));
                                    printf("Book borrowed\n");
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            unlock(bookfd, lockb);
            unlock(userfd, locku);
            pthread_mutex_unlock(&user_lock);
            close(bookfd);
            close(userfd);
        }
        
        else if (choice == 4) {
            // To check the availability of the book
            printf("Checking the availability of the book\n");
            checkBookAvailability(clientSock);
        }

        else if (choice == 5) {
            // To see the menu again
            printf("Displaying user menu\n");
            displayUserMenu(clientSock);
        }
    }  
}

void adminFunctions(int adminID, int clientSock) {
    // Admin Functions

    while(1){
        int choice;
        handleReadError(read(clientSock, &choice, sizeof(int)));
        printf("Menu Option: %d\n", choice);

        if (choice == 0) {
            // To exit
            printf("Exiting...\n");
            return;
        }

        else if (choice == 1) {
            // To see all the books
            printf("Seeing all the books\n");
            Book *books = returnBooks(); //Updation problem, not concurrent
            char buffer[BUFFER_SIZE] = "Books available:\n";
            for(int i = 0; i<MAX_BOOKS; i++){
                if(books[i].bookID > 0){
                    char temp[500];
                    printf("Got Book\n");
                    sprintf(temp, "Book ID: %d\tTitle: %s\tAuthor: %s\tYear: %d\tCopies Left: %d\n", 
                    books[i].bookID, books[i].title, books[i].author, books[i].year, books[i].copies);
                    strcat(buffer,temp);
                }
            }
            // Sending the message to the client
            handleWriteError(write(clientSock, buffer, strlen(buffer)));
        }

        else if (choice == 2) {
            // To add a book
            printf("Adding a book\n");
            Book *book = malloc(sizeof(Book));
            handleReadError(read(clientSock, book, sizeof(Book)));

            int bookfd = open("books.dat", O_CREAT | O_RDWR, 0666);
            pthread_mutex_lock(&book_lock);
            struct flock lockb = lock(bookfd, F_WRLCK, SEEK_SET, 0, 0);
            lseek(bookfd, 0, SEEK_END);
            handleWriteError(write(bookfd, book, sizeof(Book)));
            unlock(bookfd, lockb);
            pthread_mutex_unlock(&book_lock);
            close(bookfd);

            handleWriteError(write(clientSock, "Book added\n", 11));
            printf("Book added\n");
        }

        else if(choice == 3) {
            // To remove a book
            printf("Removing a book\n");
            int bookID;
            handleReadError(read(clientSock, &bookID, sizeof(int)));
            
            Book *books = returnBooks();
            int bookfd = open("books.dat", O_CREAT | O_RDWR, 0666);
            struct flock lockb = lock(bookfd, F_WRLCK, SEEK_SET, 0, 0);

            Book *book = malloc(sizeof(Book));
            int flag = 0;

            for(int i = 0; i<MAX_BOOKS; i++){
                if(read(bookfd, book, sizeof(Book)) == 0) break;
                if(book->bookID == bookID) {
                    book->bookID = 0;
                    lseek(bookfd, i*sizeof(Book), SEEK_SET);
                    for(int j = i; j<MAX_BOOKS-1; j++){
                        handleWriteError(write(bookfd, &books[j+1], sizeof(Book)));
                    }
                    flag = 1;
                    handleWriteError(write(clientSock, "Book removed\n", 13));
                    printf("Book removed\n");
                    break;
                }
            }

            if(flag == 0) {
                handleWriteError(write(clientSock, "Book not found\n", 15));
            }
            unlock(bookfd, lockb);
            close(bookfd);
        }

        else if(choice == 4) {
            // To modify a book
            printf("Modifying a book\n");
            int bookID, copies;
            handleReadError(read(clientSock, &bookID, sizeof(int)));
            handleReadError(read(clientSock, &copies, sizeof(int)));

            int bookfd = open("books.dat", O_CREAT | O_RDWR, 0666);
            struct flock lockb = lock(bookfd, F_WRLCK, SEEK_SET, 0, 0);

            Book *book = malloc(sizeof(Book));
            int flag = 0;

            for(int i = 0; i<MAX_BOOKS; i++){
                if(read(bookfd, book, sizeof(Book)) == 0) break;
                if(book->bookID == bookID) {
                    book->copies = copies;
                    lseek(bookfd, i*sizeof(Book), SEEK_SET);
                    handleWriteError(write(bookfd, book, sizeof(Book)));
                    handleWriteError(write(clientSock, "Book modified\n", 14));
                    printf("Book modified\n");
                    flag = 1;
                    break;
                }
            }
            if(flag == 0){
                handleWriteError(write(clientSock, "Book not found\n", 15));
            }
            unlock(bookfd, lockb);
            close(bookfd);
        }

        else if(choice == 5) {
            // To check the availability of the book
            printf("Checking the availability of the book\n");
            checkBookAvailability(clientSock);
        }
        else if(choice == 6) {
            //to see all the users
            printf("Seeing all the users\n");
            User *users = returnUsers(); //Updation problem, not concurrent
            char buffer[BUFFER_SIZE] = "Users available:\n";
            for(int i = 0; i<MAX_USERS; i++){
                if(users[i].userID > 0){
                    char temp[500];
                    printf("Got User\n");
                    sprintf(temp, "User ID: %d\tName: %s\tPassword: %s\n", 
                    users[i].userID, users[i].name, users[i].password);
                    strcat(buffer,temp);
                }
            }
            handleWriteError(write(clientSock, buffer, strlen(buffer)));
        }
        else if(choice == 7) {
            // To add a user
            printf("Adding a user\n");
            User *user = malloc(sizeof(User));
            handleReadError(read(clientSock, &user->userID, sizeof(user->userID)));
            handleReadError(read(clientSock, &user->name, sizeof(user->name)));
            handleReadError(read(clientSock, &user->password, sizeof(user->password)));

            int userfd = open("users.dat", O_CREAT | O_RDWR, 0666);
            pthread_mutex_lock(&user_lock);
            struct flock locku = lock(userfd, F_WRLCK, SEEK_SET, 0, 0);
            lseek(userfd, 0, SEEK_END);
            handleWriteError(write(userfd, user, sizeof(User)));
            unlock(userfd, locku);
            pthread_mutex_unlock(&user_lock);
            close(userfd);

            handleWriteError(write(clientSock, "User added\n", 11));
            printf("User added\n");
        }

        else if(choice == 8) {
            // To remove a user
            printf("Removing a user\n");
            int userID;
            handleReadError(read(clientSock, &userID, sizeof(int)));

            User *users = returnUsers();

            int userfd = open("users.dat", O_CREAT | O_RDWR, 0666);
            pthread_mutex_lock(&user_lock);
            struct flock locku = lock(userfd, F_WRLCK, SEEK_SET, 0, 0);

            User *user = malloc(sizeof(User));
            int flag = 0;

            for(int i = 0; i<MAX_USERS; i++){
                if(read(userfd, user, sizeof(User)) == 0) break;
                if(user->userID == userID) {
                    user->userID = 0;
                    lseek(userfd, i*sizeof(User), SEEK_SET);
                    for(int j = i; j<MAX_USERS-1; j++){
                        handleWriteError(write(userfd, &users[j+1], sizeof(User)));
                    }
                    handleWriteError(write(clientSock, "User removed\n", 13));
                    printf("User removed\n");
                    flag = 1;
                    break;
                }
            }

            if(flag == 0) {
                handleWriteError(write(clientSock, "User not found\n", 15));
            }
            unlock(userfd, locku);
            pthread_mutex_unlock(&user_lock);
            close(userfd);
        }

        else if(choice == 9) {
            // To display the admin menu again
            printf("Displaying admin menu\n");
            displayAdminMenu(clientSock);
        }
    }
}

void *handleClient(void *arg) {
    //thread function
    printf("Thread created\n");
    int clientSock = *(int *)arg;
    int choice;
    //flag to check whether the client has entered the system
    int enterflag = 0;

    while(1 && clientSock != -1 && enterflag == 0) {
        printf("Waiting for client input...\n"); 
        handleReadError(read(clientSock, &choice, sizeof(int)));
        printf("Choice: %d\n", choice);
        handleWriteError(write(clientSock, &choice, sizeof(int)));
        if (choice == 1) {
            // Admin login
            int adminID;
            handleReadError(read(clientSock, &adminID, sizeof(int)));
            char buffer[BUFFER_SIZE] = {0};
            handleReadError(read(clientSock, buffer, sizeof(buffer)));
            char password[100];
            strcpy(password, buffer);
            
            if(adminID == ADMIN && strcmp(password, ADMIN_PASSWORD) == 0){
                handleWriteError(write(clientSock, "Admin found", 12));
                printf("Admin found\n");
                displayAdminMenu(clientSock);
                adminFunctions(adminID, clientSock);
                enterflag = 1;
            }
            else{
                handleWriteError(write(clientSock, "Admin not found", 16));
            }
        }

        else if (choice == 2) {
            // User login
            int userID;
            handleReadError(read(clientSock, &userID, sizeof(int)));
            char buffer[BUFFER_SIZE] = {0};
            handleReadError(read(clientSock, buffer, sizeof(buffer)));
            char password[100];
            strcpy(password, buffer);
            
            int userfd = open("users.dat", O_CREAT | O_RDWR, 0666);
            struct flock locku = lock(userfd, F_RDLCK, SEEK_SET, 0, 0);
            lseek(userfd, 0, SEEK_SET);
            User *user = malloc(sizeof(User));
            int flag = 0;
            printf("Reading users\n");
            for(int i = 0; i<MAX_USERS; i++) {
                if(read(userfd, user, sizeof(User)) == 0)  {
                    write(clientSock, "User not found", strlen("User not found")); 
                    break;
                }
                if(user->userID == userID && strcmp(user->password, password) == 0){
                    handleWriteError(write(clientSock, "User found", strlen("User found")));
                    flag = 1;
                    break;
                }
            }

            unlock(userfd, locku);
            close(userfd);

            if(flag) {
                printf("User found\n");
                displayUserMenu(clientSock);
                userFunctions(userID, clientSock);
                enterflag = 1;
            }
            else{
                printf("User not found\n");
            }
        }
        else if (choice == 0) {
            // Exit
            printf("Exiting...\n");
            break;
        }
    }

    close(clientSock);
    pthread_exit(NULL);
}

int main() {
    // Socket descriptor for server and client
    int serv;  
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create the server socket
    if ((serv = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Set the server socket options
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // Bind the server socket
    if (bind(serv, (struct sockaddr *)&address, sizeof(address)) < 0)  {
        perror("Failed to bind");
        exit(EXIT_FAILURE);
    }
    // Listen for incoming connections
    if (listen(serv, MAX_USERS) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }
    // Array to hold client threads
    pthread_t client_threads[MAX_USERS];  
    int clients = 0;

    while (1) {
        // Accept a client connection
        int clientSock;
        printf("Waiting for a client to connect...\n");
        if ((clientSock = accept(serv, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept failed");
        }
        // Create a new thread to handle the client
        if (clientSock != -1) {  
            printf("Client connected\n");
            // Pass the client socket to the thread function
            int temp = clientSock;
            pthread_create(&client_threads[clients], NULL, handleClient, &temp);  // Create a new thread to handle the client  
            clients++;
            // Wait for all client threads to finish before accepting more connections
            if (clients >= MAX_USERS) {
                for (int i = 0; i < clients; i++) {   // Wait for all client threads to finish before accepting more connections
                    printf("Waiting for client thread %d to finish...\n", i);
                    pthread_join(client_threads[i], NULL);
                }
                clients = 0;
            }
        }
    }

    return 0;
}