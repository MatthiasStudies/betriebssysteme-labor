#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    fork();
    printf("Hello!\n");
    sleep(1);
}