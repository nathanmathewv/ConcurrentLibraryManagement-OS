#define MAX_BOOKS 100
#define BUFFER_SIZE 1000000
#define PORT 3949

typedef struct {
    int bookID;
    char title[100];
    char author[100];
    int year;
    int copies;
} Book;

typedef struct {
    int userID;
    char name[100];
    char password[100];
    int borrowedBooks[MAX_BOOKS];
} User;

void handleReadError(int res){
    if(res < 0){
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
}

void handleWriteError(int res){
    if(res < 0){
        perror("Write failed");
        exit(EXIT_FAILURE);
    }
}