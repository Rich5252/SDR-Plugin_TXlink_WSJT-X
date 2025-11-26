#ifndef RTTY_QUEUE_H
#define RTTY_QUEUE_H

#include <Arduino.h>

// Max number of strings (messages) the queue can hold
#define MAX_QUEUE_MESSAGES 4

// Max length of a single message string
#define MAX_MESSAGE_LENGTH 72 

// Global message buffer and queue variables
extern char message_buffer[MAX_QUEUE_MESSAGES][MAX_MESSAGE_LENGTH];
extern int queue_head;
extern int queue_tail;
extern bool is_transmitting;

// Function Prototypes
void initialiseQueue();
bool enqueueMessage(const String& newText);
const char* dequeueMessage();
void clearQueue();
bool isQueueEmpty();

#endif