/**************************************************************
* Class:  CSC-415-02 Fall 2022
* Name:Martin Salvatierra
* Student ID:920000611
* GitHub UserID:Salvatim007
* Project: Assignment 6 – Device Driver
*
* File: Salvatierra_Martin_HW6_main
*
* Description: Assignment 6 testing user side to talk to kernel about encrypted data
*
**************************************************************/
#include <stdio.h> 
#include <string.h> 
#include <sys/ioctl.h> 
#include <fcntl.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 

int main(int argc,char* argv[]){
    int fd,info;
    long numb1,numb2,numb3;
    // holds the user input
    char messageBuffer[100]; 
    //hold mutated kernel data
    char encryptedBuffer[100]; 
    //user data length
    int messageLength;
    //temp holds user input
    int intCmd;
    //encription on is 1,decrytpion is 0
    char encryptionYN = 'y';//
    //holds a number to mutate data
    int messageKey ;
    //to check faults
    int ioctlRet;
    int wRet,RRet;

    //0 is standard in, 1 std out, 2 is error, 3 norm
    //read and write permission
    fd = open("/dev/Salvatierra_Martin_HW6_main",O_RDWR);
    //succesful information for user
    printf("returned from open file: %d\n", fd);
    //check device driver opened successfully
    if(fd == -1){
        printf("device driver open error: -1 ERR\n");
        return -1;

    }else{
        printf("device open successfully: \n");

    }
        //Prompt user to decide to encrypt or decrypt data
        printf("****Welcome to encryption generator**** \n");
        printf("Do you wish to encrypt? Enter 1 \n");
        printf("Do you wish to decrypt? Enter 0 \n");
        scanf(" %d",&intCmd );

        //select the key to encrypt with
        printf("Good Choice \n");
        printf("Now Enter the Key which you wish to use \n");
        printf("Current key accepted [2- 20] \n");
        scanf(" %d",&messageKey );
        printf("\nDont forget this Key: %d\n", messageKey);

        //set state and make sure input acceptable
        if(intCmd == 1 || intCmd == 0){
            encryptionYN = intCmd;
            printf("encoding set to : %d \n", encryptionYN);
        }else{
            printf("state not accepted %d \n", intCmd);
           
        }

        //back door to encryption/decryption, flag and key pass
        //printf("encryptionYn: %d  Mesagekey: %d \n",encryptionYN,messageKey);
        ioctlRet = ioctl(fd, encryptionYN, &messageKey);
        //clear stdin
        fgetc(stdin);

        /**Set the user message*/
        printf("enter up to 100 characters\n");
        fgets(messageBuffer, 100, stdin);//puts a \n at end
        //printf("User Input length: [%s], storing in buffer\n", messageBuffer);

        messageLength = strlen(messageBuffer);// 
        //messageBuffer[messageLength - 1] = '\0';
        //using the file discriptor 

        /**write to the kernel buffer from user buffer*/
        wRet = write(fd, messageBuffer, messageLength);// including \0
        printf("wrote %d characters\n", messageLength);
        //printf("wrote = %d : %d\n", wRet, messageLength);

        /**Print out the Result of kernel change stored in user buffer now*/
        RRet = read(fd, encryptedBuffer,messageLength);
        printf(" your encrytion is: \n%s\n",encryptedBuffer);


    //finished with file
    close(fd);
    return 0;
}