#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "graph.h"
#include "fourmi.h"
#include "OptiFloyd_Warshall.h"
#include <math.h>

/**
 * @brief Construit la matrice des distances minimales (pour les sommets non reliés, donne la distance du chemin optimal les reliant)
 * @param distTab le tableau des distances initiales (sera modifié)
 * @param n la taille du tableau
 */
void Floyd_Warshall(int **distTab, int n)
{
    for (int k = 0; k < n; k++)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (distTab[i][j] == -1)
                {
                    distTab[i][j] = 9999999;
                }

                if (distTab[i][k] != -1 && distTab[k][j] != -1 && distTab[i][k] + distTab[k][j] < distTab[i][j])
                {
                    distTab[i][j] = distTab[i][k] + distTab[k][j];
                }
            }
        }
    }
}

/**
 * @brief Créé une copie d'un tableau 2D d'entier
 * @param tab le tableau des entiers à copier
 * @param n la taille du tableau
 * @return un tableau 2D d'entiers
 */
int **copie_tab(int **tab, int n)
{
    int **new = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        new[i] = malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
        {
            new[i][j] = tab[i][j];
        }
    }
    return new;
}

/**
 * @brief Construit un cycle en selectionnant toujours le point le plus proche par lequel on n'est encore pas passé (distances calculé précédemment par Floyd_Warshall)
 * @param tabWarshall Tableau des distances de Floy-Warshall
 * @param distTab le tableau des distances initiales 
 * @param n la taille du tableau
 * @param indDep l'indice dans tabSommet du point de départ
 * @param sol l'adresse d'un entier qui prend la valeur de la solution de ce cycle
 * @return un tableau d'entier de taille n+1 décrivant le cycle optimal et sa longueur
 */
void cycle_Floyd_Warshall(int **tabWarshall, int **tabDist, int n, int indDep, int *sol)
{

    int indAct = indDep;
    int indNext;

    int DISTANCE = 0;
    int minDistVoisin = 0;

    int *tabValide = malloc(n * sizeof(int));
    for (int k = 0; k < n; k++)
    {
        tabValide[k] = 0;
    }
    int nbValide = 0;

    while (nbValide != n)
    {
        minDistVoisin = 999999;
        nbValide += 1;
        tabValide[indAct] = 1;
        indNext = -1;
        for (int i = 0; i < n; i++)
        {
            if (i != indAct && tabDist[indAct][i] > 0 && tabDist[indAct][i] < minDistVoisin && tabValide[i] == 0)
            {

                indNext = i;
                minDistVoisin = tabDist[indAct][i];
            }
        }
        if (indNext == -1)
        {
            for (int j = 0; j < n; j++)
            {
                if (j != indAct && tabWarshall[indAct][j] < minDistVoisin && tabValide[j] == 0)
                {

                    indNext = j;
                    minDistVoisin = tabWarshall[indAct][j];
                }
            }
        }
        if (indNext == -1)
        {
            break;
        }
        DISTANCE += minDistVoisin;
        indAct = indNext;
    }
    DISTANCE += tabWarshall[indDep][indAct];
    *sol = DISTANCE;
}

/**
 * @brief Applique l'algo de cyclage ci-dessus sur chaque premier point et sors le plus efficace
 * @param tabWarshall Tableau des distances de Floy-Warshall
 * @param distTab le tableau des distances initiales (sera modifié)
 * @param n la taille du tableau
 * @return la longueur du chemin optimal
 */
int multi_Start_Floyd_Warshall(int **tabWarshall, int **distTab, int n)
{
    //print_dist_tab(tabWarshall, &n);
    int min = 99999999;
    int sol;
    int tmp;
    for (int i = 0; i < n; i++)
    {
        cycle_Floyd_Warshall(tabWarshall, distTab, n, i, &sol);
        tmp = sol;
        if (min > tmp)
        {
            min = sol;
        }
    }
    return min;
}

void permute(int **tab, int n)
{
    int a = 0, b = 0;
    while (a == 0)
    {
        a = rand() % (n - 1);
        b = rand() % (n - 1);
    }

    int tmp = (*tab)[a];
    (*tab)[a] = (*tab)[b];
    (*tab)[b] = tmp;
}


/**
 * @brief calcul la taille d'un chemin passer en parametre avec la méthode de Floyd-Warshall
 * @param tabWarshall Tableau des distance de Floyd-Warshall
 * @param n nombre de sommets du graph (et dimension du tableau des distances)
 * @param tab le tableau d'entier d'ordre d'apparition des sommets (tab[0] = 1er sommet du cycle)
 * @return la taille du chemin
*/
int calcul_chemin_Floy_Warshall(int **tabWarshall, int n, int *tab)
{
    int chem = 0;
    for (int i = 0; i < n - 1; i++)
    {
        chem += tabWarshall[tab[i]][tab[i + 1]];
    }
    chem += tabWarshall[tab[0]][tab[n - 1]];
    return chem;
}

/**
 * @brief applique la méthode du recuit simulé 
 * @param tabWarshall Tableau des distance de Floyd-Warshall
 * @param n nombre de sommets du graph (et dimension du tableau des distances)
*/
int recuit_simule(int **tabWarshall, int n)
{
    int *tab = malloc(n * sizeof(int));
    int chem;
    int ecart;
    int p;
    for (int i = 0; i < n; i++)
    {
        tab[i] = i;
    }
    int *tabTemp = tab;
    chem = calcul_chemin_Floy_Warshall(tabWarshall, n, tab);
    float T = chem;
    float alpha = 0.9999;
    int cpt=0;
    
    while (T > 0.0001)
    {
        cpt++;
        chem = calcul_chemin_Floy_Warshall(tabWarshall, n, tab);
        p = rand() % 1000;
        permute(&tabTemp, n);

        ecart = chem - calcul_chemin_Floy_Warshall(tabWarshall, n, tabTemp);
        ;
        if (ecart < 0 || p < (int)exp(-ecart / T))
        {
            for (int i = 0; i < n; i++)
            {
                tab[i] = tabTemp[i];
            }
        }
        T = alpha * T;
    }

    return chem;
}
