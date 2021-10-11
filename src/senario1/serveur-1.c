#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

int main(int argc, char* argv[]) {

    // Détection d'un argument pour le numéro de port
    if (argc < 2){
        printf("Utilisation :\n./serveur-1 <port-serveur>\n");
        exit(0);
    }

    int port = atoi(argv[1]);
    char port_data[15][5] = {"8001","8002","8003","8004","8005","8006","8007","8008","8009","8010","8011","8012","8013","8014","8015"};

    if (port < 0 || port > 65535 ){
        printf("Please use a valide port\n");
        exit(0);
    }

    // paramétrage Socket
    int socket_init, socket_data;
    struct sockaddr_in servAddr, servAddr_data, clientAddr;
    int desc_value;
    //paramétrage message
    char buffer[1024];
    char message[30] = "SYN-ACK";


    // Création socket principale

    if((socket_init = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Erreur socket principale");
        exit(0);
    };



    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    if (bind(socket_init, (struct sockaddr *) &servAddr, sizeof(servAddr)) != 0) {
        perror("Error while binding");
        exit(0);
    }

    // Démarrage de la phase d'échange avec le client
    strcat(message,port_data[0]);
    while(1){
        socklen_t len = sizeof(clientAddr);
        //attente du premier SYN

        

        bzero(buffer, sizeof(buffer));
        if((desc_value = recvfrom(socket_init, buffer, sizeof(buffer), 0,(struct sockaddr*)&clientAddr, &len)) < 0){
            perror("Erreur recvfrom");
            exit(0);
        };


        if((desc_value = sendto(socket_init, (const char*)message, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr))) < 0){
                perror("Erreur sendto");
                exit(0);
            }

        bzero(buffer, sizeof(buffer));
        if((desc_value = recvfrom(socket_init, buffer, sizeof(buffer), 0,(struct sockaddr*)&clientAddr, &len)) < 0){
            perror("Erreur recvfrom");
            exit(0);
        };
        // Phase de connexion fini
        if (strncmp(buffer,"ACK",3)==0){
            //Création d'un nouveau processus pour le client

            pid_t child_pid;
            child_pid = fork ();
            if (child_pid==0){
            
                //Paramétrage du nouveau socket pour la DATA
                if((socket_data = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
                    perror("Erreur socket principale");
                    exit(0);
                }; 
                servAddr_data.sin_family = AF_INET;
                servAddr_data.sin_addr.s_addr = htonl(INADDR_ANY);
                servAddr_data.sin_port = htons(atoi(port_data[0]));

                if (bind(socket_data, (struct sockaddr *) &servAddr_data, sizeof(servAddr_data)) != 0) {
                    perror("Error while binding");
                    exit(0);
                }

                //attente du nom du fichier à envoyer au client
                bzero(buffer, sizeof(buffer));
                if((desc_value = recvfrom(socket_data, buffer, sizeof(buffer), 0,(struct sockaddr*)&clientAddr, &len)) < 0){
                    perror("Erreur recvfrom");
                    exit(0);
                };
                //Définition des variables pour l'envoie du fichier
                FILE* fichier = NULL;
                char chaine[1018];
                int taille_DATA = 1018;
                int seg_nb = 1;
                char name_fichier[30];
                struct timeval timeout;
                fd_set readfs_fils;
                FD_ZERO(&readfs_fils);
 
                //Méthode pour envoyer le fichier 
                memcpy(name_fichier,buffer,30);
                bzero(buffer, sizeof(buffer));
                fichier = fopen(name_fichier, "r");
                while(taille_DATA==1018){
                    sprintf(buffer,"%06d",seg_nb);
                    taille_DATA = fread(chaine,1,1018,fichier);
                    memcpy(buffer+6,chaine,taille_DATA);
                    if((desc_value = sendto(socket_data, (const char*)buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr))) < 0){
                            perror("Erreur sendto");
                            exit(0);
                    }
                    while(1){
                        timeout.tv_sec = 0;
                        timeout.tv_usec = 700000;
                        FD_SET(socket_data, &readfs_fils);
                        int ret;
                        if((ret = select(6, &readfs_fils, NULL, NULL, &timeout)) < 0) {
                            perror("select()");
                            exit(0);
                        }   
                        if(FD_ISSET(socket_data, &readfs_fils)){
                            if((desc_value = recvfrom(socket_data, buffer, sizeof(buffer), 0,(struct sockaddr*)&clientAddr, &len)) < 0){
                                perror("Erreur recvfrom");
                                exit(0);
                            };
                            printf("Message from UDP client: %s\n", buffer);
                            bzero(buffer, sizeof(buffer));
                            seg_nb++;
                            break;
                        }
                        else{
                        if((desc_value = sendto(socket_data, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr))) < 0){
                            perror("Erreur sendto");
                            exit(0);
                        }
                        printf("erreur de transmission renvoie du paquet : %d\n", seg_nb);
                        }
                    }
                    //printf("Nom du fichier %s\n",name_fichier);
                }
                char fin[3] = "FIN"; 
                if((desc_value = sendto(socket_data, (const char*)fin, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr))) < 0){
                  perror("Erreur sendto");
                  exit(0);
                }
                close(socket_data);
                close(socket_init);
                exit(0);
            }
        }
    }
}

