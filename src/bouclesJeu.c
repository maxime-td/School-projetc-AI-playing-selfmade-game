#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "graph.h"
#include "affiche.h"
#include "fourmi.h"
#include "OptiFloyd_Warshall.h"
#include "bouclesJeu.h"

void* thread_fourmi(FourmiArgs* args){
    args->result = multi_start_fourmi(args->matDist, args->n);
    return NULL;
}


void* thread_floyd(FloydWarshallArgs* args){
    args->result = multi_Start_Floyd_Warshall(args->tabWarshall, args->tabDist, args->n);
    return NULL;
}

/**
 * @brief Exécute la boucle de jeu  de graphe
 * @param tab Le tableau des sommets
 * @param n Le nombre de sommets
*/
void boucle_jeu_graphe(sommet_t** tab, int n) {
    pthread_t thread1, thread2;
    //Initialisation
    int i; //Incrément
    int nb_noeuds_chemin = 0; //Nombre de noeuds dans le chemin
    int r = R_NOEUD; //Rayon des sommets
    int n_s_graphe;
    int all[n];
    int ** warshallDist;
    int first = 1;
    FourmiArgs argsF;
    FloydWarshallArgs argsFl;

    int scoreFourmi, scoreFloyd, scoreBest, score;

    int ** distMat = dist_tab(tab, &n);

    int * chemin_joueur = (int*) malloc(sizeof(int)*MAX_PATH); //Chemin du joueur
    for(i = 0; i < n; i += 1) //Initialisation du chemin du joueur
        chemin_joueur[i] = -1;

    SDL_bool program_on = SDL_TRUE; //Booléen de boucle de jeu
    SDL_Event event;

    int x, y; //Position de la souris au moment du clic

    sommet_t** sous_graphe; //Sous-graphe chemin pour affichage

    warshallDist = copie_tab(distMat, n);
    Floyd_Warshall(warshallDist, n);

    argsFl.n           = n           ;
    argsFl.tabDist     = distMat     ;
    argsFl.tabWarshall = warshallDist;
    pthread_create(&thread2, NULL, (void * (*)(void *))thread_floyd, &argsFl);

    argsF.matDist = distMat;
    argsF.n       = n      ; 
    pthread_create(&thread1, NULL, (void * (*)(void *))thread_fourmi, &argsF);
    //scoreFourmi = multi_start_fourmi(distMat, n);
    int update = 1;
    int etat_graphe = 0; //0 = en jeu graphe, 1 = menu de fin, 2 = en jeu espace

    //Boucle de jeu
    while (program_on) {
        //Gestion des événements
        while (SDL_PollEvent(&event))
        {
            update = 1;
            switch (event.type)
            {
                
                //pour fermer la fenetre quand on clique sur la croix
                case SDL_QUIT:
                    program_on = SDL_FALSE;
                    break;
                
                //Détection des clics
                case SDL_MOUSEBUTTONDOWN:
                    switch(event.button.button){
                        case SDL_BUTTON_LEFT: //Si on a un clic gauche
                            
                            //Pour voir si on clique sur un noeud
                            for(i = 0; i < n; i+=1) { //On parcour tous les noeuds
                                //On regarde si le clic est dans un carré autour du noeud
                                SDL_GetMouseState(&x, &y);
                                if((x >= (tab[i]->x)-r) && (x <= (tab[i]->x)+r) && (y >= (tab[i]->y)-r) && (y <= (tab[i]->y)+r)) {
                                    if (nb_noeuds_chemin == 0 || tab[i]->voisins[chemin_joueur[nb_noeuds_chemin-1]]){
                                        chemin_joueur[nb_noeuds_chemin] = i; //On l'ajoute au chemin
                                        nb_noeuds_chemin += 1;
                                    }
                                }
                            }
                            break;
                        
                        case SDL_BUTTON_RIGHT: //Si on a un clic droit on enlève le dernier noeud
                            if(etat_graphe == 0) { //Si on est encore en jeu
                                if (nb_noeuds_chemin != 0) { //Si on a déjà sélectionné au moins un noeud
                                    chemin_joueur[nb_noeuds_chemin-1] = -1; //on met le dernier noeud du tableau à -1 valeur impossible
                                    nb_noeuds_chemin -= 1; //on retire 1 au nombre de noeud
                                }
                            }       
                            break;

                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym){
                        case SDLK_SPACE: //Si espace on remet le nombre de noeud à 0
                            if(etat_graphe == 0)
                                nb_noeuds_chemin = 0;
                            break;
                            
                        case SDLK_RETURN://Si entrer on verifie que la selection est valide (cycle complet) 
                                         //Si oui on passe dans l'etat de l'écran de fin de jeu (etat_graphe = 1)  
                        
                            for (int i = 0; i < n; i++){
                                all[i] = 0;
                            }

                            for (int i = 0; i < nb_noeuds_chemin; i++){
                                all[chemin_joueur[i]] = 1;
                            }

                            if (tout_noeud(all, n) && chemin_joueur[nb_noeuds_chemin-1] == chemin_joueur[0]){
                                etat_graphe += 1;
                            } 
                            
                            break;

                        default:
                            break;
                    }
                default:
                    break;
            }
        }

        if (update) {
            update = 0;
            clear_SDL(); //Clear la fenêtre (la remetre blanc)

            switch(etat_graphe) { 
                case 0: //etat_graphe = 0, etat jeu en cours
                    affiche(tab, n, 0, 0, 0, 255, 1);
                    sous_graphe = chemin_en_graphe(chemin_joueur, nb_noeuds_chemin, tab, n, &n_s_graphe);
                    affiche(sous_graphe, n_s_graphe, 255, 0, 0, 255, 0);

                    draw_path(tab, chemin_joueur, nb_noeuds_chemin);
                    draw_int(path_size(chemin_joueur, distMat, nb_noeuds_chemin));

                    break;            

                case 1: //etat_graphe = 1, etat écran de fin
                    if (first) {
                        first = 0;
                        pthread_join(thread1, NULL); 
                        pthread_join(thread2, NULL);
                        scoreFloyd = argsFl.result;
                        scoreFourmi = argsF.result;
                    }
                
                    score = path_size(chemin_joueur, distMat, nb_noeuds_chemin); //Score du joueur

                    scoreBest = scoreFourmi;
                    if (scoreFloyd < scoreFourmi) { //recherche quelle est le meilleur score obtenu entre les differents algo et le joueur
                        scoreBest = scoreFloyd;
                    }

                    if (scoreBest > score) {
                        scoreBest = score;
                    }
                    
                    afficheFin(score, scoreBest); //On affiche l'ecran de fin 

                    if (scoreBest == score) {
                        secret1();
                    }

                    break;
                
                default: //etat_graphe = 2
                    program_on = SDL_FALSE; //On ferme cette boucle
                    break;
            }

            render();//rendre les differents elements
        }
    }

    free(chemin_joueur);
}

/**
 * @brief Exécute la boucle de jeu dans l'espace
 * @param tab Le tableau des sommets
 * @param n Le nombre de sommets
*/
void boucle_jeu_espace(sommet_t** tab, int n) {
    printf("On est dans l'espace !!!! Wouhooouuuuu !!!!\n");
}

/**
 * @brief Exécute la boucle principale du programme
 * @param tab Le tableau des sommets
 * @param n Le nombre de sommets
*/
void boucle_jeu(sommet_t ** tab, int n) {
    init(tab, n); //Affichage du graphe
    
    boucle_jeu_graphe(tab, n);

    boucle_jeu_espace(tab, n);

    closeSDL();//free de tout les elements de SDL
}