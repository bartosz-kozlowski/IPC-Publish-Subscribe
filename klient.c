#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

enum msg_type {FAILURE = 1, SUCCESS ,LOGIN, LOGOUT, REGISTER_USER, VIEW_TOPICS, VIEW_SUBSCRIBED_TOPICS ,REGISTER_TO_TOPIC, REGISTER_NEW_TOPIC , SET_PID, SEND_NEW_MESSAGE, BLOCKING, NOTIFICATION, PRIOR_NO_IMPORTANT,PRIOR_LESS_IMPORTANT ,PRIOR_IMPORTANT};

struct msg {
    long type;
    char text[256];
    char shortText[32];
    char shortText2[32];
    int value;
    int value2;
    int value3;
};

int ipc_global = 0;
int ipc_user = 0;

void printRed(const char *mess) {
    printf("\033[1;31m");
    printf("%s", mess);
    printf("\033[0m");
}

void clearInputBuffer() {
    while (getchar() != '\n');
}

void registerToTopic(struct msg message,const int *ipc_global, const int *ipc_user, char name[32])
{
    message.type = VIEW_TOPICS;
    message.value = getpid();
    msgsnd(*ipc_global, &message, sizeof(message), 0);
    printf("---------------- TEMATY ----------------\n");
    int numOfTopics = 0;
    while(1)
    {
        if((msgrcv(*ipc_user, &message, sizeof(message), VIEW_TOPICS, IPC_NOWAIT))!=-1)
        {
            numOfTopics++;
            printf("%d. ", numOfTopics);
            printf("%s\n", message.shortText);
            if(message.value == 1)
            {
                break;
            }
        }
        if((msgrcv(*ipc_user, &message, sizeof(message), FAILURE, IPC_NOWAIT))!=-1)
        {
            printRed("Brak dostępnych tematów\n");
            printRed("Możesz dodać nowy w menu!\n");
            return;
            break;
        }

    }
    int option;
    int option2;
    int wanted_num = 1;
    printf("--------- WYBIERZ NUMER TEMATU ---------\n");
    while(1)
    {
        printf("0 - ANULUJ\n");


        if (scanf("%d", &option) == 1 && option <=numOfTopics){
        
        if(option == 0)
        {
            break;
        }

        printf("------ WYBIERZ RODZAJ SUBSKRYPCJI ------\n");
        printf("1 - TRWAŁA\n");
        printf("2 - PRZEJŚCIOWA\n");
        if(scanf("%d", &option2) == 1){
            if(option2 == 2)
            {
                while (1) {
                    printf("-- PODAJ LICZBĘ WIADOMOŚCI, JAKĄ CHCESZ OTRZYMAC --\n");

                    if (scanf("%d", &wanted_num) == 1 && wanted_num > 0) {
                        break;
                    } else {
                        printRed("Wprowadź poprawną liczbę!\n");
                        clearInputBuffer();
                    }
                }
            }
        }
        else{
            printRed("Nieprawidłowe dane wejściowe! Podaj liczbę!\n");
            clearInputBuffer();
        }


        if(option<=numOfTopics && numOfTopics >0 && (option2 == 1 || option2 == 2 ) && wanted_num > 0)
        {
            strcpy(message.shortText, name);
            message.type = REGISTER_TO_TOPIC;
            message.value = option - 1;
            if(option2 == 1)
            {
                 message.value2 = -1;
            }
            else{
                message.value2 = wanted_num;
            }
            message.value3 = getpid();
            msgsnd(*ipc_global, &message, sizeof(message), 0);

            if((msgrcv(*ipc_user, &message, sizeof(message), REGISTER_TO_TOPIC, 0))!=-1)
            {
                 printf("%s\n", message.text);
                 break;
            }

            break;
        }
        else{
            printRed("Niepoprawne dane\n");
        }

    }
    else{
        printRed("Nieprawidłowe dane wejściowe! Podaj nr tematu!\n");
        clearInputBuffer();
    }
    }
}

void registerNewTopic(struct msg message,const int *ipc_global, const int *ipc_user)
{
    while(1)
    {
        char name[32];
        //REJESTRACJA NOWEGO TEMATU
        printf("Podaj nazwę tematu: (max 32 znaki)\n");
        clearInputBuffer();
        scanf("%[^\n]%*c", name);
        strcpy(message.shortText, name);
        message.type = REGISTER_NEW_TOPIC;

        message.value = getpid();
        msgsnd(*ipc_global, &message, sizeof(message), 0);
        while(1)
        {
            if((msgrcv(*ipc_user, &message, sizeof(message), SUCCESS, IPC_NOWAIT))!=-1)
            {
                 printf("%s\n", message.shortText);
                 break;
            }
            if((msgrcv(*ipc_user, &message, sizeof(message), FAILURE, IPC_NOWAIT))!=-1)
            {
                 printf("%s\n", message.shortText);
                 break;
            }
        }
        if(message.type == SUCCESS)
        {
            break;
        }
    }
}
void sendNewMessage(struct msg message,const int *ipc_global, const int *ipc_user, char name[32])
{

        message.type = VIEW_SUBSCRIBED_TOPICS;
        strcpy(message.shortText, name);
        message.value = getpid();
        msgsnd(*ipc_global, &message, sizeof(message), 0);
        printf("------- WYBIERZ TEMAT WIADMOŚCI --------\n");
        int numOfTopics = 0;
        while(1)
        {
            if((msgrcv(*ipc_user, &message, sizeof(message), VIEW_SUBSCRIBED_TOPICS, IPC_NOWAIT))!=-1)
            {
                numOfTopics++;
                printf("%d. ", numOfTopics);
                printf("%s\n", message.shortText2);
                if(message.value == 1)
                {
                    break;
                }
            }
           if((msgrcv(*ipc_user, &message, sizeof(message), FAILURE, IPC_NOWAIT))!=-1)
            {
                printRed("Brak subskrybowanych tematów\n");
                return;
                break;
            }
        }
        printf("0. Anuluj\n");
        while(1)
        {
            int option = 0;
            if(scanf("%d", &option) == 1)
            {
                if(option <= numOfTopics &&  option>0)
                {
                    message.value2 = option;
                    break;
                }
                else if(option == 0)
                {
                    return;
                    break;
                }
                else{
                    printRed("Niepoprawna opcja!\n");
                }
            }
            else{
                printRed("Nieprawidłowe dane wejściowe! Podaj poprawną liczbę!\n");
                clearInputBuffer();
            }
        }
        printf("----- WYBIERZ PRIORYTET WIADMOŚCI ------\n");
        printf("1 - Pilna\n");
        printf("2 - Ważna\n");
        printf("3 - Nieistotna\n");
        while(1)
        {
            int option = 0;
            if(scanf("%d", &option) == 1){
            if(option == 1)
            {
                message.value3 = PRIOR_IMPORTANT;
                break;
            }
            else if(option == 2)
            {
                message.value3 = PRIOR_LESS_IMPORTANT;
                break;
            }
            else if(option == 3)
            {
                message.value3 = PRIOR_NO_IMPORTANT;
                break;
            }
            else{
                printRed("Niepoprawna opcja!\n");
            }
            }
            else{
                printRed("Nieprawidłowe dane wejściowe! Podaj liczbę!\n");
                clearInputBuffer();
            }
        }
        
        char text[256];
        printf("Podaj treść wiadomości: (max 256 znaków)\n");
        clearInputBuffer();
        scanf("%[^\n]%*c", text);

        message.value = getpid();
        strcpy(message.text, text);
        strcpy(message.shortText, name);
        message.type = SEND_NEW_MESSAGE;
        msgsnd(*ipc_global, &message, sizeof(message), 0);


        if((msgrcv(*ipc_user, &message, sizeof(message), SUCCESS, IPC_NOWAIT))!=-1)
        {
            printf("%s\n", message.shortText);
        }
        if((msgrcv(*ipc_user, &message, sizeof(message), FAILURE, IPC_NOWAIT))!=-1)
        {
            printf("%s\n", message.shortText);
        }


}
void blockUser(struct msg message,const int *ipc_global, const int *ipc_user, char name[32])
{
    while(1)
    {
        char blockedUserName[32];
        printf("Podaj nick użytkownika, którego chcesz zablokowac:\n");
        clearInputBuffer();
        scanf("%[^\n]%*c", blockedUserName);
        strcpy(message.shortText, name);
        strcpy(message.shortText2, blockedUserName);
        message.type = BLOCKING;
        message.value = getpid();
        msgsnd(*ipc_global, &message, sizeof(message), 0);

        if((msgrcv(*ipc_user, &message, sizeof(message), BLOCKING, 0))!=-1)
        {
            printf("%s\n", message.text);
            break;
        }
    }
}
void getMessageSynchro(struct msg message, const int *ipc_user)
{
    int received = 0;
    for(int i = PRIOR_IMPORTANT; i >=PRIOR_NO_IMPORTANT; i--)
        {
            if((msgrcv(*ipc_user, &message, sizeof(message), i, IPC_NOWAIT))!=-1)
            {
                printf("------------ NOWA WIADOMOŚĆ ------------\n");
                printf("Temat: %s\n", message.shortText2);
                printf("Autor: %s\n", message.shortText);
                printf("Treść: %s\n", message.text);
                printf("----------------------------------------\n");
                printf("\033[0m");
                received = 1;
                break;
            }
    }
    if(received == 0)
    {
         printRed("Brak dostępnych wiadmości!\n");
    }
}

int main(int argc, char* argv[]) {

    char name[32];
    char password[32];
    ipc_global = msgget(2137, 0666);
    struct msg message;
    ipc_user = msgget(getpid()*137, 0666 | IPC_CREAT);
    while(1)
    {
        //LOGOWANIE I REJESTRACJA
        printf("Podaj login: \n");
        scanf("%[^\n]%*c", name);
        printf("Podaj haslo: \n");
        scanf("%[^\n]%*c", password);
        strcpy(message.shortText, name);
        strcpy(message.shortText2, password);
        message.type = LOGIN;

        message.value = getpid();
        msgsnd(ipc_global, &message, sizeof(message), 0);

        while(1)
        {
            if((msgrcv(ipc_user, &message, sizeof(message), SUCCESS, IPC_NOWAIT))!=-1)
            {
                 printf("%s\n", message.shortText);
                 break;
            }
            if((msgrcv(ipc_user, &message, sizeof(message), FAILURE, IPC_NOWAIT))!=-1)
            {
                 printf("%s\n", message.shortText);
                 break;
            }
        }
        if(message.type == SUCCESS)
        {
            break;
        }
    }
    printf("----- WYBIERZ TEMAT DO SUBSKRYPCJI -----\n");
    registerToTopic(message, &ipc_global, &ipc_user, name); //Wybierz początkowy temat
    //WYBOR SPOSOBU ODBIERANIA WIADMOŚCI
    int option;
    int synchro = 0;
     while(1)
    {
        printf("--- WYBÓR SPOSOBU ODBIORU WIADMOŚCI ----\n");
        printf("1 - Asynchroniczny\n");
        printf("2 - Synchroniczny\n");
        printf("----------------------------------------\n");
        if(scanf("%d", &option) == 1){

            if(option == 1){
                break;
            }
            else if(option == 2){
                synchro = 1;
                break;
            }
            else{
                printRed("Niepoprawna opcja\n");
            }
        }
        else{
            printRed("Nieprawidłowe dane wejściowe! Podaj liczbę!\n");
            clearInputBuffer();
        }


    }
    int parentPid = getpid();
    if(option == 2)//Synchroniczny
    {
        strcpy(message.shortText, name);
        message.type = SET_PID;
        message.value = parentPid;
        msgsnd(ipc_global, &message, sizeof(message), 0);
    }

    if(fork()==0){
        if(option == 1)//Asynchroniczny
        {
            strcpy(message.shortText, name);
            message.type = SET_PID;
            message.value = getpid();
            msgsnd(ipc_global, &message, sizeof(message), 0);
            int ipc_user = msgget(getpid()*137, 0666 | IPC_CREAT);
            while(1)
            {
                sleep(0.1);
                for(int i = PRIOR_IMPORTANT; i >=PRIOR_NO_IMPORTANT; i--)
                {
                    if((msgrcv(ipc_user, &message, sizeof(message), i, IPC_NOWAIT))!=-1)
                    {
                        printf("------------ NOWA WIADOMOŚĆ ------------\n");
                        printf("Temat: %s\n", message.shortText2);
                        printf("Autor: %s\n", message.shortText);
                        printf("Treść: %s\n", message.text);
                        printf("----------------------------------------\n");
                    }
                }
                if((msgrcv(ipc_user, &message, sizeof(message), NOTIFICATION, IPC_NOWAIT))!=-1)
                {
                    printf("---------- NOWE POWIADOMIENIE ----------\n");
                    printf("%s\n", message.text);
                    printf("----------------------------------------\n");
                }
            }

        }
        else{//TYLKO ODBIERANIE POWIADOMIEŃ DLA SYNCHRONICZNEGO
            int ipc_user = msgget(parentPid*137, 0666 | IPC_CREAT);
            while(1)
            {
                sleep(0.1);
                if((msgrcv(ipc_user, &message, sizeof(message), NOTIFICATION, IPC_NOWAIT))!=-1)
                {
                    printf("---------- NOWE POWIADOMIENIE ----------\n");
                    printf("%s\n", message.text);
                    printf("----------------------------------------\n");
                }
            }

        }

    }

    while(1)
    {
        printf("----------------- MENU -----------------\n");
        printf("0 - Wyloguj się\n");
        printf("1 - Wyświetlenie tematów do subskrypcji\n");
        printf("2 - Rejestracja nowego typu wiadomości\n");
        printf("3 - Wyślij nową wiadomość\n");
        printf("4 - Zablokuj uzytkownika\n");
        if(synchro == 1)
        {
            printf("5 - Odbierz wiadomość synchronicznie\n");
        }
        printf("----------------------------------------\n");

            if(scanf("%d", &option) == 1){


                if(option == 0){
                    message.type = LOGOUT;
                    strcpy(message.shortText, name);
                    message.value = getpid();
                        msgsnd(ipc_global, &message, sizeof(message), 0);
                    if((msgrcv(ipc_user, &message, sizeof(message), LOGOUT, 0))!=-1)
                    {
                        printf("%s\n", message.shortText);
                    }
                    kill(0, SIGKILL);
                    msgctl(ipc_user, IPC_RMID, NULL);
                    exit(1);
                }
                else if(option == 1){
                    registerToTopic(message, &ipc_global, &ipc_user, name);
                }
                else if(option == 2){
                    registerNewTopic(message, &ipc_global, &ipc_user);
                }
                else if(option == 3){
                    sendNewMessage(message, &ipc_global, &ipc_user, name);
                }
                else if(option == 4){
                    blockUser(message, &ipc_global, &ipc_user, name);
                }
                else if(option == 5 && synchro == 1){
                    getMessageSynchro(message, &ipc_user);
                }
                else{
                    printRed("Niepoprawna opcja\n");
                }


            }
            else{
                printRed("Nieprawidłowe dane wejściowe! Podaj liczbę!\n");
                clearInputBuffer();
            }


    }




    return 0;
}
