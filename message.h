#ifndef MESSAGE_H__
#define MESSAGE_H__

#define MAX_NUM_QUEUED_MESSAGES 64

void message_callback_add(MessageCallback callback_func, unsigned int recipient);
void message_callback_remove(MessageCallback callback_func);
void message_send_immediate(struct message_t message);
void message_send_delayed(struct message_t message);
void message_init();

#endif	/* MESSAGE_H__ */
