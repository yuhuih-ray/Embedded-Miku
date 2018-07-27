#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "led.h"

#define COMPONENT_MAX_NUM 5 
#define DISPLAY_WIDTH 32 
#define COLOR_MAPPING_SIZE 4

/*
 *   button:
 *      - 0 : A (Green)
 *      - 1 : B (Red)
 *      - 2 : X (Blue)
 *      - 3 : Y (Yellow)
 */
typedef struct {
    int slot;
    int height;
} component; 

static int color_mapping[COLOR_MAPPING_SIZE] = { 2, 1, 4, 3 };  //index : contoroller button number & SLOT number

static pthread_t displayThread;
static pthread_mutex_t currentComponentLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t lifeLock = PTHREAD_MUTEX_INITIALIZER;
static bool running;
static long totalDelay;
static int goneLife;

static component currentComponent[COMPONENT_MAX_NUM];

static void* displayLoop (void *);
static void displayLife(int life);
static void (*fp) (int) = 0;

int Display_init (int delayTimeInMs, void (*f) (int))
{
    for (int i = 0; i < COMPONENT_MAX_NUM; i++){
        currentComponent[i].height = -1;
        currentComponent[i].slot = -1;
    } 
    running = true;
    totalDelay = delayTimeInMs *1000000 ; 
    goneLife = 0;

    fp = f;

    int threadCreateResult = pthread_create (&displayThread, NULL, displayLoop, NULL);
    return threadCreateResult;
}

// functino for thread which will keep checking currentComponent and display them
static void* displayLoop (void* empty)
{
    printf ( "start displaying loop \n");

    struct timespec reqtime;
    reqtime.tv_sec = totalDelay / 1000000000; 
    reqtime.tv_nsec = totalDelay % 1000000000; 

    int width = DISPLAY_WIDTH / 4;
    int currentHeight;
    int slot;
    int goneLife = 4;
    while (running) {
        LED_clean_display();
        
        for (int i = 0; i < COMPONENT_MAX_NUM; i++){
            pthread_mutex_lock (&currentComponentLock);
            {
                currentHeight = currentComponent[i].height; 
                slot = currentComponent[i].slot;
            }
            pthread_mutex_unlock (&currentComponentLock);

            if ( currentHeight >= 16)
                currentComponent[i].height = -1;

            if ( currentHeight >=0 ){
                LED_display_rectangle(slot*8 + goneLife, currentHeight, slot*8 + width-1 - goneLife, (currentHeight+1)%16, color_mapping[currentComponent[i].slot] );
                currentComponent[i].height++;
                if (currentComponent[i].height >= 16){
                    currentComponent[i].height = -1;
                    if (fp != 0)
                        fp();
                } 
            }
        } 
        displayLife();
        nanosleep(&reqtime, NULL); 
    }
    printf ( "stop displaying loop \n");
    return NULL;
}

static void displayLife ()
{
    for (int i = 0; i < 4; i++){
        LED_display_rectangle(i*8 + goneLife, 15, (i+1)*8 - (1 + goneLife), 15, color_mapping[i]);
    }
}

void Display_cleanup(void)
{
    running = false;
    pthread_join (displayThread, NULL);
    LED_cleanup();
}

void Display_generateComponent (int button)
{
    if (button > 3 || button < 0)
        return;
    pthread_mutex_lock (&currentComponentLock);
    {
        for (int i = 0; i < COMPONENT_MAX_NUM; i++){
            if (currentComponent[i].height == -1){
                currentComponent[i].height = 0;
                currentComponent[i].slot = button;
                break;
            }
        }
    }
    pthread_mutex_unlock (&currentComponentLock);
    return;
}

void Display_decreaseLife(void)
{
    pthread_mutex_lock (&lifeLock);
    {
        if( goneLife < 4 )
            goneLife++;
    }
    pthread_mutex_unlock (&lifeLock);
}

void Display_recharegeLife(void)
{
    pthread_mutex_lock (&lifeLock);
    {
        goneLife = 0;
    }
    pthread_mutex_unlock (&lifeLock);
}
