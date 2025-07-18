#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TABLE_SIZE 4096 /* 000000000000 to 111111111111 */
#define MODES 12

enum { FALSE, TRUE };
enum { INACTIVE, ACTIVE };
enum { SEMITONE = 1, WHOLETONE = 2, LEADING_TONE = 11, OCTAVE = 12};

/*
 * A bitfield of 12 width is used to represent each scale, for example
 * 111111111111 is the chromatic scale and 101101010110 is the dorian scale.
 * Scales start as 100000000000 and are built from there, with index being used
 * to indicate the location of the most recently activated tone. starting_semitones
 * semitones indicates how many consecutive tones there are at the start of the
 * scale, so this number can be summed with any consecutive tones at the end
 * of the scale when things 'wrap around'. If has_leaped is true then
 * starting_semitones will stop being incremented when semitone intervals are
 * added. 
 */
struct Scale {
	int pitch_classes[OCTAVE]; /* bitfield representing 12 tones */
	int index;
	int semitone_counter;
	int starting_semitones;
	int has_leaped;
};

int apply_interval(int pitch_class, int interval);
void print_scale(const struct Scale *scale);
int max_interval_size(int index, int pitches_to_add);
struct Scale *copy_scale(const struct Scale *scale);
int semitone_limit_exceeded(const struct Scale *scale, int interval_to_add, int max_semitones);
void update_scale_table(const struct Scale *scale);
int search_child_scales(int len, int max_semitones, struct Scale *scale);
struct Scale *init_scale();
int enumerate_scales(int pitches_to_add, int max_semitones);
void init_scale_table();

/* hash table for every possible 1 octave scale in the 12 tone system */
int scale_table[TABLE_SIZE];

/* circular addition */
int
apply_interval(int pitch_class, int interval)
{
	int ret = pitch_class + interval;

	return (ret >= OCTAVE ? ret - OCTAVE : ret);
}

/*
 * Treats the scale bitfield as a binary number and returns its decimal value
 * Mode: index of the LSB
 */
int
hash_scale(const struct Scale *scale, int mode)
{
	int degree, hash = 0;

	if (scale->pitch_classes[mode] == INACTIVE)
		return -1;
	for (degree = 0; degree < OCTAVE; degree++) {
		if (scale->pitch_classes[apply_interval(degree, mode)])
			hash += pow(2, degree);
	}
	return hash;
}

void
print_scale(const struct Scale *scale)
{
	int i;

	for (i = 0; i < OCTAVE; i++)
		printf("%d", scale->pitch_classes[i]);
	putchar('\n');
}

/* 
 * From this point in the scale, what is the largest interval that could
 * legally be. That is, it doesn't cause the scale to exceed an octave and 
 * leaves enough room to add the remaining notes which needed to be added 
 * still
 */
int
max_interval_size(int index, int pitches_to_add)
{
	return OCTAVE - index - pitches_to_add;
}

struct Scale *
copy_scale(const struct Scale *scale)
{
	struct Scale *new_scale = malloc(sizeof(struct Scale));
	int pc;

	for (pc = 0; pc < OCTAVE; pc++) {
		new_scale->pitch_classes[pc] = scale->pitch_classes[pc];
	}
	new_scale->index = scale->index;
	new_scale->starting_semitones = scale->starting_semitones;
	new_scale->has_leaped = scale->has_leaped;
	new_scale->semitone_counter = scale->semitone_counter;
	return new_scale;
}

/*
 * Returns TRUE if the consecutive tone limit has been exceeded by adding this
 * proposed interval. This may happen in 2 ways- if this interval is a semitone
 * then it will be consecutive with the previous tone, or if the interval added
 * means the 11th of the 12th tone system is activated, meaning it will be
 * consecutive with the very first note of the scale. In other words, it will
 * have 'wrapped around'.
 */
int
semitone_limit_exceeded(const struct Scale *scale, int interval_to_add, int max_semitones)
{
	int adj_semitones = 0;

	if (interval_to_add == SEMITONE)
		adj_semitones += scale->semitone_counter + 1;
	if (scale->index + interval_to_add == LEADING_TONE)
		adj_semitones += scale->starting_semitones + 1;
	return !(adj_semitones < max_semitones);
}

/* flags entries in the table as enumerated for all modes of this scale */
void
update_scale_table(const struct Scale *scale)
{
	int mode;

	for (mode = 0; mode < OCTAVE; mode++) 
		if (scale->pitch_classes[mode])
			scale_table[hash_scale(scale, mode)] = ACTIVE;
}

/*
 * Enumerates all possible 1 octave scales of N notes at most M consective
 * chromatic pitches. Starting from the first pitch, a branch is created for
 * each possible interval which could occur from this pitch, with this process
 * being recursively applied to each subsequent branch until the scale has been
 * completed. The return value is an integer representing the total number of 
 * scales which have been enumerated.
 */
int
search_child_scales(int pitches_to_add, int max_semitones, struct Scale *scale)
{
	int interval, upper, number_of_scales = 0, ret;
	struct Scale *new_scale;

	if (pitches_to_add == 0) {
		if (scale_table[hash_scale(scale, 0)] == INACTIVE) {
			print_scale(scale);
			update_scale_table(scale);
			ret = 1;
		} else
			ret = 0;
		return ret;
	}
	upper = max_interval_size(scale->index, pitches_to_add);
	for (interval = SEMITONE; interval <= upper; interval++) {
		if (semitone_limit_exceeded(scale, interval, max_semitones) == FALSE) {
			new_scale = copy_scale(scale);
			new_scale->index += interval;
			new_scale->pitch_classes[new_scale->index] = ACTIVE;
			if (interval == SEMITONE) {
				new_scale->semitone_counter++;
				if (new_scale->has_leaped == FALSE)
					new_scale->starting_semitones++;
			} else {
				new_scale->semitone_counter = 0;
				new_scale->has_leaped = TRUE;
			}
			number_of_scales += search_child_scales(pitches_to_add - 1, max_semitones, new_scale);
			free(new_scale);
		}
	}
	return number_of_scales;
}

struct Scale *
init_scale()
{
	struct Scale *scale = malloc(sizeof(struct Scale));
	int i;

	scale->index = 0;
	scale->pitch_classes[0] = ACTIVE;
	for (i = 1; i < OCTAVE; i++)
		scale->pitch_classes[i] = INACTIVE;
	scale->semitone_counter = 0;
	scale->starting_semitones = 0;
	scale->has_leaped = FALSE;
	return scale;
}

int
enumerate_scales(int pitches_to_add, int max_semitones)
{
	struct Scale *scale = init_scale();
	int number_of_scales;
	
	number_of_scales = search_child_scales(pitches_to_add-1, max_semitones, scale);
	free(scale);
	return number_of_scales;
}

void
init_scale_table()
{
	int i;

	for (i = 0; i < TABLE_SIZE; i++)
		scale_table[i] = INACTIVE;
}

int
main(int argc, char *argv[])
{
	int number_of_scales;

	if (argc < 3) {
		printf("Please supply the following arguments:\n");
		printf("\tN: number of active pitches\n");
		printf("\tM: max number of consecutive chromatic notes\n");
		printf("\teg: scales N M\n");
		return 0;
	}
	number_of_scales = enumerate_scales(atoi(argv[1]), atoi(argv[2]));
	printf("total: %d\n", number_of_scales);
	return 0;
}

