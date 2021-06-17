/* Avaliação 1 - Sistemas Operacionais - 2021
 * Integrantes / RA :
 * - Augusto Aranda Figueiredo ---------- 2017012181
 * - Matheus Botelho Menezes dos Santos - 2017012780 
 * - Gabriel Vitor Lage Duarte Siqueira - 2017011854
 * 
 * Comando utilizado para compilar:
 * clang++ -std=c++17 -pthread robots.cpp -o ./build/robots
 * 
 * Codigo no Github do integrante Augusto: 
 * https://github.com/Klusmo/so_prova1
 */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <queue>
#include <vector>
#include <math.h>
using namespace std;

struct robo {
    int x, y;
    vector<double> vel;

    robo(){
        x = rand()%300;
        y = rand()%400;
        vel.push_back(0);
    }

    void position(){
        cout << x <<" - " << y << endl;
    }

    void update(int _x, int _y, int tempo){
        int d_x, d_y;
        int ax = (x + _x < 0 || x + _x > 300) ? -1 : 1;
        int ay = (y + _y < 0 || y + _y > 400) ? -1 : 1;

        d_x = _x * ax;
        d_y = _y * ay;
        vel.push_back(  (sqrt( (d_x * d_x) + (d_y * d_y)) / tempo) * 1000 ); //1000 tempo de ms para seg, dist em cm
        x += d_x;
        y += d_y;
    }
};

struct Item {
    int x, y, bot, time;

    void print(){
        
        cout << "Robo " << bot << ": ";
        cout << "Posição: (" << setw(2) << x << ", " << setw(2) << y << ") ";
        cout << "Tempo: " << setw(4) << time << endl;
        //cout << "-------------------" << endl << endl << endl;
    }
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

    void state(){
        int f, e;
        sem_getvalue(&full, &f);
        sem_getvalue(&full, &e);
        cout<< "Full  slots: " << f << endl;
        cout<< "Empty slots: " << e << endl;
    }

    bool canRemove(){
        int f;
        sem_getvalue(&full, &f);
        return f > 0;
    }

    ~Buffer(){
        sem_destroy(&full);
        sem_destroy(&empty);
    }


};
Buffer* buff;

struct consumerData{
    bool* status;
    robo* bots;
};


void* produtor(void* arg){
    bool* state = (bool*) arg;

    for (int i = 0; i < 3; i++){
        Item it;
        it.bot = i;
        it.time = (rand() % 3900) + 100;
        it.x = (rand() % 21) - 10;
        it.y = (rand() % 21) - 10;
        
	    //wait
        usleep(it.time * 1000);
        // it.print();
        
        buff->insert(it);
    }
    *state = true;

    pthread_exit(0);
}

void* consumidor(void* arg) {
    consumerData cd = *((consumerData*) arg);
    
    // status[0] é a camera
    // status[1] é a gps
    // status[2] é a odometro
    while ((!cd.status[0] || !cd.status[1] || !cd.status[2]) || buff->canRemove()){
        Item i = buff->remove();
        
        cd.bots[i.bot].update(i.x, i.y, i.time);
    }

    pthread_exit(0);
}



int main(){
    srand(time(NULL));

	pthread_t camera, gps, odometro, c1;
    bool status[] = {false, false, false}; 
    robo bots[3];

    int n;
    cout << "Insira o tamanho do buffer: ";
    cin >> n;

    while (n <= 0){
        cout << "O valor deve ser maior que zero" << endl;
        cout << "Insira o tamanho do buffer: ";
        cin >> n;    
    }
    
    buff = new Buffer(n);
    
    // Inicializa produtores
    pthread_create(&camera  , NULL, produtor, &status[0]);
    pthread_create(&gps     , NULL, produtor, &status[1]);
    pthread_create(&odometro, NULL, produtor, &status[2]);
    
    // Inicializa Consumidor
    consumerData cd;
    cd.status = status;
    cd.bots = bots;
    pthread_create(&c1      , NULL, consumidor,  &cd);
    do {
        usleep(500 * 1000);
        system("clear");
        string fonte[] = {"Camera", "GPS", "Odometro"};
        string barra = "-------------------------------------------------------------------------------";

        cout<< endl << barra << endl ;
        cout << "| Fontes => ";
        for (int i = 0; i < 3; i++){
            string str = status[i] ? "Finalizado" : "Rodando";
            string cor = status[i] ? "\033[0;32m" : "\033[0;33m";
            cout<< fonte[i] << ": " << cor << setw(12) << str << "\033[m" << " | ";
        }
        cout<< endl << barra << endl ;
        
        cout<< endl << "Velocidade dos robos";
        cout<< endl << barra << endl ;
        for (int i = 0; i < 3; i++)
            cout<< "|" << setw(23) <<"Robo "<< i + 1 << " ";
        cout << "|";
        cout<< endl << barra << endl ;

        int l = -1;
        for (int i = 0; i < 3; i ++){
            int size = bots[i].vel.size();
            if (l < size)
                l = size;
        }
        
        for (int i = 1; i < l; i++) {
            for (int j = 0; j < 3; j ++)
                if (bots[j].vel.size() < i)
                    cout << "| " << setw(24) << " ";
                else
                    cout << "| " << setw(17) << fixed << setprecision(2) <<(bots[j].vel)[i] << " cm/seg";
            cout << "|" << endl;
        }
        cout<< barra << endl ;

    } while(!status[0] || !status[1] || !status[2]);

    pthread_join(camera  , NULL);
	pthread_join(gps     , NULL);
	pthread_join(odometro, NULL);
	pthread_join(c1      , NULL);
    
    cout<< " Fim ... " << endl;

    delete buff;

	return 0;
}
