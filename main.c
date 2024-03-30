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

  for (int i = 1; i < map->capacity; i++) {
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
void pms(int size, HashMap **orders) {
  HashMap *unique_pizzas = createHashMap();

  for (int i = 0; i < size; i++) {
    float quantity = hashMapGetFloat(orders[i], "quantity");
    const char *pizza_name = hashMapGetString(orders[i], "pizza_name");

    if (hashMapGetFloat(unique_pizzas, pizza_name) == 0.0f) {
      hashMapInsert(unique_pizzas, pizza_name, &quantity, FLOAT);
    } else {
      float quantity_ptr = hashMapGetFloat(unique_pizzas, pizza_name);
      quantity_ptr += quantity;
      hashMapInsert(unique_pizzas, pizza_name, &quantity_ptr, FLOAT);
    }
  }

  const char *most_ordered_pizza = getHighestValueKey(unique_pizzas);
  printf("The most ordered pizza is '%s'.\n", most_ordered_pizza);

  freeHashMap(unique_pizzas);
}

void pls(int size, HashMap **orders) {
  HashMap *unique_pizzas = createHashMap();

  for (int i = 0; i < size; i++) {
    float quantity = hashMapGetFloat(orders[i], "quantity");
    const char *pizza_name = hashMapGetString(orders[i], "pizza_name");

    if (hashMapGetFloat(unique_pizzas, pizza_name) == 0.0f) {
      hashMapInsert(unique_pizzas, pizza_name, &quantity, FLOAT);
    } else {
      float quantity_ptr = hashMapGetFloat(unique_pizzas, pizza_name);
      quantity_ptr += quantity;
      hashMapInsert(unique_pizzas, pizza_name, &quantity_ptr, FLOAT);
    }
  }

  const char *least_ordered_pizza = getLowestValueKey(unique_pizzas);
  printf("The least ordered pizza is '%s'.\n", least_ordered_pizza);

  freeHashMap(unique_pizzas);
}

void dms(int size, HashMap **orders) {}

void dls(int size, HashMap **orders) {}
/***************/

/* Define function pointers and their names */
void (*commandsFuncs[])(int, HashMap **orders) = {pms, pls, dms, dls};
const char *commandsNames[] = {"pms", "pls", "dms", "dls"};

void execute_command(const char *command, int size, HashMap **orders) {
  for (size_t i = 0; i < sizeof(commandsNames) / sizeof(commandsNames[0]);
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

    char pizza_name_id[20];
    char order_date[10];
    char order_time[8];
    char pizza_size[10];
    char pizza_category[20];
    char pizza_ingredients[100];
    char pizza_name[50];
    float pizza_id, order_id, quantity, unit_price, total_price;

    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }

    sscanf(line,
           "%f,%f,%20[^,],%f,%10[^,],%8[^,],%f,%f, "
           "%10[^,],%20[^,],\"%100[^\"]\",%50[^\n]",
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