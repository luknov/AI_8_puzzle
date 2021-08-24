#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#pragma warning (disable : 4996)

struct Uzol {									//štruktúra uzla
	unsigned int h;								//ve¾kos heuristiky uzla
	unsigned int g;								//håbka uzla
	int **stav;									//stav uzla
	char operator;								//operator uzla - U, D, L, R (UP, DOWN, LEFT, RIGHT)
	struct Uzol *parent;						//odkaz na predchodcu
};

struct ListUzol {								//štruktúra uzla v zozname
	struct Uzol *akt;
	struct ListUzol* prev;
	struct ListUzol* next;
};

struct UzolList {								//štruktúra zoznamu uzlov
	unsigned int pocet;
	struct ListUzol* head;
	struct ListUzol* tail;
};

struct Riesenie {								//štruktúra riešenia
	char operator;
	struct Riesenie* next;
};

int f(struct Uzol *uzol) {						//výpoèet vyhodnocovacej funkcie uzla
	return uzol->g + uzol->h;
}

int heuristika1(int **arr1, int **arr2, int x, int y){			//výpoèet 1. heuristiky - Poèet políèok, ktoré nie sú na svojom mieste
	int i, j, h1 = 0;
	
	for (i = 0; i < x; i++)
		for (j = 0; j < y; j++)
			if (arr1[i][j] != arr2[i][j])						//ak sa políèka nerovnajú, zvýš hodnotu h
				if (arr1[i][j] != 0)							//pri poèítaní neberiem do úvahy prázdne políèko
					h1++;
	
	return h1;
}

int find_row(int **arr1, int **arr2, int x, int y, int max_c) {	//nájdenie riadku, kde sa nachádza políèko
	int i = 0, j = 0;											//slúži na výpoèet 2. heuristiky
	
	while (arr1[x][y] != arr2[i][j]) {
		j++;
		if (j == max_c) {
			i++;
			j = 0;
		}
	}

	return i;
}

int find_col(int **arr1, int **arr2, int x, int y, int max_c) {	//nájdenie pozície v ståpci
	int i = 0, j = 0;

	while (arr1[x][y] != arr2[i][j]) {
		j++;
		if (j == max_c) {
			i++;
			j = 0;
		}
	}

	return j;
}


int heuristika2(int **arr1, int **arr2, int x, int y){			//výpoèet 2. heuristiky - Súèet vzdialeností jednotlivých políèok od ich cie¾ovej pozície
	int i, j, h2 = 0, r, c;

	for (i = 0; i < x; i++)
		for (j = 0; j < y; j++) {
			if (arr1[i][j] == 0)								//neberiem do úvahy prázdne políèko
				continue;
			else {
				r = find_row(arr1, arr2, i, j, y);				//nájdenie pozície v riadku
				c = find_col(arr1, arr2, i, j, y);				//nájdenie pozície v ståpci
				h2 += abs(i - r);								//výpoèet heuristiky - manhattan distance	
				h2 += abs(j - c);
			}
		}

	return h2;
}

void input(int **arr, int x, int y) {							//funkcia pre zadanie vstupného a výstupného stavu
	int i, j;
	
	for (i = 0; i < x; i++)
		for (j = 0; j < y; j++)
			scanf("%d", &arr[i][j]);

}

int** alloc(int **arr, int x, int y) {							//funkcia pre alokovanie stavu
	int i;

	arr = (int **)malloc(x * sizeof(int*));
	for (i = 0; i < x; i++)
		arr[i] = (int*)malloc(y * sizeof(int));

	return arr;
}

int rovnost_stavov(int **stav, int **goal, int x, int y) {		//pozri, èi sa stavy rovnajú, ak áno vrá 1, ak nie 0
	int i, j;
	
	for (i = 0; i < x; i++)
		for (j = 0; j < y; j++)
			if (stav[i][j] != goal[i][j])						//ak sa nerovná 1 políèko -> stavy sa nerovnajú
				return 0;

	return 1;
}

struct Uzol *vytvor_uzol(unsigned int g, unsigned int h, int **stav, struct Uzol *parent, int x, int y, char move) {	//vytvorenie uzla
	struct Uzol *novy = (struct Uzol*)malloc(sizeof(struct Uzol));
	
	if (novy != NULL) {
		novy->stav = alloc(novy->stav, x, y);				//priradenie všetkých údajov
		novy->g = g;
		novy->h = h;
		novy->stav = stav;
		novy->parent = parent;
		novy->operator = move;
	}
	return novy;
}

void vloz_do_zoznamu(struct Uzol *uzol, struct UzolList** list) {				//funkcia vloží uzol do zoznamu uzlov
	struct ListUzol *tmp = (struct ListUzol*)malloc(sizeof(struct ListUzol));
	
	tmp->akt = uzol;

	if (*list && (*list)->pocet == 0) {					//ak zoznam existuje, ale poèet uzlov v òom je 0
		(*list)->head = tmp;							//vlož 1. uzol
		(*list)->tail = tmp;
		tmp->next = NULL;
		tmp->prev = NULL;
		(*list)->pocet++;
	}

	if (*list == NULL) {								//ak zoznam neexistuje, vytvor zoznam
		*list = (struct UzolList*)malloc(sizeof(struct UzolList));
		(*list)->pocet = 0;
		(*list)->head = NULL;
		(*list)->tail = tmp;
	}
	else
		(*list)->head->prev = tmp;
	
	tmp->next = (*list)->head;
	tmp->prev = NULL;
	(*list)->head = tmp;

	(*list)->pocet++;

}

struct Uzol* vyber_zo_zoznamu(struct UzolList** list) {				//funkcia vyberie zo zoznamu uzol na konci
	struct ListUzol *prevUzol;
	struct Uzol *vyber;

	if (*list == NULL || (*list)->pocet == 0)
		return NULL;

	vyber = (*list)->tail->akt;
	prevUzol = (*list)->tail->prev;

	free((*list)->tail);

	if ((*list)->pocet == 1)
		(*list)->head = NULL;
	else
		prevUzol->next = NULL;

	(*list)->tail = prevUzol;
	(*list)->pocet--;

	return vyber;
}

struct Uzol* posun(int **stav, int x, int y, char move) {				//funkcia pre posun v puzzle, vracia nový uzol
	int i, j, row = 0, col = 0;
	char tmp;
	struct Uzol *novy = (struct Uzol*)malloc(sizeof(struct Uzol));

	novy->stav = alloc(novy->stav, x, y);

	for (i = 0; i < x; i++)							//zapamätám si pozíciu prázdneho políèka
		for (j = 0; j < y; j++) {
			if (stav[i][j] == 0) {
				row = i;
				col = j;
			}
			novy->stav[i][j] = stav[i][j];
		}

	if (row + 1 < x && move == 'U') {				//ak sa môžem posunú hore
		tmp = novy->stav[row + 1][col];				//vymeò prázdne políèko s políèkom, ktoré sa posunie
		novy->stav[row + 1][col] = 0;
		novy->stav[row][col] = tmp;
		novy->operator = 'U';
		return novy;
	}

	else if (row - 1 >= 0 && move == 'D') {			//ak sa môžem posunú dole
		tmp = novy->stav[row - 1][col];
		novy->stav[row - 1][col] = 0;
		novy->stav[row][col] = tmp;
		novy->operator = 'D';
		return novy;
	}

	else if (col + 1 < y && move == 'L') {			//ak sa môžem posunú v¾avo
		tmp = novy->stav[row][col + 1];
		novy->stav[row][col + 1] = 0;
		novy->stav[row][col] = tmp;
		novy->operator = 'L';
		return novy;
	}

	else if (col - 1 >= 0 && move == 'R') {			//ak sa môžem posunú vpravo
		tmp = novy->stav[row][col - 1];
		novy->stav[row][col - 1] = 0;
		novy->stav[row][col] = tmp;
		novy->operator = 'R';
		return novy;
	}

	free(novy);
	return NULL;
}

struct UzolList *getchildren(struct Uzol *parent, int **goal, int h, int x, int y) {	//funkcia na nájdenie nasledovníkov uzla
	struct UzolList *children = NULL;
	struct Uzol *test = NULL;
	struct Uzol *child = NULL;
	int h_pom = 0;

	if ((test = posun(parent->stav, x, y, 'D')) && parent->operator != 'U') {			//zisti, èi sa môžem posunú dole, a zároveò predošlý uzol nemal operátor UP - týmto eliminujem možnos navštívi znovu rovnaký stav
		if (h == 1)																		//zisti, ktorá heuristika bola zadaná
			h_pom = heuristika1(test->stav, goal, x, y);
		if (h == 2)
			h_pom = heuristika2(test->stav, goal, x, y);
		child = vytvor_uzol(parent->g + 1, h_pom, test->stav, parent, x, y, test->operator);	//vytovrenie nasledovníka
		vloz_do_zoznamu(child, &children);														//vloženie nasledovníka do zoznamu
	}

	if ((test = posun(parent->stav, x, y, 'U')) && parent->operator != 'D') {
		if (h == 1)
			h_pom = heuristika1(test->stav, goal, x, y);
		if (h == 2)
			h_pom = heuristika2(test->stav, goal, x, y);
		child = vytvor_uzol(parent->g + 1, h_pom, test->stav, parent, x, y, test->operator);
		vloz_do_zoznamu(child, &children);
	}

	if ((test = posun(parent->stav, x, y, 'L')) && parent->operator != 'R') {
		if (h == 1)
			h_pom = heuristika1(test->stav, goal, x, y);
		if (h == 2)
			h_pom = heuristika2(test->stav, goal, x, y);
		child = vytvor_uzol(parent->g + 1, h_pom, test->stav, parent, x, y, test->operator);
		vloz_do_zoznamu(child, &children);
	}

	if ((test = posun(parent->stav, x, y, 'R')) && parent->operator != 'L') {
		if (h == 1)
			h_pom = heuristika1(test->stav, goal, x, y);
		if (h == 2)
			h_pom = heuristika2(test->stav, goal, x, y);
		child = vytvor_uzol(parent->g + 1, h_pom, test->stav, parent, x, y, test->operator);
		vloz_do_zoznamu(child, &children);
	}

	return children;
}

void vloz_zoznam(struct UzolList **vloz, struct UzolList *list) {	//funkcia vloží do zoznamu uzly - uzol s najmenšou f je vložený na koniec
	struct ListUzol *vloz_uzol;				//uzol na vloženie do list
	struct ListUzol *list_uzol;
	struct ListUzol *tmp;
	struct Uzol *uzol;

	if (list->pocet == 0) {					//ak zoznam, ku ktorému sa má pripoji, je prázdny
		uzol = vyber_zo_zoznamu(vloz);
		vloz_do_zoznamu(uzol, &list);
	}

	while ((vloz_uzol = (*vloz)->head) != NULL) { 
		list_uzol = list->head;
		
		while (list_uzol != NULL && f(vloz_uzol->akt) < f(list_uzol->akt)) {	//pokia¾ nie som na konci zoznamu a hodnota f je menšia
			list_uzol = list_uzol->next;
		}

		tmp = vloz_uzol->next;

		if (list_uzol == NULL) {				//vloženie na koniec
			list->tail->next = vloz_uzol;
			vloz_uzol->prev = list->tail;
			vloz_uzol->next = NULL;
			list->tail = vloz_uzol;
		}

		else if (list_uzol->prev != NULL) {		//vloženie do list okrem head
				vloz_uzol->prev = list_uzol->prev;
				vloz_uzol->next = list_uzol;
				list_uzol->prev->next = vloz_uzol;
				list_uzol->prev = vloz_uzol;
		}

		else {									//vloženie do head
				vloz_uzol->next = list->head;
				vloz_uzol->prev = NULL;
				list->head->prev = vloz_uzol;
				list->head = vloz_uzol;
		}
		
		(*vloz)->head = tmp;
		(*vloz)->pocet--;
		list->pocet++;
	}

	free(*vloz);
	*vloz = NULL;
}

struct Riesenie* Asearch(int **vstup, int **vystup, int x, int y, int h) {		//A* h¾adanie
	struct Uzol *uzol = (struct Uzol*)malloc(sizeof(struct Uzol));
	struct Uzol *root;
	struct Uzol *goal = (struct Uzol*)malloc(sizeof(struct Uzol));
	struct Riesenie *riesenieHead = NULL;
	struct Riesenie *novy = NULL;
	struct UzolList *queue = NULL;
	struct UzolList *children = NULL;
	int equal;
	unsigned int h_pom = 0;

	if (h == 1)													//zistím, ktorá heuristika bola zadaná
		h_pom = heuristika1(vstup, vystup, x, y);
	else if (h == 2)
		h_pom = heuristika2(vstup, vystup, x, y);
	
	goal->stav = vystup;									
	root = vytvor_uzol(0, h_pom, vstup, NULL, x, y, ' ');		//vytvorím prvý uzol
	vloz_do_zoznamu(root, &queue);								//vložím ho do zoznamu

	while (queue->pocet > 0) {									//pokia¾ je poèet uzlov v zozname > 0
																						
		uzol = vyber_zo_zoznamu(&queue);						//vyberiem uzol zo zoznamu
		
		equal = rovnost_stavov(uzol->stav, goal->stav, x, y);	//skontrolujem èi som v koncovom stave
		if (equal == 1)
			break;

		children = getchildren(uzol, vystup, h_pom, x, y);		//vytvor nasledovníkov uzla

		vloz_zoznam(&children, queue);							//vložím ich do zoznamu
	}

	while (uzol != NULL) {										//naplnenie zoznamu riešenia
		novy = (struct Riesenie*)malloc(sizeof(struct Riesenie));
		novy->operator = uzol->operator;
		novy->next = riesenieHead;
		riesenieHead = novy;
		uzol = uzol->parent;								
	}

	return riesenieHead;
}

int main() {
	int m, n, h, pocitadlo;
	int **vstup_stav = NULL, **vystup_stav = NULL;
	struct Riesenie *riesenie = NULL;

	printf("Zadaj rozmery: ");							//zadanie rozmerov puzzle - m*n
	scanf("%d %d", &m, &n);

	vstup_stav = alloc(vstup_stav, m, n);				//alokovanie vstupného a koncového stavu
	vystup_stav = alloc(vystup_stav, m, n);

	printf("Zadaj vstup:\n");							//zadanie stavov
	input(vstup_stav, m, n);

	printf("Zadaj vystup:\n");
	input(vystup_stav, m, n);

	printf("Zadaj cislo heuristiky: ");					//výber heuristiky
	scanf("%d", &h);

	printf("Riesenie: \n");

	clock_t start = clock();							//poèítanie èasu behu programu
	riesenie = Asearch(vstup_stav, vystup_stav, m, n, h);
	double runtime = (double)(clock() - start) / CLOCKS_PER_SEC;

	if (riesenie == NULL) {								//ak je riešenie prázdne
		printf("Nema riesenie.\n");
		return 0;
	}

	if (riesenie->next == NULL) {						//ak riešenie nemá žiadneho nasledovníka
		printf("Vstupny stav je rovnaky ako koncovy.\n");
		return 0;
	}

	pocitadlo = 1;
	for (riesenie = riesenie->next; riesenie != NULL; riesenie = riesenie->next) {		//výpis riešenia
		printf("%d. %c \n", pocitadlo, riesenie->operator);
		pocitadlo++;
	}

	printf("\nRuntime: %f\n", runtime);

	return 0;
}