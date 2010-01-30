#include "dm.h"

void
message_callback_add(MessageCallback callback_func, unsigned int recipient)
{
	LOG("adding message callback %p\n", callback_func);

	struct message_callback_list_t *cb = malloc(sizeof(*cb));
	cb->recipient = recipient;
	cb->func = callback_func;
	cb->next = GAMESTATE.message_callback;
	GAMESTATE.message_callback = cb;
}

void
message_callback_remove(MessageCallback callback_func)
{
	LOG("removing message callback %p\n", callback_func);

	/* scan through callbacks to find the one to remove */
	struct message_callback_list_t *cb, *prev_cb = NULL;
	for(cb = GAMESTATE.message_callback; cb; prev_cb = cb, cb = cb->next) {
		if(cb->func == callback_func) {

			/* fix up previous-link's next pointers */
			if(prev_cb)
				prev_cb->next = cb->next;
			else
				GAMESTATE.message_callback = cb->next;

			free(cb);
			break;
		}
	}			
}

static void
message_send(struct message_t message)
{
	struct message_callback_list_t *cb;
	for(cb = GAMESTATE.message_callback; cb; cb = cb->next)
		if(cb->recipient == message.recipient) {
			cb->func(message);
			break;
		}
}

static struct message_t MESSAGE_QUEUE[MAX_NUM_QUEUED_MESSAGES];
static unsigned int MESSAGE_NEXT = 0;
static unsigned int MESSAGE_AVAILABLE = 0;

void
message_send_delayed(struct message_t message)
{
	MESSAGE_QUEUE[MESSAGE_AVAILABLE] = message;
	MESSAGE_AVAILABLE = (MESSAGE_AVAILABLE + 1) % MAX_NUM_QUEUED_MESSAGES;
}

static void
process_queue()
{
	while(MESSAGE_NEXT != MESSAGE_AVAILABLE) {
		message_send(MESSAGE_QUEUE[MESSAGE_NEXT]);
		MESSAGE_NEXT = (MESSAGE_NEXT + 1) % MAX_NUM_QUEUED_MESSAGES;
	}
}

void
message_init()
{
	update_callback_add(process_queue);
}

void
message_send_immediate(struct message_t message)
{
	message_send(message);
}
