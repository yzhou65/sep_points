/**
 * The following algorithm is a greedy algorithm that optimally separates points
 * in the coordinate system by using axis-parallel lines.
 * It begins by creating a complete graph using the given points so that every point 
 * has connections to all other points.
 * The connections will be unlinked when separation starts.
 * Subsequently the points are separated by lines needed at most so that every two 
 * adjacent points have a line between them from the left to right or from the bottom 
 * to top. The current lines are pending, not final.
 * While separating points, every time the algorithm finds a line that can break the 
 * most links and commits this line. 
 * It repeats this step until all points are disconnected. 
 * The sub-problem is to find the line that can break the most connections at the 
 * current situation. The problem is solved by keeping finding the optimal solution 
 * for every sub-problem. This corresponds to the property of a greedy algorithm.
 *
 */
#include <stdlib.h>
#include <stdio.h>

/**
 * 100 is the maximum number of points.
 * 100 is also the maximum index of an input instance file.
 */
#define MAX_POINTS 100

typedef struct Line {
    int axis;
    float coord;
} myline;

typedef struct Point mypoint;
struct Point {
    int id;
    int x;
    int y;
    mypoint **connections;
};

enum Axis {
    V, H
};

enum File_Status{
    FILE_SUCCESS,
    FILE_NOT_EXISTS,
    FILE_NO_POINTS,
    FILE_ERROR_POINTS
};

/**
 * All initial points read from an input .txt file.
 */
mypoint mypoints[MAX_POINTS];

/**
 * The arrays of pointers to point structs sorted by x- or y-coordinate.
 * Based on the project description, points in the input files are pre-sorted
 * by the x-coordinates.
 */
mypoint *x_points[MAX_POINTS];
mypoint *y_points[MAX_POINTS];

/**
 * On the horizontal or vertical level, the lines needed at most to separate
 * two adjacent points, i.e. every two adjacent points has a line right in the
 * middle from the left to right or the bottom to top.
 * Note: the lines are not final.
 */
myline all_lines[MAX_POINTS * 2];
myline *lines[MAX_POINTS * 2];

/**
 * The array of pointers to the finalized axis-parallel lines that optimally
 * separate points.
 */
myline *final_lines[MAX_POINTS];

unsigned int num_points = 0;
unsigned int num_edges = 0;
unsigned int num_lines = 0;
unsigned int num_all_lines = 0;


/**
 * Reads an input .txt file and stores points' information.
 * @param id - the numerous part of the file name indicating file index.
 * @return - a file status
 */
int read_file(int id) {
    char file_name[200];
    sprintf(file_name, "input/instance%.2d.txt", id);

    FILE *input = fopen(file_name, "r");

    if(input == NULL){
        return FILE_NOT_EXISTS;
    }

    if(fscanf(input, "%d", &num_points) == EOF){
        return FILE_NO_POINTS;
    }

    /// Values scanned from the input file and stored as points' information
    int i = 0;
    int x = 0;
    int y = 0;
    while(fscanf(input, "%d %d", &x, &y) != EOF && i < MAX_POINTS){
        mypoints[i].x = x;
        mypoints[i].y = y;
        x_points[i] = &(mypoints[i]);
        y_points[i] = &(mypoints[i]);
        i++;
    }

    if(i != num_points){
        return FILE_ERROR_POINTS;
    }

    fclose(input);
    return FILE_SUCCESS;
}

/**
 * Writes the results into an output .txt file
 * @param id - the numerous part of the file name indicating file index
 */
void write_file(int id){
    char file_name[200];
    sprintf(file_name, "output_greedy/greedy_solution%.2d.txt", id);
    FILE *output = fopen(file_name, "w");
    fprintf(output, "%d\n", num_lines);

    int i = 0;
    for(; i < num_lines; i++){
        if(final_lines[i]->axis == V) {
            fprintf(output, "v ");
        } else {
            fprintf(output, "h ");
        }
        fprintf(output, "%.1f\n", final_lines[i]->coord);
    }
    fclose(output);
}


/**
 * Links all points.
 */
void link_points() {
    int i = 0;
    int j = 0;
    for (i = 0; i < num_points; i++) {
        mypoints[i].id = i;
        mypoints[i].connections = malloc(sizeof (mypoint *) * MAX_POINTS);
        for (j = 0; j < num_points; j++) {
            if (i == j) {
                mypoints[i].connections[j] = NULL;
            }
            else {
                mypoints[i].connections[j] = &(mypoints[j]);
                num_edges++;
            }
        }
    }

    if (num_edges != num_points * (num_points - 1)) {
        printf("The number of points is incorrect");
        exit(0);
    }
}

/**
 * Unlinks two points
 * @param pt1 - pointer to mypoint
 * @param pt2 - pointer to mypoint
 */
void unlink_points(mypoint *pt1, mypoint *pt2) {
    if (pt1->connections[pt2->id] != NULL) {
        pt1->connections[pt2->id] = NULL;
        pt2->connections[pt1->id] = NULL;
        num_edges -= 2;
    }
}

/**
 * Frees the memory of all points' connections.
 * Re-initializes the number of points, lines, edges and all_lines.
 */
void restore() {
    int i = 0;
    for (; i < num_points; i++) {
        free(mypoints[i].connections);
    }
    num_points = 0;
    num_lines = 0;
    num_edges = 0;
    num_all_lines = 0;
}


/**
 * Returns the index of the point closest to the left or bottom of a line,
 * which takes at most O(n) time.
 * If there is no point to the left or bottom of the line, return -1.
 * @param ln - pointer to a line struct
 */
int closest_point(myline *ln) {
    float coord = 0;
    int i = 0;
    for (; i < num_points; i++) {
        if (ln->axis == V) {
            coord = (float)x_points[i]->x;
        } else {
            coord = (float)y_points[i]->y;
        }
        if (coord > ln->coord) {
            return i - 1;
        }
    }
    return -1;
}

/**
 * Pre-separate points by using axis-parallel lines, which are not final.
 * From left to right, every two adjacent points are separated by a line whose x-coordinate
 * equals to the average of their x-coordinates.
 * From bottom to top, same lines are created.
 */
void pre_separate() {
    int i = 0;
    myline *v_ln;
    myline *h_ln;
    for (; i < num_points - 1; i++) {
        v_ln = &(all_lines[num_all_lines]);
        v_ln->axis = V;
        v_ln->coord = ((float)x_points[i]->x + (float)x_points[i + 1]->x) / 2;
        lines[num_all_lines] = v_ln;
        num_all_lines++;

    }
    for (i = 0; i < num_points - 1; i++) {
        h_ln = &(all_lines[num_all_lines]);
        h_ln->axis = H;
        h_ln->coord = ((float)y_points[i]->y + (float)y_points[i + 1]->y) / 2;
        lines[num_all_lines] = h_ln;
        num_all_lines++;
    }
}


/**
 * Returns the number of links that a line can break, which takes O(n^2)
 * Returns -1 if no link can be broken.
 * @param ln - pointer to a line struct
 * @return the number of links that a line can break.
 */
int links_to_break(myline *ln) {
    if (ln == NULL) {
        return 0;
    }

    mypoint **pt;
    if (ln->axis == V) {
        pt = x_points;
    } else {
        pt = y_points;
    }

    /// computes the number of links of points on different sides of the line.
    int closest = closest_point(ln);
    int num_links = 0;
    int i, j;
    for (i = 0; i <= closest; i++) {
        for (j = closest + 1; j < num_points; j++) {
            if (mypoints[pt[i]->id].connections[pt[j]->id] != NULL) {
                num_links++;
            }
        }
    }
    return num_links;
}

/**
 * Finalizes the axis-parallel lines that optimally separates points.
 * @param ln - pointer to a line struct
 */
void finalize_lines(myline *ln) {
    if (ln == NULL) {
        return;
    }
    final_lines[num_lines] = ln;
    int closest = closest_point(ln);

    mypoint **pt;
    if (ln->axis == V) {
        pt = x_points;
    } else {
        pt = y_points;
    }

    /// unlinks points at the two sides of the line to be committed
    int i, j;
    for (i = 0; i <= closest; i++) {
        for (j = closest + 1; j < num_points; j++) {
            unlink_points(pt[i], pt[j]);
        }
    }
    num_lines++;
}

/**
 *  Compares two points' y-coordinate for the following sorting step.
 */
int y_compare(const void *a, const void *b){
    return (*(mypoint **)a)->y - (*(mypoint **)b)->y;
}


int main() {
    printf("----------- Program starts -----------\n");
    int file_index = 1;
    int file_num = 0;
    for (; file_index < MAX_POINTS; file_index++) {
        int status = read_file(file_index);

        switch (status) {
            case FILE_NOT_EXISTS:
            	printf("No instance%.2d.txt found.\n", file_index);
                continue;

            case FILE_ERROR_POINTS:
                printf("instance%.2d.txt has incorrect number of points.\n", file_index);
                continue;

            case FILE_NO_POINTS:
                printf("There are no points in instance%.2d.txt\n", file_index);
                continue;
            default:
                break;
        }

        /// Sort the points by y-coordinate. Points are pre-sorted by x-coordinate.
        qsort(y_points, num_points, sizeof(mypoint *), &y_compare);

        link_points();
        pre_separate();

        while (num_edges > 0) {
            /// Finds the line that can break the most links.
            int num_link = links_to_break(lines[0]);
            int line_index = 0;
            int j;
            for (j = 1; j < num_all_lines; j++) {
                int temp = links_to_break(lines[j]);
                if (temp > num_link) {
                    line_index = j;
                    num_link = temp;
                }
            }
            finalize_lines(lines[line_index]);
            lines[line_index] = NULL;
        }

        write_file(file_index);
        
        restore();
        file_num++;
    }
    printf("%d files done.\n", file_num);
    printf("No more input files.\n");
    printf("----------- Program ends -----------\n");
}

