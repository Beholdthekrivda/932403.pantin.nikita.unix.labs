#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

struct Payload
{
    int id;
};

struct Monitor 
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    bool ready;
    int stopping;
    Payload* data;

    void produce() 
    {
        for (int i = 0; i < 5; i++)
        {
            sleep(1);
            pthread_mutex_lock(&mutex); 

            while (ready)
            {
                pthread_cond_wait(&cond, &mutex);
            }

            data = new Payload();
            data->id = (i + 1) * 2;
            cout << "Поставщик отправил сообщение" << endl;
            ready = true;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }

        pthread_mutex_lock(&mutex); 
        stopping = 1;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    };

    void consume() 
    {
        while (true)
        {
            pthread_mutex_lock(&mutex); 

            while (!ready && !stopping)
            {
                pthread_cond_wait(&cond, &mutex);
            }

            if (stopping && !ready)
            {
                pthread_mutex_unlock(&mutex);
                break;
            }

            Payload *p = data;
            cout << "Потребитель обработал событие, id: " << p->id << endl;
            delete p;
            data = nullptr;
            ready = false;

            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex); 
        }
    };
};

void* producer_run(void* arg) 
{
    Monitor *m = (Monitor *)arg;

    m->produce();

    return nullptr;
}
void* consumer_run(void* arg) 
{
    Monitor *m = (Monitor *)arg;

    m->consume();

    return nullptr;
}

int main()
{
    Monitor monitor;

    monitor.ready = false;
    monitor.stopping = 0;
    monitor.data = nullptr;

    pthread_t t_prod, t_cons;
    pthread_create(&t_prod, NULL, producer_run, &monitor);
    pthread_create(&t_cons, NULL, consumer_run, &monitor);
    pthread_join(t_prod, NULL);
    pthread_join(t_cons, NULL);

    pthread_mutex_destroy(&monitor.mutex);
    pthread_cond_destroy(&monitor.cond);

    return 0;
}