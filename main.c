#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HASHMAP */
#define MAX_HASH_SIZE 12

typedef enum { STRING, FLOAT } ValueType;

typedef struct {
  char *key;
  union {
    char *str_value;
    float float_value;
  } value;
  ValueType type;
} HashMapEntry;

typedef struct {
  HashMapEntry *entries;
  int capacity;
  int size;
} HashMap;

unsigned long hash(const char *key) {
  unsigned long hash = 5381;
  int c;

  while ((c = *key++) != 0) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

HashMap *createHashMap() {
  HashMap *map = malloc(sizeof(HashMap));
  if (!map)
    exit(EXIT_FAILURE);
  map->capacity = MAX_HASH_SIZE;
  map->size = 0;
  map->entries = calloc(map->capacity, sizeof(HashMapEntry));
  if (!map->entries)
    exit(EXIT_FAILURE);
  return map;
}

void resizeHashMap(HashMap *map) {
  int new_capacity = map->capacity * 2;
  HashMapEntry *new_entries =
      realloc(map->entries, new_capacity * sizeof(HashMapEntry));
  if (!new_entries)
    exit(EXIT_FAILURE);

  // Initialize newly allocated memory
  memset(new_entries + map->capacity, 0, map->capacity * sizeof(HashMapEntry));

  map->entries = new_entries;
  map->capacity = new_capacity;
}

void hashMapInsert(HashMap *map, const char *key, const void *value,
                   ValueType type) {
  if (map->size >= map->capacity)
    resizeHashMap(map);
  unsigned long index = hash(key) % map->capacity;
  while (map->entries[index].key != NULL &&
         strcmp(map->entries[index].key, key) != 0) {
    index = (index + 1) % map->capacity;
  }
  map->entries[index].key = strdup(key);
  if (type == STRING)
    map->entries[index].value.str_value = strdup(value);
  else
    map->entries[index].value.float_value = *(float *)value;
  map->entries[index].type = type;
  map->size++;
}

const char *hashMapGetString(HashMap *map, const char *key) {
  unsigned long index = hash(key) % map->capacity;
  while (map->entries[index].key != NULL) {
    if (strcmp(map->entries[index].key, key) == 0 &&
        map->entries[index].type == STRING) {
      return map->entries[index].value.str_value;
    }
    index = (index + 1) % map->capacity;
  }
  return NULL;
}

float hashMapGetFloat(HashMap *map, const char *key) {
  unsigned long index = hash(key) % map->capacity;
  while (map->entries[index].key != NULL) {
    if (strcmp(map->entries[index].key, key) == 0 &&
        map->entries[index].type == FLOAT) {
      return map->entries[index].value.float_value;
    }
    index = (index + 1) % map->capacity;
  }
  return 0.0f;
}

const char *getHighestValueKey(HashMap *map) {
  float highest_value = 0.0f; // Assuming all values are positive
  const char *highest_value_key = NULL;

  for (int i = 0; i < map->capacity; i++) {
    if (map->entries[i].key != NULL && map->entries[i].type == FLOAT) {
      float current_value = map->entries[i].value.float_value;
      if (current_value > highest_value) {
        highest_value = current_value;
        highest_value_key = map->entries[i].key;
      }
    }
  }

  return highest_value_key;
}

const char *getLowestValueKey(HashMap *map) {
  float lowest_value;
  const char *lowest_value_key = NULL;

  for (int i = 0; i < map->capacity; i++) {
    if (map->entries[i].key != NULL && map->entries[i].type == FLOAT) {
      float current_value = map->entries[i].value.float_value;
      if (lowest_value_key == NULL || current_value < lowest_value) {
        lowest_value = current_value;
        lowest_value_key = map->entries[i].key;
      }
    }
  }

  return lowest_value_key;
}

const char *getAverageValue(HashMap *map) {
  float all_values = 0.0f;
  int size = 0;

  for (int i = 0; i < map->capacity; i++) {
    if (map->entries[i].key != NULL && map->entries[i].type == FLOAT) {
      all_values += map->entries[i].value.float_value;
      size += 1;
    }
  }

  const char *mean;

  char float_to_str[20];
  sprintf(float_to_str, "%0.1f", all_values / size);
  mean = float_to_str;

  return mean;
}

void freeHashMap(HashMap *map) {
  for (int i = 0; i < map->capacity; i++) {
    if (map->entries[i].key != NULL) {
      free(map->entries[i].key);
      if (map->entries[i].type == STRING)
        free(map->entries[i].value.str_value);
    }
  }
  free(map->entries);
  free(map);
}
/***************/

/* METRIC FUNCTIONS */
void processOrderFilter(int size, HashMap **orders, const char *message,
                        const char *most_or_least, const char *filter,
                        const char *key, bool extra_ptr) {
  HashMap *unique = createHashMap();

  for (int i = 0; i < size; i++) {
    float value = hashMapGetFloat(orders[i], key);

    const char *filter_key;
    if (strcmp(filter, "order_id") != 0) {
      filter_key = hashMapGetString(orders[i], filter);
    } else {
      char float_to_str[20];
      sprintf(float_to_str, "%0.0f", hashMapGetFloat(orders[i], filter));
      filter_key = float_to_str;
    }

    if (hashMapGetFloat(unique, filter_key) != 0.0f) {
      value += hashMapGetFloat(unique, filter_key);
    }

    hashMapInsert(unique, filter_key, &value, FLOAT);
  }

  const char *extreme_value;
  if (strcmp(most_or_least, "most") == 0) {
    extreme_value = getHighestValueKey(unique);
  } else if (strcmp(most_or_least, "least") == 0) {
    extreme_value = getLowestValueKey(unique);
  } else if (strcmp(most_or_least, "average") == 0) {
    extreme_value = getAverageValue(unique);
  }

  if (extra_ptr == false) {
    printf(message, extreme_value);
  } else {
    float extra_val = hashMapGetFloat(unique, extreme_value);
    printf(message, extreme_value, extra_val);
  }

  printf("\n");

  freeHashMap(unique);
}

void pms(int size, HashMap **orders) {
  processOrderFilter(size, orders, "The most ordered pizza is %s.", "most",
                     "pizza_name", "quantity", false);
}

void pls(int size, HashMap **orders) {
  processOrderFilter(size, orders, "The least ordered pizza is %s.", "least",
                     "pizza_name", "quantity", false);
}

void dms(int size, HashMap **orders) {
  processOrderFilter(
      size, orders,
      "The date with the most revenue is %s with a total of $%.2f.", "most",
      "order_date", "total_price", true);
}

void dls(int size, HashMap **orders) {
  processOrderFilter(
      size, orders,
      "The date with the least revenue is %s with a total of $%.2f.", "least",
      "order_date", "total_price", true);
}

void dmsp(int size, HashMap **orders) {
  processOrderFilter(
      size, orders,
      "The date with the most sold pizzas is %s with a total of %.0f.", "most",
      "order_date", "quantity", true);
}
void dlsp(int size, HashMap **orders) {
  processOrderFilter(
      size, orders,
      "The date with the least sold pizzas is %s with a total of %.0f.",
      "least", "order_date", "quantity", true);
}

void apo(int size, HashMap **orders) {
  processOrderFilter(size, orders,
                     "The average ordered pizzas per order is %s.", "average",
                     "order_id", "quantity", false);
}
void apd(int size, HashMap **orders) {
  processOrderFilter(size, orders, "The average ordered pizzas per day is %s.",
                     "average", "order_date", "quantity", false);
}

void ims(int size, HashMap **orders) {
  HashMap *unique = createHashMap();

  for (int i = 0; i < size; i++) {
    float quantity = hashMapGetFloat(orders[i], "quantity");
    char *token =
        strtok(strdup(hashMapGetString(orders[i], "pizza_ingredients")), ",");

    while (token != NULL) {
      // Trim leading spaces
      while (*token == ' ')
        token++;

      float amount = quantity;
      if (hashMapGetFloat(unique, token) != 0.0f) {
        amount += hashMapGetFloat(unique, token);
      }

      hashMapInsert(unique, token, &amount, FLOAT);

      token = strtok(NULL, ",");
    }
  }

  const char *most_ordered_ingredient = getHighestValueKey(unique);
  printf("The most ordered ingredient is %s.\n", most_ordered_ingredient);

  freeHashMap(unique);
}
void hp(int size, HashMap **orders) {
  processOrderFilter(size, orders, "The most ordered pizza category is %s.",
                     "most", "pizza_category", "quantity", false);
}
/***************/

/* Define function pointers and their names */
void (*commandsFuncs[])(int, HashMap **orders) = {pms,  pls, dms, dls, dmsp,
                                                  dlsp, apo, apd, ims, hp};
const char *commandsNames[] = {"pms",  "pls", "dms", "dls", "dmsp",
                               "dlsp", "apo", "apd", "ims", "hp"};

void execute_command(const char *command, int size, HashMap **orders) {
  for (size_t i = 0; i < sizeof(commandsNames) / sizeof(commandsNames[0]) + 1;
       i++) {
    if (strcmp(command, commandsNames[i]) == 0) {
      commandsFuncs[i](size, orders);
      return;
    }
  }
  printf("Command '%s' not found.\n", command);
}
/***************/

/* CSV FILE READER */
void read_csv(const char *filename, HashMap ***orders, int *size) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Error opening file %s.\n", filename);
    return;
  }

  char line[1024];

  // Read the header and discard it
  if (fgets(line, sizeof(line), file) == NULL) {
    printf("Empty file.\n");
    fclose(file);
    return;
  }

  *orders =
      malloc(sizeof(HashMap *) *
             MAX_HASH_SIZE); // Allocate memory for array of hashmap pointers
  if (!(*orders))
    exit(EXIT_FAILURE);

  int index = 0;
  while (fgets(line, sizeof(line), file) != NULL) {
    (*orders)[index] = createHashMap(); // Create a new hashmap for each line

    char pizza_name_id[50];
    char order_date[12];
    char order_time[10];
    char pizza_size[2];
    char pizza_category[50];
    char pizza_ingredients[100];
    char pizza_name[50];
    float pizza_id, order_id, quantity, unit_price, total_price;

    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }

    sscanf(line,
           "%f,%f,%[^,],%f,%[^,],%[^,],%f,%f,%[^,],%[^,],\"%[^\"]\",%[^\n]",
           &pizza_id, &order_id, pizza_name_id, &quantity, order_date,
           order_time, &unit_price, &total_price, pizza_size, pizza_category,
           pizza_ingredients, pizza_name);

    hashMapInsert((*orders)[index], "pizza_id", &pizza_id, FLOAT);
    hashMapInsert((*orders)[index], "order_id", &order_id, FLOAT);
    hashMapInsert((*orders)[index], "pizza_name_id", pizza_name_id, STRING);
    hashMapInsert((*orders)[index], "quantity", &quantity, FLOAT);
    hashMapInsert((*orders)[index], "order_date", order_date, STRING);
    hashMapInsert((*orders)[index], "order_time", order_time, STRING);
    hashMapInsert((*orders)[index], "unit_price", &unit_price, FLOAT);
    hashMapInsert((*orders)[index], "total_price", &total_price, FLOAT);
    hashMapInsert((*orders)[index], "pizza_size", pizza_size, STRING);
    hashMapInsert((*orders)[index], "pizza_category", pizza_category, STRING);
    hashMapInsert((*orders)[index], "pizza_ingredients", pizza_ingredients,
                  STRING);
    hashMapInsert((*orders)[index], "pizza_name", pizza_name, STRING);

    index++;
  }

  *size = index; // Set the size to the number of hashmap pointers created

  fclose(file);
}
/***************/

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <filename> <command1> <command2> ...\n", argv[0]);
    return 1;
  }

  const char *fileName = argv[1];
  HashMap **orders;
  int size;

  read_csv(fileName, &orders, &size);

  if (size == 0) {
    printf("No orders were placed.\n");
    return 1;
  }

  // Execute commands
  for (int arg = 2; arg < argc; arg++) {
    execute_command(argv[arg], size, orders);
  }

  // Free memory
  for (int i = 0; i < size; i++) {
    freeHashMap(orders[i]);
  }
  free(orders);

  return 0;
}
