#include <stdio.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include "bitset.h"
#include "graph.h"
#include "graph6.h"
#include "vertice_queue.h"

#define MAX_PATH_LENGTH 4096

typedef struct {
    bool aggregate;
    i32 max_cop;
    u8 workers;
} args_t;

typedef struct {
    u32 len;
    u32 *list;
} neigh_list_t;

/**
 * Computes the following equation:
 * c(G) \leq k
 * This algorithm is based off the one given at
    @article{bonato2010cops,
      title={Cops and robbers from a distance},
      author={Bonato, Anthony and Chiniforooshan, Ehsan and Pra{\l}at, Pawe{\l}},
      journal={Theoretical Computer Science},
      volume={411},
      number={43},
      pages={3834--3844},
      year={2010},
      publisher={Elsevier}
    }
 * @param g the graph
 * @param k the cop number "target"
 * @return
 */
bool bonato_al_algo2(graph_t *g, u8 k) {

    if (k >= 4) {
        printf("%d, ", k);
    }

    graph_t *tensor_graph = tensor_power(g, k);

    u32 n = g->n;
    u32 N = tensor_graph->n;

    bitset_t **phi = (bitset_t **) (malloc(sizeof(bitset_t *) * N));

    for (u32 i = 0; i < N; ++i) {
        phi[i] = NULL;
    }

    // This is line 1 of the algorithm
    // It sets the function to a bit mask of the neighbours in graph G
    // There are n vertices in G; therefore the max size of a neighbourhood is n
    // This is bitset_not very storage efficient as it is probably sparse.
    u32 *cached_tuple = malloc(sizeof(u32) * k);
    neigh_list_t *adjacency_list = malloc(sizeof(neigh_list_t) * N);

    for (u32 i = 0; i < N; ++i) {
        adjacency_list[i].len = 0; // We use len 0 as an indicator that it's empty since len >= 1 for all vertices
        u32 *tuple = int_to_tuple(k, cached_tuple, n, i);
        // This is convenient to preallocate the neighbourhoods anyways as we will bitset_not
        // have to do more mallocs bitset_and free later
        bitset_t *neigh = neighbourhood(g, tuple, k);
        bitset_not(neigh, neigh);
        phi[i] = neigh;
    }

    vertice_queue_t *q = vertice_queue_new(N);

    for (u32 i = 0; i < N; ++i) {
        vertice_queue_push(q, i);
    }

    while (q->sz > 0) {
        // Pop (line 4)
        u32 T = vertice_queue_pop(q);

        // Prepare the data for the rest of the while loop
        bitset_t *phi_t = phi[T];

        u32 phi_t_sz;
        u32 *phi_t_vertex_set = bitset_indices(phi_t, &phi_t_sz);
        bitset_t *phi_t_neighbourhood = neighbourhood(g, phi_t_vertex_set, phi_t_sz);

        neigh_list_t *nei = adjacency_list + T;

        // Memoize
        u32 neigh_sz;
        u32 *neighbours_indices = NULL;
        if (0 == nei->len) {
            bitset_t *neigh_of_t = tensor_graph->rows[T];
            neighbours_indices = bitset_indices(neigh_of_t, &neigh_sz);
            nei->len = neigh_sz;
            nei->list = neighbours_indices;
        } else {
            neigh_sz = nei->len;
            neighbours_indices = nei->list;
        }

        for (size_t i = 0; i < neigh_sz; ++i) {
            u32 t_prime = neighbours_indices[i];
            if (bitset_and(phi[t_prime], phi_t_neighbourhood)) {
                vertice_queue_push(q, t_prime);
            }
        }

        // Cleanup
        free(phi_t_vertex_set); // this was temporary
//        free(neighbours_indices); // this was temporary also
        bitset_destroy(phi_t_neighbourhood); // again, used for intersecting neighs
    }


    free(cached_tuple);
    destroy_graph(tensor_graph);
    vertice_queue_destroy(q);

    bool satisfied = 0;
    /*
     *  Checks whether there exists an empty position bitset_set (all 0)
     */
    for (u32 i = 0; i < N; ++i) {
        bitset_t *v = phi[i];
        neigh_list_t *t = adjacency_list + i;

        if (t->len > 0) {
            free(t->list);
        }

        if (!satisfied && !bitset_any(v)) {
            satisfied = TRUE;
        }
        // We will bitset_not used this array anymore; get rid of it
        bitset_destroy(v);
    }

    free(adjacency_list);
    free(phi);

    return satisfied;
}


u32 cop_number(graph_t *g, u8 max_k) {
    u32 k = 1;

    while (!bonato_al_algo2(g, k)) {
        k++;
        if (k > max_k) {
            printf("Over %d.\n", max_k);
            return max_k + 1;
        }
    }

    return k;
}


/**
 * Print the usage message of the program
 */
void usage(bool quick) {
    printf("Usage: path_to_g6 [-h (help)] [-k cop_number] [-w no_workers=1] [-c] [-s] [-a]\n\n");

    if (!quick) {
        printf("Copper is a tool to compute the cop number of graphs, stored in the g6 file format. The g6 file format\n");
        printf("can contain a single or multiple graphs. The tool supports the following commands:");

        const u8 params = 5;
        char *usage_str[5] = {
                "-k : the cop maximum cop number to check. Beyond this number, graphs will not be computed.",
                "-w : the maximum number of workers to use in parallel. The program may decide to not use them all.",
                "-c : if the computation must be timed using wall clock time (real time).",
                "-s : silent mode, does not print a description of received parameters.",
                "-a : aggregate mode, will not print the graph's cop number, but will print a table aggregating the result. Requires -k specified."
        };

        for (u8 i = 0; i < params; ++i) {
            printf("\t%s\n", usage_str[i]);
        }

    }
}

typedef struct {
    pthread_mutex_t *mut;
    pthread_mutex_t *aggr_mut;
    pthread_cond_t *consume;
    pthread_cond_t *produce;
    u32 *breakdown;
    char *line;
    args_t *args;
    bool done;
} task_profile_t;

void *cop_number_worker(void *task_profile_t_void) {
    task_profile_t *profile = (task_profile_t *) task_profile_t_void;
    args_t *args = profile->args;

    while (TRUE) {
        // We are ready to work on a graph
        pthread_mutex_lock(profile->mut);

        while (!profile->done && NULL == profile->line) {
            pthread_cond_wait(profile->consume, profile->mut);
        }

        if (profile->done) {
            // If work is done, bail out
            pthread_mutex_unlock(profile->mut);
            break;
        }

        // We got the following task
        graph_t *g = from_g6(profile->line);
        // Tell we received the task, are ready to continue
        profile->line = NULL;
        pthread_cond_signal(profile->produce);
        // We do not need access to those variables
        pthread_mutex_unlock(profile->mut);

        u32 k = cop_number(g, args->max_cop);
        destroy_graph(g);

        // We need to update the breakdown
        // Since this is updated by all workers, we keep it
        // locked.
        if (args->aggregate) {
            pthread_mutex_lock(profile->aggr_mut);

            profile->breakdown[k - 1] += 1;

            pthread_mutex_unlock(profile->aggr_mut);
        } else {
            pthread_mutex_lock(profile->mut);
            printf("%d\n", k);
            pthread_mutex_unlock(profile->mut);
        }
    }

    return NULL;
}


bool handle_file(char *file_path, args_t *args) {
    bool ok = TRUE;
    FILE *f = NULL;

    u32 *breakdown = NULL;
    bool aggregate;
    i32 max_cop;

    if ((aggregate = args->aggregate)) {
        breakdown = malloc(sizeof(u32) * (max_cop = args->max_cop));
        for (i32 k = 0; k < max_cop; ++k) {
            breakdown[k] = 0;
        }
    }

    task_profile_t task;
    pthread_mutex_t aggr_mut, task_mut;
    pthread_cond_t produce, consume;

    task.mut = &task_mut;
    task.produce = &produce;
    task.consume = &consume;
    task.line = NULL;
    task.breakdown = breakdown;
    task.done = FALSE;
    task.args = args;

    if (aggregate) {
        task.aggr_mut = &aggr_mut;
        pthread_mutex_init(task.aggr_mut, NULL);
    }

    pthread_mutex_init(task.mut, NULL);
    pthread_cond_init(task.produce, NULL);
    pthread_cond_init(task.consume, NULL);

    pthread_t *worker_list = malloc(sizeof(pthread_t) * args->workers);

    for (u8 i = 0; i < args->workers; ++i) {
        pthread_create(worker_list + i, NULL, cop_number_worker, &task);
    }

    if (NULL == (f = fopen(file_path, "r"))) {
        ok = FALSE;
    } else {
        char *line = NULL;
        size_t len = 0;
        bool first_line = TRUE;
        while (-1 != getline(&line, &len, f)) {
            char *line_to_read;
            if (first_line && 0 == strncmp(G6_HEADER, line, G6_HEADER_LEN)) {
                if ('\0' == line[G6_HEADER_LEN]) {
                    // The only line in the file was this header
                    continue;
                }
                line_to_read = line + G6_HEADER_LEN;
            } else {
                line_to_read = line;
            }
            first_line = FALSE;

            /*
             * Send the task to a worker
             */
            pthread_mutex_lock(task.mut);

            task.line = line_to_read;
            pthread_cond_signal(task.consume);

            while (NULL != task.line) {
                pthread_cond_wait(task.produce, task.mut);
            }

            pthread_mutex_unlock(task.mut);
        }
        free(line);
    }

    pthread_mutex_lock(task.mut);
    task.done = TRUE;
    // Wake up the ones that are waiting
    pthread_cond_broadcast(task.consume);
    pthread_mutex_unlock(task.mut);

    // Wait for all workers to finish; do not have the
    // full results yet
    for (u8 i = 0; i < args->workers; ++i) {
        pthread_join(worker_list[i], NULL);
    }

    if (aggregate) {
        for (u32 k = 0; k < max_cop; ++k) {
            printf("%d ", breakdown[k]);
        }
        printf("\n");
    }

    if (aggregate) {
        pthread_mutex_destroy(task.aggr_mut);
    }

    pthread_mutex_destroy(task.mut);
    pthread_cond_destroy(task.produce);
    pthread_cond_destroy(task.consume);

    free(worker_list);

    if (NULL != f) {
        fclose(f);
    }

    if (breakdown) {
        free(breakdown);
    }

    return ok;
}

bool handle_folder(char *folder_path, args_t *args) {
    char path[MAX_PATH_LENGTH];

    DIR *folder = opendir(folder_path);
    if (NULL == folder) {
        printf("Failed to open the folder. Aborting.");
        return FALSE;
    }

    struct dirent *entry = NULL;
    while (NULL != (entry = readdir(folder))) {
        char *name = entry->d_name;

        // Skip those
        if ((name[0] == '.' && name[1] == '\0') ||
            (name[0] == '.' && name[1] == '.' && name[2] == '\0')) {
            continue;
        }

        // Compute the len of all parts
        size_t folder_len = strlen(folder_path);
        bool requires_trailing_slash = ('/' != folder_path[folder_len - 1]);
        size_t name_len = strlen(name);
        size_t combined_len = folder_len + name_len + requires_trailing_slash;

        // This will be the complete file path, add 1 for \0
        memcpy(path, folder_path, folder_len);

        if (requires_trailing_slash) {
            path[folder_len] = '/';
        }

        memcpy(path + requires_trailing_slash + folder_len, name, name_len);
        path[combined_len] = '\0';

        printf(args->aggregate ? "%s " : "%s\n", name);
        handle_file(path, args);
    }

    closedir(folder);

    return FALSE;
}

int main(int argc, char *argv[]) {
#define USAGE_AND_LEAVE() do {usage(TRUE); return 1;} while(0)
    bool take_time, aggregate, silent;
    take_time = aggregate = silent = FALSE;
    i32 max_cop = -1;
    u8 workers = 1;

    time_t before = time(NULL);

    /*
     * Parse the options
     */
    if (1 == argc) {
        USAGE_AND_LEAVE();
    }

    char *path = argv[1];

    int c;
    while ((c = getopt(argc, argv, "hacsk:w:")) != -1) {
        switch (c) {
            case 'h':
                usage(FALSE);
                exit(1);
            case 'a':
                aggregate = TRUE;
                break;
            case 'c':
                take_time = TRUE;
                break;
            case 's':
                silent = TRUE;
                break;
            case 'k':
                max_cop = atoi(optarg);
                break;
            case 'w':
                workers = atoi(optarg);
                break;
            case '?':
                USAGE_AND_LEAVE();
            default:
                break;
        }
    }

    if (!silent) {
        printf("Samuel Yvon\n");
        printf("Cop Number Calculator\n");
        printf("Will use at maximum %d workers.\n", workers);

        if (aggregate) {
            printf("Aggregating results.\n");
        }

        if (take_time) {
            printf("Timing the computations.\n");
        }
    }

    args_t args = {
            aggregate,
            max_cop,
            workers
    };

    if (aggregate && (max_cop < 0)) {
        printf("The aggregate parameters require a cop number arguments. Aborting.");
        return 1;
    }

    struct stat path_info;
    if (0 == stat(path, &path_info)) {
        if (path_info.st_mode & S_IFDIR) {
            handle_folder(path, &args);
        } else {
            handle_file(path, &args);
        }
    } else {
        printf("The supplied file path is incorrect. Failure to start computation.\n");
        exit(1);
    }

    if (take_time) {
        time_t duration = time(NULL) - before;
        printf("Duration: %ld second(s)", duration);
    }

    return 0;
}
