#include "dm.h"

struct pos_t {
	int parent_in_closed;
	unsigned int x, y;
	int f, g, h;
	char valid;
};

static struct pos_t *OPEN, *CLOSED;
static unsigned int LIST_SIZE;
/* these are the indicies of the next unused spot */
static unsigned int OPEN_AVAILABLE, CLOSED_AVAILABLE;
/* these are the indicies where everything before this is insured to
 * be invalid */
static unsigned int OPEN_NEXT_MIN;

static astar_pos_vector_t FINAL_PATH;
static int FINAL_PATH_LENGTH;

static void
reset_vectors()
{
	int i;
	for(i=0; i<LIST_SIZE; i++)
		OPEN[i].valid = 0;

	OPEN_AVAILABLE = 0;
	CLOSED_AVAILABLE = 0;
	OPEN_NEXT_MIN = 0;

	FINAL_PATH_LENGTH = 0;
}

void
astar_init()
{
	/* The sizes of the open and closed list will never be larger than
	 * the number of squares on the map. */
	
	LIST_SIZE = GAMESTATE.map.width * GAMESTATE.map.height;
	OPEN = malloc(sizeof(*OPEN) * LIST_SIZE);
	CLOSED = malloc(sizeof(*CLOSED) * LIST_SIZE);
	FINAL_PATH = malloc(sizeof(*FINAL_PATH) * LIST_SIZE);

	reset_vectors();
}

void
astar_destroy()
{
	free(OPEN);
	free(CLOSED);
	free(FINAL_PATH);
}

static int
add_to_open(struct pos_t pos) {
	OPEN[OPEN_AVAILABLE++] = pos;
	return OPEN_AVAILABLE-1;
}

static int
add_to_closed(struct pos_t pos) {
	CLOSED[CLOSED_AVAILABLE++] = pos;
	return CLOSED_AVAILABLE-1;
}

static unsigned int
find_lowest_f_in_open()
{
	unsigned int i, lowest_i;
	float lowest_f;

	/* find first valid */
	for(i=OPEN_NEXT_MIN; i<OPEN_AVAILABLE; i++)
		if(OPEN[i].valid)
			break;
		else
			++OPEN_NEXT_MIN;
	lowest_i = i;
	lowest_f = OPEN[i].f;
	
	for(; i<OPEN_AVAILABLE; i++)
	{
		float cand = OPEN[i].f;
		if(OPEN[i].valid && cand < lowest_f) {
			lowest_f = cand;
			lowest_i = i;
		}
	}

	return lowest_i;
}

static int
find_in_open(unsigned int x, unsigned int y)
{
	int i;

	for(i=OPEN_NEXT_MIN; i<OPEN_AVAILABLE; i++)
		if(OPEN[i].valid && OPEN[i].x == x && OPEN[i].y == y)
			return i;
	
	return -1;
}

static int
find_in_closed(unsigned int x, unsigned int y)
{
	int i;

	for(i=0; i<CLOSED_AVAILABLE; i++)
		/* valid is not used in closed list */
		if(CLOSED[i].x == x && CLOSED[i].y == y)
			return i;
	
	return -1;
}

static int
int_abs(int a)
{
	if(a >=0)
		return a;
	else
		return -a;
}
	

static int
get_heuristic_cost(unsigned int start_x,
				   unsigned int start_y,
				   unsigned int end_x,
				   unsigned int end_y)
{
	return int_abs(end_x - start_x) * 10 + int_abs(end_y - start_y) * 10;
}

void
astar_best_path(struct astar_pos_t begin,
				struct astar_pos_t end)
{
	reset_vectors();
	
	struct pos_t begin_pos = {-1, begin.x, begin.y, 0, 0, 0, 1};
	struct pos_t end_pos = {-1, end.x, end.y, 0, 0, 0, 1};
	
	add_to_open(begin_pos);

	for(;;)
	{
		/* find lowest f in open */
		int next_open = find_lowest_f_in_open();		

		/* if open is empty then end */
		if(next_open == OPEN_AVAILABLE)
			break;
		
		/* add this new node to the closed list */
		int closed_index = add_to_closed(OPEN[next_open]);

		/* remove from open */
		OPEN[next_open].valid = 0;

		/* add the adjacent squares */
		int adj;
		for(adj=0; adj<4; adj++) {
			
			struct pos_t new_node = {-1, 0, 0, 0, CLOSED[closed_index].g, 0, 1};
		
			switch(adj) {
			case 0:					/* NORTH */
				new_node.x = CLOSED[closed_index].x;
				new_node.y = CLOSED[closed_index].y-1;
				break;
			case 1:					/* SOUTH */
				new_node.x = CLOSED[closed_index].x;
				new_node.y = CLOSED[closed_index].y+1;
				break;
			case 2:					/* EAST */
				new_node.x = CLOSED[closed_index].x+1;
				new_node.y = CLOSED[closed_index].y;
				break;
			case 3:					/* WEST */
				new_node.x = CLOSED[closed_index].x-1;
				new_node.y = CLOSED[closed_index].y;
				break;
			}

			int cost = map_move_cost_at(new_node.x, new_node.y);
			if(cost != -1 && find_in_closed(new_node.x, new_node.y) == -1)
			{
				int h = get_heuristic_cost(new_node.x, new_node.y, end_pos.x, end_pos.y);
				int found_in_open = find_in_open(new_node.x, new_node.y);
				if(found_in_open == -1) {
					new_node.parent_in_closed = closed_index;
					new_node.h = h;
					new_node.g = new_node.g + cost;
					new_node.f = new_node.g + new_node.h;
					add_to_open(new_node);					
				} else {
					if(new_node.g + cost < OPEN[found_in_open].g) {
						/* XXX: this branch hasn't been tested */

						OPEN[found_in_open].parent_in_closed = closed_index;
						OPEN[found_in_open].g = new_node.g + cost;
						OPEN[found_in_open].f = OPEN[found_in_open].g + OPEN[found_in_open].h;
					}
				}
			}
		}	

		/* check for stop */
		if(find_in_closed(end_pos.x, end_pos.y) != -1)
			break;
	}

	/* did we find a path? */
	int end_index = find_in_closed(end_pos.x, end_pos.y);
	if(end_index == -1) {
		FINAL_PATH_LENGTH = -1;
		return;
	}

	/* find number of steps */
	int steps = 0;
	int temp = end_index;
	while(CLOSED[temp].parent_in_closed != -1) {
		++steps;
		temp = CLOSED[temp].parent_in_closed;
	}
	
	/* backtracking to get path */
	int step = steps;
	temp = end_index;
	while(CLOSED[temp].parent_in_closed != -1) {
		FINAL_PATH[step].x = CLOSED[temp].x;
		FINAL_PATH[step].y = CLOSED[temp].y;

		--step;
		temp = CLOSED[temp].parent_in_closed;
	}
	FINAL_PATH[0].x = CLOSED[temp].x;
	FINAL_PATH[0].y = CLOSED[temp].y;
	FINAL_PATH[steps].x = end_pos.x;
	FINAL_PATH[steps].y = end_pos.y;
	
	FINAL_PATH_LENGTH = steps+1;
}

astar_pos_vector_t
astar_retrieve_path()
{
	return FINAL_PATH;
}

int 
astar_retrieve_path_length()
{
	return FINAL_PATH_LENGTH;
}
