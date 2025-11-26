// rtty_queue.ino

#include "rtty_queue.h"

// --- Global Variable Definitions (Only defined here) ---
char message_buffer[MAX_QUEUE_MESSAGES][MAX_MESSAGE_LENGTH];
int queue_head = 0; // Where the next message is ADDED (write index)
int queue_tail = 0; // Where the next message is TAKEN (read index)
bool is_transmitting = false; // Status flag

// Initialize the queue state
void initialiseQueue() {
    queue_head = 0;
    queue_tail = 0;
    is_transmitting = false;
    // Clearing the buffer is optional but good practice
    memset(message_buffer, 0, sizeof(message_buffer)); 
}

// Add a message to the queue
bool enqueueMessage(const char* newText) {
    //Serial.print("enqueueMessage for tx - "); Serial.println(newText);
    int next_head = (queue_head + 1) % MAX_QUEUE_MESSAGES;

    if (next_head == queue_tail) {
        // Queue is full
        return false; 
    }

    // 1. Copy the new string into the buffer at the head
    strncpy(message_buffer[queue_head], newText, MAX_MESSAGE_LENGTH - 1);
    message_buffer[queue_head][MAX_MESSAGE_LENGTH - 1] = '\0'; // Ensure null termination

    // 2. Advance the head
    queue_head = next_head;
    //Serial.print("Q msg:");Serial.println(newText);
    return true;
}

// Get the next message from the queue
const char* dequeueMessage() {
    if (queue_head == queue_tail) {
        // Queue is empty
        return NULL;
    }
    
    // Get the pointer to the message at the tail
    const char* message = message_buffer[queue_tail];

    // Advance the tail
    queue_tail = (queue_tail + 1) % MAX_QUEUE_MESSAGES;
    
    return message;
}

void clearQueue() {
    // Resetting both pointers to 0 means queue_head == queue_tail,
    // which is the condition for isQueueEmpty() being true.
    queue_head = 0;
    queue_tail = 0;
}

bool isQueueEmpty() {
    return (queue_head == queue_tail);
}