/*
 * csim.c - A cache simulator that can replay traces from Valgrind
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is LRU.
 *
 * Implementation and assumptions:
 *
 *  1. Each load/store can cause at most one cache miss. (I examined the trace,
 *  the largest request I saw was for 8 bytes).
 *
 *  2. Instruction loads (I) are ignored, since we are interested in evaluating
 *  data cache performance.
 *
 *  3. data modify (M) is treated as a load followed by a store to the same
 *  address. Hence, an M operation can result in two cache hits, or a miss and a
 *  hit plus an possible eviction.
 *
 * The function printSummary() is given to print output.
 * Please use this function to print the number of hits, misses and evictions.
 * IMPORTANT: This is crucial for the driver to evaluate your work.
 *
 */
//Sushruti Bansod
//sdb88

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

//#define DEBUG_ON
#define ADDRESS_LENGTH 64

/* Type: Memory address */
typedef unsigned long long int mem_addr_t;

/*
 * Data structures to represent the cache we are simulating
 *
 * TODO: Define your own!
 *
 * E.g., Types: cache, cache line, cache set
 * (you can use a counter to implement LRU replacement policy)
 */
 typedef struct cache_line{
   char valid;
   mem_addr_t tag;

   int counter;
   struct cache_line * next;
 }cache_line_t;

//create a pointer to an array
//the array is made up of cachlines and is located in the set
 typedef cache_line_t* cache_set_t;

 //now create the pointer to the array of sets which is IN THE cache
 typedef cache_set_t* cache_t;



/* Globals set by command line args */
int verbosity = 0; /* print trace if set */
int s = 0; /* set index bits */
int b = 0; /* block offset bits */
int E = 0; /* associativity */
char* trace_file = NULL;

/* Derived from command line args */
int S; /* number of sets */
int B; /* block size (bytes) */

/* Counters used to record cache statistics */
int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;
unsigned long long int lru_counter = 1;


//the cache that we are simulating through this lab
cache_t cache;

/*
 * initCache - Allocate memory (with malloc) for cache data structures (i.e., for each of the sets and lines per set),
 * writing 0's for valid and tag and LRU
 *
 * TODO: Implement
 *
 */
void initCache()
{
  //we know s is 0
  // so lets fix that
  S = (1 << s);
  B = (1 << b);

  //incrementing variables
  int i;
  int j;

  cache = (cache_set_t*)malloc(sizeof(cache_set_t)* S);
  //so basically this is fnding spcae for s sets within the entire cache
  //helps to think of it like a matrix
  for(i = 0; i < S; i++)
  {
    cache[i] = malloc(sizeof(cache_line_t) * E);
    //part 2 of matrix
    //finds the space for e cachelines in EACH sets
    //reminder that we have S sets
    for(j = 0; j < E; j++)
    {//sets all the vals to 0
      cache[i][j].valid = 0;
      cache[i][j].tag = 0;
      cache[i][j].counter = 0;
      cache[i][j].next  = NULL;

      if(i > 0 && j == 0) //tis mean like the cache is just starting now
      {
        //This sons high brain shit lmao
        //basically we want to set the next of the previous item to this...
        //java would be curr.previous.next = curr; except with the specific addy of curr
        cache[i-1][E-1].next = &cache[i][j];
      }else if(j > 0)
      {
        //this will run every other time than the first becuase
        //we are not at the first line of the catset
        //similar to the previous one but just define a straight line of next
        cache[i][j-1].next = &cache[i][j];
      }
    }
  }

}


/*
 * freeCache - free allocated memory
 *
 * This function deallocates (with free) the cache data structures of each
 * set and line.
 *
 * TODO: Implement
 */
void freeCache()
{
  //go through each 'index' of the cache and make that shit null and free it

  for(int i = 0; i < E; i++)
  {//frees each specific spot
    free(*(cache + i)); //frees the thing that the cache set is pointing to
  }
  //frees the entire cache
  //remeber free before setting to null otherwise error
  free(cache);
  cache = NULL;
}


/*
 * accessData - Access data at memory address addr
 *   If it is already in cache, increase hit_count
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase eviction_count if a line is evicted.
 *
 * TODO: Implement
 *///USE THE ABOVE IFS AS IF STATEMENTS!!!
void accessData(mem_addr_t addr)
{
  //first lets fet the right addys

  //to calculate the addy of the firstbit of s
  mem_addr_t bit_s = addr >> b;
  //to calculate addy of t bit the tag
  mem_addr_t bit_t = addr >> (b+s);

  //to calculate the set and tag of this specific addy
  ///This is creating the mask so we want a certain num of ones
  //so that power does that
  mem_addr_t set = bit_s & ((int) pow (2, s) - 1); //the -1 gives the right val?
  mem_addr_t tag = bit_t & ((int)pow(2, 64 - (b+s)) - 1);


  //setting min and max values
  int min = INT_MAX;
  int max = 0;

  //this part is going to search the set for the largest counter val
  //by setting it to max we can use it in the next part to calculate the hit_count;
  for(int i = 0; i < E; i++)
  {
    //mem_addr_t temp = (mem_addr_t)(*(cache + set)+i);
    //Remeber what hoffman said, when you create a temp var, youre only
    //pointing it to the things now creating a whole new instantce
    //this is why when i did temp it didnt recognize the ->
    //because its like the .next of the things its pointing to which is wrong
    //so the best way would be to just do the cache+set pointer everytime
    if((*(cache + set)+i)->counter > max && (*(cache + set)+i)->valid == 1)
    {
      max = (*(cache + set)+i)->counter;
    }
  }

 //If it is already in cache, increase hit_count
  //this goes through to E and checks if there is a hit.
  //hit means counter is greater than max and isvalid.
  //if there is then increments hit-count and moves the pointer to line forward
  for(int i = 0; i < E; i++)
  {
      if((*(cache + set)+i)-> tag == tag && (*(cache + set)+i)->valid == 1)
      {
        (*(cache + set)+i)->counter = max +1; //increment the Counter
        hit_count++; //increment hitcoutn
        return; //this hits the first thing in the method todo!!
      }
  }

  //If it is not in cache, bring it in cache, increase miss count.
  //check for miss, and then set the line
  for(int i = 0; i < E; i++)
  {
    if((*(cache + set)+i)->valid == 0)
    {
      (*(cache + set)+i)->tag = tag; //think about the java, like temp.val = this.val
      (*(cache + set)+i)->valid = 1; //set it as valid cuz not its a hit
      (*(cache + set)+i)->counter= max + 1;
      miss_count++;
      return;
    }
  }

  //Also increase eviction_count if a line is evicted
  //so find the eviction place, and the lower counter val and rhen replace
  //the old one
  for(int i = 0; i < E; i++)
  {
    if((*(cache + set)+i)->counter < min && (*(cache + set)+i)->valid == 1)
    {
      min = (*(cache + set)+i)->counter;
    }
  }

//this is going to evict the line
  for(int i = 0; i < E; i++)
  {
    if((*(cache + set)+i)->counter == min)
    {
      (*(cache + set)+i)->tag = tag;
      (*(cache + set)+i)->valid = 1;
      (*(cache + set)+i)->counter = max + 1;
      miss_count++;
      eviction_count++;
      return;
    }
  }
}


/*
 * replayTrace - replays the given trace file against the cache
 *
 * This function:
 * - opens file trace_fn for reading (using fopen)
 * - reads lines (e.g., using fgets) from the file handle (may name `trace_fp` variable)
 * - skips lines not starting with ` S`, ` L` or ` M`
 * - parses the memory address (unsigned long, in hex) and len (unsigned int, in decimal)
 *   from each input line
 * - calls `access_data(address)` for each access to a cache line
 *
 * TODO: Implement
 *
 */
void replayTrace(char* trace_fn)
{
  ////this shit gonna hurt like a mother fucker
  //so in this file ik im gonna have to use fget and put or some shit
  // am allowed to search online for that!!!!
  //most of them are simialr to the java but the params can be VERY different

  FILE *trace_fp = fopen(trace_fn, "r"); //the fopen opns a file to make it readable


  if(!trace_fp) // this checks for its nullness so basically a try catch for a null pointer
  {
    fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno)); //print makes it writeable
       exit(1);
  }

  char buffer[1000]; //this will be the car array so that the replay is possible

  mem_addr_t addr = 0;
  int size = 0;

//char *fgets(char *str, int n, FILE *stream)
// reads a line from the specified stream and stores it into the string pointed to by str
  while(fgets(buffer,1000,trace_fp) != NULL)
  {
    if(buffer[1] == 'S' || buffer[1] == 'L' || buffer[1] == 'M') //checks the buffer index for SLM
    {
      // int sscanf(const char *str, const char *format, ...) reads formatted input from a string.
      sscanf(buffer+3, "%llx,%u", &addr, &size);

      if(verbosity) //important to check for verbos so we know when to print set
      {
        printf("%c %llx, %u ", buffer[1], addr, size );
      }
      accessData(addr); //code he gave us lmao

      if(buffer[1] == 'M') //debug bt it worked.
      {
        accessData(addr);
      }

      if(verbosity)
      {
        printf("\n");
      }
    }
  }

//closes the file when done reading it
  fclose(trace_fp);
}

/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[])
{
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 *
 * !! DO NOT MODIFY !!
 *
 * printSummary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded.
 */
void printSummary(int hits, int misses, int evictions)
{
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

/*
 * main - Main routine
 */
int main(int argc, char* argv[])
{
    char c;

    while( (c=getopt(argc,argv,"s:E:b:t:vh")) != -1){
        switch(c){
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            trace_file = optarg;
            break;
        case 'v':
            verbosity = 1;
            break;
        case 'h':
            printUsage(argv);
            exit(0);
        default:
            printUsage(argv);
            exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    /* Compute S, E and B from command line args */
    S = (unsigned int) pow(2, s);
    B = (unsigned int) pow(2, b);

    /* Initialize cache */
    initCache();

#ifdef DEBUG_ON
    printf("DEBUG: S:%u E:%u B:%u trace:%s\n", S, E, B, trace_file);
#endif

    replayTrace(trace_file);

    /* Free allocated memory */
    freeCache();

    /* Output the hit and miss statistics for the autograder */
    printSummary(hit_count, miss_count, eviction_count);

    return 0;
}
