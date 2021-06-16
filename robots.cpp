// C program to demonstrate working of Semaphores
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <math.h>

using namespace std;
// -Wall -Werror -Wextra

// Tabuleiro tamanho max 300 x 400
struct robo {
    int x, y;
    double vel;

    robo(){
        x = rand()%300;
        y = rand()%400;
        vel = 0.0;
    }

    void posicao(){
        cout << x << " - " << y <<endl;
    }

    void update(int _x, int _y, int tempo){
        int d_x, d_y;
        int ax = (x + _x < 0 || x + _x > 300) ? -1 : 1;
        int ay = (y + _y < 0 || y + _y > 300) ? -1 : 1;

        d_x = x + _x * ax;
        d_y = y + _y * ay;
        vel = (sqrt( (d_x * d_x) + (d_y * d_y))/tempo) * 0.1; //0.1 para cv parametros por segundo
    }
};

struct Item {
    int x, y, bot, time;
};

struct Buffer {
    sem_t full, empty;
    queue<Item> content;

    Buffer(int n){
        sem_init(&full, 0, 0);
        sem_init(&empty, 0, n);
    }

    void insert(Item it){
        sem_wait(&empty);

        content.push(it);

        sem_post(&full);
    }
    
    Item remove(){
        sem_wait(&full);
        
        Item it = content.front();
        content.pop();

        sem_post(&empty);
        return it;
    }

    ~Buffer(){
        sem_destroy(&full);
        sem_destroy(&empty);
    }
};

Buffer* buff;

void* thread(void* arg){
	//wait
    Item it;
    
    it.bot = *((int*) arg);
	it.time = (rand() % 3900) + 100;

    usleep(it.time * 1000);
    it.x = (rand() % 21) - 10;
    it.y = (rand() % 21) - 10;
    
    buff->insert(it);

    pthread_exit(0);
}


int main(){
    srand(time(NULL));
    robo *r1 = new robo(); 
    
    buff = new Buffer(3);
    
	

	pthread_t t1,t2;
	pthread_create(&t1, NULL, thread, NULL);
	
    sleep(2);
	
    pthread_create(&t2, NULL,thread, NULL);
	
    pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	

    delete r1;
    

	return 0;
}
