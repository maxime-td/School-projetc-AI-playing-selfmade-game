#ifndef AFFICHE_H
#define AFFICHE_H

/**
 * @brief Trace un disque
 * @param renderer Le renderer où tracer le disque
 * @param center_x La coordonnée x du centre du disque
 * @param center_y La coordonnée y du centre du disque
 * @param radius Le rayon du disque
*/
void draw_disk(SDL_Renderer* renderer, int center_x, int center_y, int radius);

/**
 * @brief Dessine un graphe à l'aide d'un rendu SDL.
 * @param renderer Le rendu SDL utilisé pour afficher le graphe.
 * @param graph Le pointeur vers le graphe à dessiner.
 * @param n Le nombre de sommets dans le tableau.
 */
void draw_graph(SDL_Renderer* renderer, sommet_t** tab, int n);

void affiche(sommet_t ** tab, int n);


#endif // AFFICHE_H