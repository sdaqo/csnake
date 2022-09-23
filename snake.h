typedef struct Direction {
	int dx;
	int dy;
} Direction;

typedef struct Body {
	int posx;
	int posy;

	struct Body *next;
} Body;

typedef struct Apple {
	int posx;
	int posy;
} Apple;
