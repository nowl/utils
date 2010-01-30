#ifndef ASTAR_H__
#define ASTAR_H__

struct astar_pos_t {
	unsigned int x, y;
};

typedef struct astar_pos_t* astar_pos_vector_t;

void astar_init();
void astar_destroy();

/* It is the user's responsibility to free the pointer returned by
 * this function when finished with it. */
void astar_best_path(struct astar_pos_t begin,
					 struct astar_pos_t end);
astar_pos_vector_t astar_retrieve_path();
int astar_retrieve_path_length();

#endif	/* ASTAR_H__ */
