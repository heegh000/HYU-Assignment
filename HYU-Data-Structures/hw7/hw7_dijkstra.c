#include <stdio.h>
#include <stdlib.h>

#define MAX 7
#define INF 1000
#define UNDEF -1

int graph[MAX][MAX] = {
	{ 0, 28, INF, INF, INF, 10, INF },
	{ 28, 0, 16, INF, INF, INF, 14 },
	{ INF, 16, 0, 12, INF, INF, INF },
	{ INF, INF, 12, 0 , 22, 25, 24 },
	{ INF, INF, INF, 22, 0, 25, 24 },
	{ 10, INF, INF, INF, 25, 0, INF },
	{ INF, 14, INF, 18, 24, INF, 0 },
};

int dist[MAX];
int prev[MAX];
int check[MAX];

void dijkstra(int s, int n);
int getMinDist(int n);
void getPath(int d);

int main() {
	int i;
	dijkstra(0, MAX);
	
	for(i = 0; i < MAX; i++)
		printf("%d ", dist[i]);
	printf("\n");

	for(i = 0; i < MAX; i++)
		getPath(i);

	return 0;
	
}

void dijkstra(int s, int n) {
	int i, k;

	dist[s] = 0;
	prev[s] = UNDEF;

	for(i = 0; i < n; i++) {
		if( i != s) {
			dist[i] = INF;
			prev[i] = UNDEF;
			check[i] = 0;
		}
	}

	int minDist = INF;
	int pos;
	int newDist;

	for(k = 0; k < n; k++) {
		
		pos =  getMinDist(MAX);		
		for(i = 0; i < n; i++) {
			if(graph[pos][i] != INF && !check[i]) {
				minDist = dist[i];
				newDist = dist[pos] + graph[pos][i];
				
				if(minDist > newDist){
					dist[i] = newDist;
					prev[i] = pos;
				}
			}
		}
	}
}

int getMinDist(int n) {
	int i;
	int pos;
	int minDist = INF;

	for(i = 0; i < n; i++) {
		if(minDist > dist[i] && !check[i]) {
			minDist = dist[i];
			pos = i;
		}		

	}

	check[pos] = 1;

	return pos;
}


void getPath(int d) {
	while(prev[d] != UNDEF) {
		printf("%d <- ", d);
		d = prev[d];
	}
	printf("%d\n", d);
}
