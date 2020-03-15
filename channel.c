#include "channel.h"

/* Reference: Channels Slides posted on Canvas
              Operating Systems: Three Easy Pieces Textbook
*/

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel
channel_t* channel_create(size_t size)
{
    /* IMPLEMENT THIS */
    /* Initializing all the members of struct channel_t */
	channel_t* channel = (channel_t *) malloc(sizeof(channel_t));
	channel->buffer = buffer_create(size);
    channel->closed = 0;
    // pthread_mutex_t mutex; 
    // pthread_cond_t full, empty;
    pthread_cond_init(&channel->full, NULL);
    pthread_cond_init(&channel->empty, NULL);
    //pthread_cond_init(&channel->select, NULL);
	pthread_mutex_init(&channel->mutex, NULL);
    //channel->size = size;
	//channel->mutex = mutex;

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
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
     /* When close function closes the channel from outside and we are still waiting we should check if the channel is close  */
    while(buffer_add(channel->buffer, data)==-1){
        pthread_cond_wait(&channel->empty, &channel->mutex);//Wait till channel not empty
           // return GEN_ERROR;          
        if(channel->closed){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
        }
    }
    // if(!(buffer_add(channel->buffer, data)))
    //     return GEN_ERROR;
    pthread_cond_signal(&channel->full);
    pthread_mutex_unlock(&channel->mutex);
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
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    while(buffer_remove(channel->buffer, data)==-1){
        pthread_cond_wait(&channel->full, &channel->mutex);
        if(channel->closed){
            pthread_mutex_unlock(&channel->mutex);
            return CLOSED_ERROR;
        }
    }
        
    //buffer_remove(channel->buffer, data);
    pthread_cond_signal(&channel->empty);
    pthread_mutex_unlock(&channel->mutex);
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
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    if(buffer_add(channel->buffer, data)==-1){
        pthread_mutex_unlock(&channel->mutex);      //Was causing error when I was not unlocking in this case
        return CHANNEL_FULL;
    }
    pthread_cond_signal(&channel->full);            //Was missing signal here and was waiting for very long in test cases thus failing them
    pthread_mutex_unlock(&channel->mutex);
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

    pthread_mutex_lock(&channel->mutex);
    if(channel->closed){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;                        
    }   
    if(buffer_remove(channel->buffer, data)==-1){
        pthread_mutex_unlock(&channel->mutex);          //Earlier error when not unlocking in this if case.
        return CHANNEL_EMPTY;
    }
    pthread_cond_signal(&channel->empty);            //Was missing signal here and was waiting for very long in test cases thus failing them
    pthread_mutex_unlock(&channel->mutex);
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
    pthread_mutex_lock(&channel->mutex);
    if(channel->closed){
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    channel->closed  = 1;
    pthread_cond_broadcast(&channel->full);
    pthread_cond_broadcast(&channel->empty);
    pthread_mutex_unlock(&channel->mutex);
    //channel->closed = 0;                 
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
    //pthread_mutex_lock(&channel->mutex);
    if(channel->closed == 0){
        //pthread_mutex_unlock(&channel->mutex);
        return DESTROY_ERROR;
    }
    /* Destroying all the mutex and condition variables initialized. Calling buffer_free function to free the buffer. Also, freeing the memory for channel */
    pthread_mutex_destroy(&channel->mutex);
    pthread_cond_destroy(&channel->full);
    pthread_cond_destroy(&channel->empty);
    buffer_free(channel->buffer);
    free(channel);
    //pthread_mutex_unlock(&channel->mutex);
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
   size_t count = 0;
   int flag = 1;
    while(flag){
        //count;
        channel_t *channel = channel_list[count].channel;
        void *data = channel_list[*selected_index].data;
        // if(channel_list[count].dir == RECV && buffer_current_size(channel->buffer) == channel->size){
        //     channel_non_blocking_receive(channel, &data);
        //     flag = 0;
        // //channel_list[selected_index]->
        // }
        //Performing SEND
        if(channel_list[count].dir == SEND && buffer_current_size(channel->buffer) == 0){
            pthread_mutex_lock(&channel->mutex);
            if(channel->closed){
            pthread_mutex_unlock(&channel->mutex);
                return CLOSED_ERROR;
            }
            if(buffer_add(channel->buffer, data)==-1){
                pthread_mutex_unlock(&channel->mutex);      //Was causing error when I was not unlocking in this case
                 return CHANNEL_FULL;
            }
            //pthread_cond_signal(&channel->full);            //Was missing signal here and was waiting for very long in test cases thus failing them
            pthread_mutex_unlock(&channel->mutex);
            selected_index = count; //
            flag = 0;
            return SUCCESS;
            //channel_list[selected_index]->
        }
        //Performing RECV
        else if(channel_list[count].dir == RECV && buffer_current_size(channel->buffer) == channel->size){
            pthread_mutex_lock(&channel->mutex);
            if(channel->closed){
                pthread_mutex_unlock(&channel->mutex);
                return CLOSED_ERROR;                        
            }   
            if(buffer_remove(channel->buffer, data)==-1){
                pthread_mutex_unlock(&channel->mutex);          //Earlier error when not unlocking in this if case.
                return CHANNEL_EMPTY;
            }
            //pthread_cond_signal(&channel->empty);            //Was missing signal here and was waiting for very long in test cases thus failing them
            pthread_mutex_unlock(&channel->mutex);
            selected_index = count; 
            flag = 0;
            return SUCCESS;
        }
        count = (count+1)%channel_count;
    }

    return SUCCESS;
}
//Each iteration lock for 

//While we wait in select, some other thread might comein and specifically not call select and change the channel condition right.?