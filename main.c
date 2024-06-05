#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_NOEUDS 100

typedef struct NoeudAdj {
    int dest;
    int poids;
    struct NoeudAdj* suivant;
} NoeudAdj;

typedef struct ListeAdj {
    NoeudAdj* tete;
} ListeAdj;

typedef struct Graphe {
    int numNoeuds;
    ListeAdj* tableau;
} Graphe;

typedef struct NoeudTas {
    int v;
    int distance;
} NoeudTas;

typedef struct TasMin {
    int taille;
    int capacite;
    int* pos;
    NoeudTas** tableau;
} TasMin;

NoeudAdj* nouveauNoeudAdj(int dest, int poids) {
    NoeudAdj* nouveauNoeud = (NoeudAdj*) malloc(sizeof(NoeudAdj));
    nouveauNoeud->dest = dest;
    nouveauNoeud->poids = poids;
    nouveauNoeud->suivant = NULL;
    return nouveauNoeud;
}

Graphe* creerGraphe(int numNoeuds) {
    int i;
    Graphe* graphe = (Graphe*) malloc(sizeof(Graphe));
    graphe->numNoeuds = numNoeuds;
    graphe->tableau = (ListeAdj*) malloc(numNoeuds * sizeof(ListeAdj));
    for (i = 0; i < numNoeuds; ++i)
        graphe->tableau[i].tete = NULL;
    return graphe;
}

void ajouterArete(Graphe* graphe, int src, int dest, int poids) {
    NoeudAdj* nouveauNoeud = nouveauNoeudAdj(dest, poids);
    nouveauNoeud->suivant = graphe->tableau[src].tete;
    graphe->tableau[src].tete = nouveauNoeud;

    nouveauNoeud = nouveauNoeudAdj(src, poids);
    nouveauNoeud->suivant = graphe->tableau[dest].tete;
    graphe->tableau[dest].tete = nouveauNoeud;
}

NoeudTas* nouveauNoeudTas(int v, int distance) {
    NoeudTas* noeudTas = (NoeudTas*) malloc(sizeof(NoeudTas));
    noeudTas->v = v;
    noeudTas->distance = distance;
    return noeudTas;
}

TasMin* creerTasMin(int capacite) {
    TasMin* tasMin = (TasMin*) malloc(sizeof(TasMin));
    tasMin->pos = (int*) malloc(capacite * sizeof(int));
    tasMin->taille = 0;
    tasMin->capacite = capacite;
    tasMin->tableau = (NoeudTas**) malloc(capacite * sizeof(NoeudTas*));
    return tasMin;
}

void echangerNoeudTas(NoeudTas** a, NoeudTas** b) {
    NoeudTas* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(TasMin* tasMin, int idx) {
    int plusPetit, gauche, droite;
    plusPetit = idx;
    gauche = 2 * idx + 1;
    droite = 2 * idx + 2;

    if (gauche < tasMin->taille && tasMin->tableau[gauche]->distance < tasMin->tableau[plusPetit]->distance)
        plusPetit = gauche;

    if (droite < tasMin->taille && tasMin->tableau[droite]->distance < tasMin->tableau[plusPetit]->distance)
        plusPetit = droite;

    if (plusPetit != idx) {
        NoeudTas* plusPetitNoeud = tasMin->tableau[plusPetit];
        NoeudTas* idxNoeud = tasMin->tableau[idx];

        tasMin->pos[plusPetitNoeud->v] = idx;
        tasMin->pos[idxNoeud->v] = plusPetit;

        echangerNoeudTas(&tasMin->tableau[plusPetit], &tasMin->tableau[idx]);

        minHeapify(tasMin, plusPetit);
    }
}

bool estVide(TasMin* tasMin) {
    return tasMin->taille == 0;
}

NoeudTas* extraireMin(TasMin* tasMin) {
    if (estVide(tasMin))
        return NULL;

    NoeudTas* racine = tasMin->tableau[0];

    NoeudTas* dernierNoeud = tasMin->tableau[tasMin->taille - 1];
    tasMin->tableau[0] = dernierNoeud;

    tasMin->pos[racine->v] = tasMin->taille - 1;
    tasMin->pos[dernierNoeud->v] = 0;

    --tasMin->taille;
    minHeapify(tasMin, 0);

    return racine;
}

void diminuerCle(TasMin* tasMin, int v, int distance) {
    int i = tasMin->pos[v];
    tasMin->tableau[i]->distance = distance;

    while (i && tasMin->tableau[i]->distance < tasMin->tableau[(i - 1) / 2]->distance) {
        tasMin->pos[tasMin->tableau[i]->v] = (i - 1) / 2;
        tasMin->pos[tasMin->tableau[(i - 1) / 2]->v] = i;
        echangerNoeudTas(&tasMin->tableau[i], &tasMin->tableau[(i - 1) / 2]);

        i = (i - 1) / 2;
    }
}

bool estDansTasMin(TasMin* tasMin, int v) {
    if (tasMin->pos[v] < tasMin->taille)
        return true;
    return false;
}

void afficherTableau(int distance[], int n) {
    int i;
    printf("Sommet Distance depuis la source\n");
    for (i = 0; i < n; ++i)
        printf("%d \t\t %d\n", i, distance[i]);
}

void dijkstra(Graphe* graphe, int src) {
    int v;
    int V = graphe->numNoeuds;
    int distance[V];
    TasMin* tasMin = creerTasMin(V);

    for (v = 0; v < V; ++v) {
        distance[v] = INT_MAX;
        tasMin->tableau[v] = nouveauNoeudTas(v, distance[v]);
        tasMin->pos[v] = v;
    }

    tasMin->tableau[src] = nouveauNoeudTas(src, distance[src]);
    tasMin->pos[src] = src;
    distance[src] = 0;
    diminuerCle(tasMin, src, distance[src]);

    tasMin->taille = V;

    while (!estVide(tasMin)) {
        NoeudTas* minHeapNode = extraireMin(tasMin);
        int u = minHeapNode->v;

        NoeudAdj* pCrawl = graphe->tableau[u].tete;
        while (pCrawl != NULL) {
            int v = pCrawl->dest;

            if (estDansTasMin(tasMin, v) && distance[u] != INT_MAX && pCrawl->poids + distance[u] < distance[v]) {
                distance[v] = distance[u] + pCrawl->poids;
                diminuerCle(tasMin, v, distance[v]);
            }
            pCrawl = pCrawl->suivant;
        }
    }

    afficherTableau(distance, V);
}

int main() {
    int V = 9;
    Graphe* graphe = creerGraphe(V);
    ajouterArete(graphe, 0, 1, 4);
    ajouterArete(graphe, 0, 7, 8);
    ajouterArete(graphe, 1, 2, 8);
    ajouterArete(graphe, 1, 7, 11);
    ajouterArete(graphe, 2, 3, 7);
    ajouterArete(graphe, 2, 8, 2);
    ajouterArete(graphe, 2, 5, 4);
    ajouterArete(graphe, 3, 4, 9);
    ajouterArete(graphe, 3, 5, 14);
    ajouterArete(graphe, 4, 5, 10);
    ajouterArete(graphe, 5, 6, 2);
    ajouterArete(graphe, 6, 7, 1);
    ajouterArete(graphe, 6, 8, 6);
    ajouterArete(graphe, 7, 8, 7);

    dijkstra(graphe, 0);

    return 0;
}

