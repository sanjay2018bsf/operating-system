#include <stdio.h>
#include <assert.h>
#include<string.h>
#include <time.h>
#include <pthread.h>

int NumCats,NumMice,Numbowls,cats_eats,mouse_eats;

#define NumBowls  3         

const int cat_wait=8;           
const int cat_eat=4;
const int mouse_wait=10;         
const int mouse_eat=3;


typedef struct FoodBowl
	 {
      int free_Bowls;             
      int cats_eating;            
      int mice_eating;            
      int cats_waiting;           
    enum {
        none_eating,
        cat_eating,
        mouse_eating
    } status[NumBowls];         
    pthread_mutex_t mutex;      
    pthread_cond_t free_cv;     
    pthread_cond_t cat_cv;      
} FoodBowl_t;


int NumCats,NumMice;

static const char *progname = "CAT AND MICE SYNCHRONIZATION SOLUTION";

static void Display(const char *name, pthread_t pet, const char *what,FoodBowl_t *FoodBowl, int my_FoodBowl)
{
    
    printf("  %s (id %x) %s eating from FoodBowl %d\n", name, pet, what, my_FoodBowl);
}


static void*cat(void *arg)
{
    FoodBowl_t *FoodBowl = (FoodBowl_t *) arg;
    int n = cats_eats;
    int my_FoodBowl = -1;
    int i;

    for (n = cats_eats; n > 0; n--) {

        pthread_mutex_lock(&FoodBowl->mutex);
    
        pthread_cond_broadcast(&FoodBowl->cat_cv);
        
		FoodBowl->cats_waiting++;
        
	while (FoodBowl->free_Bowls <= 0 || FoodBowl->mice_eating > 0) 
	{
          pthread_cond_wait(&FoodBowl->free_cv, &FoodBowl->mutex);
        }
        FoodBowl->cats_waiting--;

        assert(FoodBowl->free_Bowls > 0);
        FoodBowl->free_Bowls--;
        assert(FoodBowl->cats_eating < NumCats);
        FoodBowl->cats_eating++;
        
        
        for (i = 0; i < NumBowls && FoodBowl->status[i] != none_eating; i++) ;
         my_FoodBowl = i;
        
		assert(FoodBowl->status[my_FoodBowl] == none_eating);
        FoodBowl->status[my_FoodBowl] = cat_eating;
        Display("cat", pthread_self(), "started", FoodBowl, my_FoodBowl);
        pthread_mutex_unlock(&FoodBowl->mutex);

        sleep(cat_eat);
        
        pthread_mutex_lock(&FoodBowl->mutex);
        assert(FoodBowl->free_Bowls < NumBowls);
        FoodBowl->free_Bowls++;
        assert(FoodBowl->cats_eating > 0);
        FoodBowl->cats_eating--;
        FoodBowl->status[my_FoodBowl] = none_eating;

      
        pthread_cond_broadcast(&FoodBowl->free_cv);
        Display("cat", pthread_self(), "finished", FoodBowl, my_FoodBowl);
        pthread_mutex_unlock(&FoodBowl->mutex);

        sleep(rand() % cat_wait);
    }

    return NULL;
}

static void*mouse(void *arg)
{
    FoodBowl_t *FoodBowl = (FoodBowl_t *) arg;
    int n = mouse_eats;
    struct timespec ts;
    struct timeval tp;
    int my_FoodBowl;
    int i;

    for (n = mouse_eats; n > 0; n--)
	 {
        pthread_mutex_lock(&FoodBowl->mutex);
        
        while (FoodBowl->free_Bowls <= 0 || FoodBowl->cats_eating > 0 || FoodBowl->cats_waiting > 0)
		 {
            pthread_cond_wait(&FoodBowl->free_cv, &FoodBowl->mutex);
         }

        assert(FoodBowl->free_Bowls > 0);
        FoodBowl->free_Bowls--;
        assert(FoodBowl->cats_eating == 0);
        assert(FoodBowl->mice_eating < NumMice);
        FoodBowl->mice_eating++;

        for (i = 0; i < NumBowls && FoodBowl->status[i] != none_eating; i++) ;
        my_FoodBowl = i;
        
        assert(FoodBowl->status[my_FoodBowl] == none_eating);
        FoodBowl->status[my_FoodBowl] = mouse_eating;
        
		Display("mouse", pthread_self(), "started", FoodBowl, my_FoodBowl);
        
		pthread_mutex_unlock(&FoodBowl->mutex);
        
     
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += mouse_eat;
        pthread_mutex_lock(&FoodBowl->mutex);
        pthread_cond_timedwait(&FoodBowl->cat_cv, &FoodBowl->mutex, &ts);
        pthread_mutex_unlock(&FoodBowl->mutex);
        
        pthread_mutex_lock(&FoodBowl->mutex);
        assert(FoodBowl->free_Bowls < NumBowls);
        FoodBowl->free_Bowls++;
        assert(FoodBowl->cats_eating == 0);
        assert(FoodBowl->mice_eating > 0);
        FoodBowl->mice_eating--;
        FoodBowl->status[my_FoodBowl]=none_eating;

        
        pthread_cond_broadcast(&FoodBowl->free_cv);
        Display("mouse", pthread_self(), "finished", FoodBowl, my_FoodBowl);
        pthread_mutex_unlock(&FoodBowl->mutex);
        
        /* sleep to avoid Starvation */
        sleep(rand() % mouse_wait);
    }

    return NULL;
}


int main(int argc, char *argv[])
{
    int i, err;
    FoodBowl_t _FoodBowl, *FoodBowl;
	
	printf("Enter the number of cats: ");
	scanf("%d",&NumCats);

	printf("Enter the number of Mice: ");
	scanf("%d",&NumMice);

	printf(" how many times a cat wants to eat: ");
	scanf("%d",&cats_eats);

	printf(" how many times a mice wants to eat: ");
	scanf("%d",&mouse_eats);


    pthread_t cats[NumCats];
    pthread_t mice[NumMice];

    srand(time(NULL)); 
    FoodBowl = &_FoodBowl;
    memset(FoodBowl, 0, sizeof(FoodBowl_t));
    FoodBowl->free_Bowls = NumBowls;
    pthread_mutex_init(&FoodBowl->mutex, NULL);
    pthread_cond_init(&FoodBowl->free_cv, NULL);
    pthread_cond_init(&FoodBowl->cat_cv, NULL);
    
     printf("\n Cat And Mice Synchronization for  Eating from Bowls\n\n");
     printf("[1,2,3] BOWLS OCCUPIED BY \n\n");
    
    for (i = 0; i < NumCats; i++) {
        err = pthread_create(&cats[i], NULL, cat, FoodBowl);
        if (err != 0) {
            printf("unable to create cat thread");
        }
    }

    for (i = 0; i < NumMice; i++) {
        err = pthread_create(&mice[i], NULL, mouse, FoodBowl);
         if (err != 0) 
		{
            printf(" unable to create mouse thread");
        } 
    }


    for (i = 0; i < NumCats; i++) 
	{
         pthread_join(cats[i], NULL);
    }
    for (i = 0; i < NumMice; i++)
	 {
         pthread_join(mice[i], NULL);
    }
    
    pthread_mutex_destroy(&FoodBowl->mutex);
    pthread_cond_destroy(&FoodBowl->free_cv);
    pthread_cond_destroy(&FoodBowl->cat_cv);
    
    return 0;
}
