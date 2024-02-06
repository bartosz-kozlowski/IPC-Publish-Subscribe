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

struct user {
    char name[32];
    char password[32];
    int pid_to_send;
    int ipc_user;
    int following;
    char blocked[99][32];
    int blocked_num;
};

struct sub {
    char name[32];
    int wanted_num;
};

struct topic {
    char name[32];
    struct sub subs[100];
    int numberOfSubs;
};


void log_in(struct user *registered_users, int *numberOfUsers,const int *maxUsers, struct msg message)
{
            int ipc_user = msgget(message.value*137, 0666);
            int userExists = -1;
            for(int i = 0; i < *maxUsers; i++)
            {
                if (strcmp(registered_users[i].name, message.shortText) == 0) {//LOGOWANIE

                    userExists = i;
                    break;
                }
            }

                if(userExists != -1) {//LOGOWANIE UZYTKOWNIKA
                    if(strcmp(registered_users[userExists].password, message.shortText2) == 0)
                    {
                        strcpy(message.shortText2, "");
                        strcpy(message.shortText, "Zalogowano");
                        registered_users[*numberOfUsers].ipc_user = ipc_user;
                        message.type = SUCCESS;
                        message.value = LOGIN;
                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }
                    else{
                        strcpy(message.shortText2, "");
                        strcpy(message.shortText, "Złe haslo");
                        message.type = FAILURE;
                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }
                }
                else{ //REJESTRACJA UZYTKOWNIKA
                    if(*numberOfUsers<*maxUsers){
                        strcpy(registered_users[*numberOfUsers].name, message.shortText);
                        strcpy(registered_users[*numberOfUsers].password, message.shortText2);
                        registered_users[*numberOfUsers].ipc_user = ipc_user;
                        registered_users[*numberOfUsers].following = 0;
                        registered_users[*numberOfUsers].blocked_num = 0;
                        (*numberOfUsers)++;

                        strcpy(message.shortText, "Utworzono konto");
                        message.type = SUCCESS;
                        message.value = REGISTER_USER;

                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }
                    else{
                        strcpy(message.shortText, "Przekroczono liczbę użytkowników");
                        message.type = FAILURE;
                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }

                }
}
void register_topic(struct topic *topics, int *numberOfTopics,const int *maxTopics, struct user *registered_users,const int *numberOfUsers, struct msg message)
{
            int ipc_user = msgget(message.value*137, 0666);
            int topicExists = -1;
            for(int i = 0; i < *numberOfTopics; i++)
            {
                if (strcmp(topics[i].name, message.shortText) == 0) {

                    topicExists = i;
                    break;
                }
            }

                if(topicExists != -1) {

                    strcpy(message.shortText, "Temat już istnieje!");
                    message.type = FAILURE;
                    msgsnd(ipc_user, &message, sizeof(message), 0);
                }
                else{ //REJESTRACJA TEMATU
                    if(*numberOfTopics<*maxTopics){
                        strcpy(topics[*numberOfTopics].name, message.shortText);
                        topics[*numberOfTopics].numberOfSubs = 0;
                        (*numberOfTopics)++;

                        for(int i = 0; i < *numberOfUsers; i++)
                        {
                            if (registered_users[i].pid_to_send != -1 && registered_users[i].ipc_user != ipc_user) {//WYSYLANIE POWIADOMIENIA DO ZALOGOWANYCH

                                int ipc_notif = msgget(registered_users[i].pid_to_send*137, 0666);
                                sprintf(message.text, "Utworzono nowy temat: %s", topics[(*numberOfTopics)-1].name);
                                message.type = NOTIFICATION;
                                msgsnd(ipc_notif, &message, sizeof(message), 0);
                            }
                        }

                        strcpy(message.shortText, "Utworzono temat");
                        message.type = SUCCESS;
                        message.value = REGISTER_NEW_TOPIC;

                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }
                    else{
                        strcpy(message.shortText, "Przekroczono liczbę tematów");
                        message.type = FAILURE;
                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }

                }
}

void blocking(struct user *registered_users,const int *numberOfUsers, struct msg message)
{
            int ipc_user = msgget(message.value*137, 0666);
            int isBlockedExists =  0;
             for(int i = 0; i < *numberOfUsers; i++)
            {
                if (strcmp(registered_users[i].name, message.shortText2) == 0) {
                    isBlockedExists =  1;
                }
            }
            if(isBlockedExists == 1)
            {
                   for(int i = 0; i < *numberOfUsers; i++)
                    {
                        if (strcmp(registered_users[i].name, message.shortText) == 0) {

                            int blockedNum = registered_users[i].blocked_num;
                            int alreadyBlocked = 0;
                            if(blockedNum<(*numberOfUsers)-1){
                                  for(int j = 0; j < blockedNum; j++)
                                    {
                                        if (strcmp(registered_users[i].blocked[j], message.shortText2) == 0) {
                                            alreadyBlocked = 1;
                                            break;
                                        }
                                    }
                                if(alreadyBlocked == 0)
                                {
                                    strcpy(registered_users[i].blocked[blockedNum], message.shortText2);
                                    registered_users[i].blocked_num++;
                                    sprintf(message.text, "Zablokowano użytkownika: %s", message.shortText2);
                                    message.type = BLOCKING;
                                }
                                else{
                                    sprintf(message.text, "Blokujesz już tego użytkownika: %s", message.shortText2);
                                    message.type = BLOCKING;
                                }

                            }
                            else{
                                strcpy(message.text, "Blokujesz juz wszystkich uzytkownikow");
                                message.type = BLOCKING;
                            }

                            break;
                        }
                    }

            }
            else{
                strcpy(message.text, "Brak takiego użytkownika");
                message.type = BLOCKING;
            }
            msgsnd(ipc_user, &message, sizeof(message), 0);
}

void print_topics(struct topic *topics,const int *numberOfTopics, struct msg message)
{
            int ipc_user = msgget(message.value*137, 0666);
            if(*numberOfTopics!=0)
            {
                for(int i = 0; i < *numberOfTopics; i++)
                {
                    usleep(200000);
                    strcpy(message.shortText, topics[i].name);
                    message.type = VIEW_TOPICS;
                    if(i!=*numberOfTopics-1 && *numberOfTopics != 0){
                        message.value = 0;
                    }
                    else
                    {
                        message.value = 1; //Ostatni temat
                    }
                    msgsnd(ipc_user, &message, sizeof(message), 0);

                }
            }
            else{
                message.type = FAILURE;
                msgsnd(ipc_user, &message, sizeof(message), 0);
            }
}

void print_subscribed_topics(struct topic *topics,const int *numberOfTopics, struct user *registered_users,const int *numberOfUsers, struct msg message)
{
            int ipc_user = msgget(message.value*137, 0666);
            int howManySubscribed;
            for(int i = 0; i < *numberOfUsers; i++)
            {
                if (strcmp(registered_users[i].name, message.shortText) == 0) {
                        howManySubscribed = registered_users[i].following;
                        break;
                }
            }

            int tempCounter = 0;
            if(howManySubscribed!= 0)
            {
                for(int i = 0; i < *numberOfTopics; i++)
                {
                    for(int j = 0; j < topics[i].numberOfSubs; j++)
                    {
                        if (strcmp(topics[i].subs[j].name, message.shortText) == 0) {
                            usleep(200000);
                            strcpy(message.shortText2, topics[i].name);
                            message.type = VIEW_SUBSCRIBED_TOPICS;
                            if(tempCounter!=howManySubscribed-1 && howManySubscribed != 0){
                                message.value = 0;
                            }
                            else
                            {
                                message.value = 1; //Ostatni temat
                            }
                            msgsnd(ipc_user, &message, sizeof(message), 0);
                            tempCounter++;
                        }
                    }
                }
            }
            else{
                message.type = FAILURE;
                msgsnd(ipc_user, &message, sizeof(message), 0);
            }
}

void receive_and_send(struct topic *topics,const int *numberOfTopics, struct user *registered_users,const int *numberOfUsers, struct msg message)
{
            int count = 0;
            int topicToSendNum = -1;
            char senderName[32];
            strcpy(senderName, message.shortText);
            for(int i = 0; i<*numberOfTopics; i++) //Znajdywanie tematu wiadmosci (uztykownik podaje numer z listy SWOICH KTORE SUBSKRYBUJE)
            {
                for(int j = 0; j<topics[i].numberOfSubs; j++)
                {
                    if(strcmp(topics[i].subs[j].name, senderName) == 0)
                    {
                        count++;
                       if(count == message.value2)
                       {
                           topicToSendNum = i;
                           break;
                       }
                    }
                }
                if(topicToSendNum != -1)
                {
                    break;
                }
            }
            for(int i = 0; i<topics[topicToSendNum].numberOfSubs; i++) //Wysylanie
            {
                for(int j = 0; j<*numberOfUsers; j++)
                {
                    if(strcmp(registered_users[j].name, topics[topicToSendNum].subs[i].name) == 0)
                    {
                        int receiverBlockingSender = 0;
                        for(int k = 0; k<registered_users[j].blocked_num; k++) //Sprawdzanie czy user nie blokuje
                        {
                            if(strcmp(registered_users[j].blocked[k], senderName) == 0)
                            {
                                receiverBlockingSender = 1;
                                break;
                            }
                        }
                        if(receiverBlockingSender == 0 && registered_users[j].pid_to_send != -1 && topics[topicToSendNum].subs[i].wanted_num != 0
                            && (strcmp(registered_users[j].name, senderName) != 0) )//Jezeli nie blokuje + Zalogowany + przejsciowa
                        {
                            strcpy(message.shortText, senderName);
                            strcpy(message.shortText2, topics[topicToSendNum].name);
                            message.type = message.value3;
                            int ipc_toSend = msgget(registered_users[j].pid_to_send*137, 0666);
                            msgsnd(ipc_toSend, &message, sizeof(message), 0);
                            if(topics[topicToSendNum].subs[i].wanted_num > 0)
                            {
                                topics[topicToSendNum].subs[i].wanted_num--;
                            }
                        }
                        break;
                    }
                }
            }

}

void subscribe_topic(struct topic *topics, struct user *registered_users,const int *numberOfUsers, struct msg message)
{
                int ipc_user = msgget(message.value3*137, 0666);
                int numberOfSubs = topics[message.value].numberOfSubs;
                if(numberOfSubs<100)
                {
                    int checkIfSubscribed = 0;
                    for(int i = 0; i<numberOfSubs; i++)
                    {
                        if(strcmp(topics[message.value].subs[i].name, message.shortText) == 0)
                        {
                            checkIfSubscribed = 1;
                            break;
                        }
                    }
                    if(checkIfSubscribed == 0)
                    {
                        strcpy(topics[message.value].subs[numberOfSubs].name, message.shortText);
                        topics[message.value].subs[numberOfSubs].wanted_num = message.value2;
                        topics[message.value].numberOfSubs++;
                        for (int i = 0; i < *numberOfUsers; i++) {
                            if (strcmp(registered_users[i].name, message.shortText) == 0) {
                                registered_users[i].following++;
                                break;
                            }
                        }
                        sprintf(message.text, "Zasubskrybowano temat: %s", topics[message.value].name);
                        message.type = REGISTER_TO_TOPIC;
                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }
                    else{
                        strcpy(message.text, "Już subskrybujesz ten temat!");
                        message.type = REGISTER_TO_TOPIC;
                        msgsnd(ipc_user, &message, sizeof(message), 0);
                    }
                }
                else
                {
                    strcpy(message.text, "Błąd");
                    message.type = REGISTER_TO_TOPIC;
                    msgsnd(ipc_user, &message, sizeof(message), 0);
                }
}


void logout_user(struct user *registered_users,const int *numberOfUsers, struct msg message)
{
     for(int i = 0; i < *numberOfUsers; i++)
            {
                if (strcmp(registered_users[i].name, message.shortText) == 0) {
                    registered_users[i].pid_to_send = -1;
                    int ipc_user = msgget(message.value*137, 0666);
                    strcpy(message.shortText, "Wylogowano pomyślnie\n");
                    message.type = LOGOUT;
                    msgsnd(ipc_user, &message, sizeof(message), 0);
                    break;
                }
            }
}

int main(int argc, char* argv[]) {

    int ipc_global = msgget(2137, 0666 | IPC_CREAT);
    struct msg message;
    int maxUsers = 100;
    int maxTopics = 32;

    struct user registered_users[maxUsers];
    struct topic topics[maxTopics];

    for (int i = 0; i < maxUsers; i++) {
        registered_users[i].pid_to_send = -1; //wylogowany uzytkownik ma wartosc -1
    }

    int numberOfUsers = 0;
    int numberOfTopics = 0;

    while(1)
    {
        // ------------------------- LOGOWANIE I REJESRACJA --------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), LOGIN, IPC_NOWAIT))!=-1)
        {
            log_in(registered_users, &numberOfUsers, &maxUsers, message);
        }
        // ------------------------- USTAWIENIE PID DO WYSYLANIA WIADMOSCI --------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), SET_PID, IPC_NOWAIT))!=-1)
        {
            for(int i = 0; i < numberOfUsers; i++)
            {
                if (strcmp(registered_users[i].name, message.shortText) == 0) {
                    registered_users[i].pid_to_send = message.value;
                    break;
                }
            }
        }
        // ------------------------- WYŚWIETLANIE TEMATÓW --------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), VIEW_TOPICS, IPC_NOWAIT))!=-1)
        {
             print_topics(topics, &numberOfTopics, message);

        }
        // ------------------------- WYŚWIETLANIE SUBSKRYBOWANYCH TEMATÓW --------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), VIEW_SUBSCRIBED_TOPICS, IPC_NOWAIT))!=-1)
        {
            print_subscribed_topics(topics, &numberOfTopics, registered_users, &numberOfUsers, message);
        }
        // ------------------------- ODBIERANIE I WYSYLANIE NOWEJ WIADMOSCI --------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), SEND_NEW_MESSAGE, IPC_NOWAIT))!=-1)
        {
            receive_and_send(topics, &numberOfTopics, registered_users, &numberOfUsers, message);
        }
        // ------------------------- REJESTRACJA DO TEMATU --------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), REGISTER_TO_TOPIC, IPC_NOWAIT))!=-1)
        {
            subscribe_topic(topics, registered_users, &numberOfUsers, message);
        }
         // ------------------------- REJESTRACJA NOWEGO TEMATU--------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), REGISTER_NEW_TOPIC, IPC_NOWAIT))!=-1)
        {
           register_topic(topics, &numberOfTopics, &maxTopics, registered_users, &numberOfUsers, message);
        }
          // ------------------------- BLOKOWANIE UZYTKOWNIKA--------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), BLOCKING, IPC_NOWAIT))!=-1)
        {
           blocking(registered_users, &numberOfUsers, message);
        }
         // ------------------------- WYLOGOWANIE USERA--------------------------------
        if((msgrcv(ipc_global, &message, sizeof(message), LOGOUT, IPC_NOWAIT))!=-1)
        {
            logout_user(registered_users, &numberOfUsers, message);
        }
    }
    msgctl(ipc_global, IPC_RMID, NULL);

    return 0;
}
