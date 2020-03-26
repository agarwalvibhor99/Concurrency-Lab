#include "channel.h"

/* Reference: Channels Slides posted on Canvas
              Operating Systems: Three Easy Pieces Textbook
*/

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel


int Pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr){
    int value = pthread_mutex_init(mutex, NULL);
    if (value != 0){
        printf("Error Initializing Mutex");
        return -1;
    }
    return 1;
}

int Pthread_cond_init(pthread_cond_t *cond, pthread_mutexattr_t *attr){
    int value = pthread_cond_init(cond, NULL);
    if (value != 0){
        printf("Error Initializing Condition Variable");
        return -1;
    }
    return 1;
}

int Pthread_mutex_lock(pthread_mutex_t *mutex){
    int value = pthread_mutex_lock(mutex);
    if (value != 0){
        printf("Error Locking Mutex");
        return -1;
    }
    return 1;
}

int Pthread_mutex_unlock(pthread_mutex_t *mutex){
    int value = pthread_mutex_unlock(mutex);
    if (value != 0){
        printf("Error Unlocking Mutex");
        return -1;
    }
    return 1;
}

int Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
    int value = pthread_cond_wait(cond, mutex);
    if (value != 0){
        printf("Error Wait");
        return -1;
    }
    return 1;
}


int Pthread_cond_signal(pthread_cond_t *cond){
    int value = pthread_cond_signal(cond);
    if (value != 0){
        printf("Error Signalling Condition Variable");
    }
    return 1;
}

int Pthread_mutex_destroy(pthread_mutex_t *mutex){
    int value = pthread_mutex_destroy(mutex);
    if (value != 0){
        printf("Error Destroying Mutex");
        return -1;
    }
    return 1;
}

int Pthread_cond_destroy(pthread_cond_t *cond){
    int value = pthread_cond_destroy(cond);
    if (value != 0){
        printf("Error Destroying Condition Variable");
        return -1;
    }
    return 1;
}

int Pthread_cond_broadcast(pthread_cond_t *cond){
    int value = pthread_cond_broadcast(cond);
    if (value != 0){
        printf("Error Broadcast");
        return -1;
    }
    return 1;
}

channel_t* channel_create(size_t size)
{
    /* IMPLEMENT THIS */
    /* Initializing all the members of struct channel_t */
	channel_t* channel = (channel_t *) malloc(sizeof(channel_t));
	channel->buffer = buffer_create(size);
    channel->closed = 0;
    if(Pthread_cond_init(&channel->full, NULL)==-1){
        return NULL;
    }
    if(Pthread_cond_init(&channel->empty, NULL)==-1){
        return NULL;
    }
	if(Pthread_mutex_init(&channel->mutex, NULL)==-1){
        return NULL;
    }
    channel->list = list_create();
    //channel->size = size;
    return channel;
}

// Writes data to the given channel
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
/* Reference from Textbook: Three Easy Pieces - Consumer Producer Problem */
enum channel_status channel_send(channel_t *channel, void* data)
{
    /* IMPLEMENT THIS */
    Pthread_mutex_lock(&channel->mutex);
    if(channel == NULL)
        return GEN_ERROR;
    if(channel->closed){
        if(Pthread_mutex_unlock(&channel->mutex)==-1)
            return GEN_ERROR;
        return CLOSED_ERROR;
    }
     /* When close function closes the channel from outside and we are still waiting we should check if the channel is close  */
    while(buffer_add(channel->buffer, data)==-1){
        if(Pthread_cond_wait(&channel->empty, &channel->mutex)==-1)//Wait till channel not empty
            return GEN_ERROR;          
        if(channel->closed){
            if(Pthread_mutex_unlock(&channel->mutex)==-1)
                return GEN_ERROR;
            return CLOSED_ERROR;
        }
    }
    if(Pthread_cond_signal(&channel->full)==-1)
        return GEN_ERROR;
    //Pthread_cond_signal(&channel->list->head->data);
    if(Pthread_mutex_unlock(&channel->mutex)==-1)
        return GEN_ERROR;
    return SUCCESS;
}

// Reads data from the given channel and stores it in the function’s input parameter, data (Note that it is a double pointer).
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
/* Reference from Textbook: Three Easy Pieces - Consumer Producer Problem */
enum channel_status channel_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */
    Pthread_mutex_lock(&channel->mutex);
    if(channel == NULL)
        return GEN_ERROR;
    if(channel->closed){
        Pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    while(buffer_remove(channel->buffer, data)==-1){
        Pthread_cond_wait(&channel->full, &channel->mutex);
        if(channel->closed){
            Pthread_mutex_unlock(&channel->mutex);
            return CLOSED_ERROR;
        }
    }
    Pthread_cond_signal(&channel->empty);
    Pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Writes data to the given channel
// This is a non-blocking call i.e., the function simply returns if the channel is full
// Returns SUCCESS for successfully writing data to the channel,
// CHANNEL_FULL if the channel is full and the data was not added to the buffer,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
/* 
 * Similar to blocking send except not using while loop since waiting isn't required. Starting with locking and just using one if and returning CHANNEL_FULL
 * if buffer is full and accordingly unlocking after updating buffer.
 */
enum channel_status channel_non_blocking_send(channel_t* channel, void* data)
{
    /* IMPLEMENT THIS */
    Pthread_mutex_lock(&channel->mutex);
    if(channel == NULL)
        return GEN_ERROR;
    if(channel->closed){
        Pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    if(buffer_add(channel->buffer, data)==-1){
        Pthread_mutex_unlock(&channel->mutex);      //Was causing error when I was not unlocking in this case
        return CHANNEL_FULL;
    }
    Pthread_cond_signal(&channel->full);            //Was missing signal here and was waiting for very long in test cases thus failing them
    Pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Reads data from the given channel and stores it in the function’s input parameter data (Note that it is a double pointer)
// This is a non-blocking call i.e., the function simply returns if the channel is empty
// Returns SUCCESS for successful retrieval of data,
// CHANNEL_EMPTY if the channel is empty and nothing was stored in data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
/* 
 * Similar to blocking receive except not using while loop since waiting isn't required. Starting with locking and just using one if and returning CHANNEL_EMPTY
 * if buffer is empty and accordingly unlocking after updating buffer.
 */
enum channel_status channel_non_blocking_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */

    Pthread_mutex_lock(&channel->mutex);
    if(channel == NULL)
        return GEN_ERROR;
    if(channel->closed){
        Pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;                        
    }   
    if(buffer_remove(channel->buffer, data)==-1){
        Pthread_mutex_unlock(&channel->mutex);          //Earlier error when not unlocking in this if case.
        return CHANNEL_EMPTY;
    }
    Pthread_cond_signal(&channel->empty);            //Was missing signal here and was waiting for very long in test cases thus failing them
    //Pthread_cond_signa
    Pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// GEN_ERROR in any other error case
/*
 * Using pthread_cond_broadcast which wakes all the threads. Changing the closed flag to 0 so that all threads when wake up to
 * updated closed value.
 */
enum channel_status channel_close(channel_t* channel)
{
    /* IMPLEMENT THIS */
    Pthread_mutex_lock(&channel->mutex);
    if(channel == NULL)
        return GEN_ERROR;
    if(channel->closed){
        Pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    channel->closed  = 1;
    Pthread_cond_broadcast(&channel->full);
    Pthread_cond_broadcast(&channel->empty);
    //pthread_cond_broadcast(&channel->)
    Pthread_mutex_unlock(&channel->mutex);
    return SUCCESS;
}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// GEN_ERROR in any other error case
enum channel_status channel_destroy(channel_t* channel)
{
    /* IMPLEMENT THIS */
    /* If the channel is not initialized/is closed */
    if(channel == NULL)
        return GEN_ERROR;
    if(channel->closed == 0){
        return DESTROY_ERROR;
    }
    /* Destroying all the mutex and condition variables initialized. Calling buffer_free function to free the buffer. Also, freeing the memory for channel */
    Pthread_mutex_destroy(&channel->mutex);
    Pthread_cond_destroy(&channel->full);
    Pthread_cond_destroy(&channel->empty);
    buffer_free(channel->buffer);
    list_destroy(channel->list);
    free(channel);
    return SUCCESS;
}

// Takes an array of channels, channel_list, of type select_t and the array length, channel_count, as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error
enum channel_status channel_select(select_t* channel_list, size_t channel_count, size_t* selected_index)
{
    /* IMPLEMENT THIS */
/*
//     struct myData {
//    pthread_cond_t *select;
//    pthread_mutex_t *select_mutex;    
//    } myData;
    pthread_mutex_t select_mutex;
    pthread_cond_t select;
//    pthread_mutex_init(&myData.select_mutex, NULL);
//    pthread_cond_init(&myData.select, NULL);
   
    Pthread_mutex_init(&select_mutex, NULL);
    Pthread_cond_init(&select, NULL);
    Pthread_mutex_lock(&select_mutex);
//     pthread_mutex_init(&myData.select_mutex, NULL);
//    pthread_cond_init(&myData.select, NULL);
   
   //pthread_mutex_lock(&myData.select_mutex);

   for(int i = 0 ; i < channel_count; i++){
       //channel_list[i].channel->list->head->select = &select;
       //channel_list[i].channel->list->head->select_mutex = &select_mutex;
        Pthread_mutex_lock(&channel_list[i].channel->mutex);
        list_insert(channel_list[i].channel->list, &select);
         Pthread_mutex_unlock(&channel_list[i].channel->mutex);
    }
   while(true){
    for(int i = 0 ; i < channel_count; i++){
        channel_t *channel = channel_list[i].channel;  
        //sem_post(&select);
        //pthread_mutex_lock(&channel->select_mutex);
        //void *data = channel_list[i].data;

        //Performing SEND
        Pthread_mutex_lock(&channel->mutex);
        if(channel_list[i].dir == SEND){ // 
            
            if(channel->closed){
            Pthread_mutex_unlock(&channel->mutex);
                return CLOSED_ERROR;
            }
            if(buffer_add(channel->buffer, data) == 0){
                //pthread_cond_signal(&select);            //Was missing signal here and was waiting for very long in test cases thus failing them
                Pthread_mutex_unlock(&channel->mutex);
                *selected_index = (size_t)i; //
                //flag = 0;
                return SUCCESS;
            }
        }
        //Performing RECV
        //pthread_mutex_lock(&channel->mutex);
        if(channel_list[i].dir == RECV){// remove second condition
           
            if(channel->closed){
                Pthread_mutex_unlock(&channel->mutex);
                return CLOSED_ERROR;                        
            }   
            if(buffer_remove(channel->buffer, data) == 0){
                pthread_cond_signal(&myData.select);
                //remocve_list;
                pthread_mutex_unlock(&channel->mutex);
                *selected_index = (size_t) i; 
                //flag = 0;
                return SUCCESS;
            }
        }
        Pthread_mutex_unlock(&channel->mutex);
    }
    //sem_wait(&select);
    Pthread_cond_wait(&select, &select_mutex); // I can't have this here because channel was local to the loop I created. Confused what should I wait on
   }
    //Pthread_mutex_unlock(&myData.select_mutex);
    */
    return SUCCESS;
}


//Created a new conditional variable for select to wait, and signalling in both the non blocking and blocking send and receive functions to wake this. 
//But since channel variable created is local to the loop can't signal it. Need some local variable that can help in signalling 



/*
 * Array are pointers. We are given a pointer to channel_list.



*/