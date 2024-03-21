#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// CSV reader config
#define MAX_LINE_LENGTH 1024
#define MAX_FIELDS 12
//

struct order {
  float pizza_id;
  float order_id;
  char pizza_name_id[20];
  float quantity;
  char order_date[10];
  char order_time[8];
  float unit_price;
  float total_price;
  char pizza_size[10];
  char pizza_category[20];
  char pizza_ingredients[100];
  char pizza_name[50];
};

void unique_pizza_names(
    int size, struct order *orders,
    struct {
      const char *pizza_name;
      float total_quantity;
    } pizza_totals[],
    int *num_unique_pizzas) {
  *num_unique_pizzas = 0;

  // Loop through the orders to calculate the total quantity for each pizza
  for (int i = 0; i < size; i++) {
    int j;
    // Check if this pizza type already exists in the pizza_totals array
    for (j = 0; j < *num_unique_pizzas; j++) {
      if (strcmp(orders[i].pizza_name, pizza_totals[j].pizza_name) == 0) {
        // If it exists, add the quantity to its total
        pizza_totals[j].total_quantity += orders[i].quantity;
        break;
      }
    }
    // If the pizza type doesn't exist, add it to the pizza_totals array
    if (j == *num_unique_pizzas) {
      pizza_totals[*num_unique_pizzas].pizza_name = orders[i].pizza_name;
      pizza_totals[*num_unique_pizzas].total_quantity = orders[i].quantity;
      (*num_unique_pizzas)++;
    }
  }
}

void unique_pizza_days(
    int size, struct order *orders,
    struct {
      const char *date;
      float total;
    } pizza_totals[],
    int *num_unique_days) {
  *num_unique_days = 0;

  // Loop through the orders to calculate the total sales for each day
  for (int i = 0; i < size; i++) {
    int j;
    // Check if this day already exists in the pizza_totals array
    for (j = 0; j < *num_unique_days; j++) {
      if (strcmp(orders[i].order_date, pizza_totals[j].date) == 0) {
        // If it exists, add the quantity to its total
        pizza_totals[j].total += orders[i].total_price;
        break;
      }
    }
    // If the day doesn't exist, add it to the pizza_totals array
    if (j == *num_unique_days) {
      pizza_totals[*num_unique_days].date = orders[i].order_date;
      pizza_totals[*num_unique_days].total = orders[i].total_price;
      (*num_unique_days)++;
    }
  }
}

// Metrics functions
void pms(int size, struct order *orders) {
  struct {
    const char *pizza_name;
    float total_quantity;
  } pizza_totals[size]; // Assuming a maximum number of pizzas
  int num_unique_pizzas;
  unique_pizza_names(size, orders, pizza_totals, &num_unique_pizzas);

  // Find the pizza with the highest total quantity
  float max_quantity = pizza_totals[0].total_quantity;
  const char *most_ordered_pizza = pizza_totals[0].pizza_name;
  for (int i = 1; i < num_unique_pizzas; i++) {
    if (pizza_totals[i].total_quantity > max_quantity) {
      max_quantity = pizza_totals[i].total_quantity;
      most_ordered_pizza = pizza_totals[i].pizza_name;
    }
  }

  printf("The most ordered pizza is '%s'.\n", most_ordered_pizza);
}

void pls(int size, struct order *orders) {
  struct {
    const char *pizza_name;
    float total_quantity;
  } pizza_totals[size]; // Assuming a maximum number of pizzas
  int num_unique_pizzas;
  unique_pizza_names(size, orders, pizza_totals, &num_unique_pizzas);

  // Find the pizza with the minimum total quantity
  float min_quantity = pizza_totals[0].total_quantity;
  const char *least_ordered_pizza = pizza_totals[0].pizza_name;
  for (int i = 1; i < num_unique_pizzas; i++) {
    if (pizza_totals[i].total_quantity < min_quantity) {
      min_quantity = pizza_totals[i].total_quantity;
      least_ordered_pizza = pizza_totals[i].pizza_name;
    }
  }

  printf("The least ordered pizza is '%s'.\n", least_ordered_pizza);
}

void dms(int size, struct order *orders) {
  struct {
    const char *date;
    float total;
  } pizza_totals[size]; // Assuming a maximum number of pizzas
  int num_unique_days;
  unique_pizza_days(size, orders, pizza_totals, &num_unique_days);

  // Find the day with the highest total quantity
  float max_total = pizza_totals[0].total;
  const char *most_ordered_date = pizza_totals[0].date;
  for (int i = 1; i < num_unique_days; i++) {
    if (pizza_totals[i].total > max_total) {
      max_total = pizza_totals[i].total;
      most_ordered_date = pizza_totals[i].date;
    }
  }

  printf("The most ordered date is %s with a total of $%.2f.\n",
         most_ordered_date, max_total);
}

void dls(int size, struct order *orders) {
  struct {
    const char *date;
    float total;
  } pizza_totals[size]; // Assuming a maximum number of pizzas
  int num_unique_days;
  unique_pizza_days(size, orders, pizza_totals, &num_unique_days);

  // Find the day with the highest total quantity
  float min_total = pizza_totals[0].total;
  const char *least_ordered_date = pizza_totals[0].date;
  for (int i = 1; i < num_unique_days; i++) {
    if (pizza_totals[i].total < min_total) {
      min_total = pizza_totals[i].total;
      least_ordered_date = pizza_totals[i].date;
    }
  }

  printf("The least ordered date is %s with a total of $%.2f.\n",
         least_ordered_date, min_total);
}

// Define function pointers and their names
void (*commandsFuncs[])(int, struct order *) = {pms, pls, dms, dls};
const char *commandsNames[] = {"pms", "pls", "dms", "dls"};

void execute_command(const char *command, int size, struct order *orders) {
  for (size_t i = 0; i < sizeof(commandsNames) / sizeof(commandsNames[0]);
       i++) {
    if (strcmp(command, commandsNames[i]) == 0) {
      commandsFuncs[i](size, orders);
      return;
    }
  }
  printf("Command '%s' not found.\n", command);
}

void read_csv(const char *filename, struct order **orders, int *size) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    printf("Error opening file %s.\n", filename);
    return;
  }

  char line[MAX_LINE_LENGTH];

  // Read the header and discard it
  if (fgets(line, sizeof(line), file) == NULL) {
    printf("Empty file.\n");
    fclose(file);
    return;
  }

  struct order *temp_orders = NULL;
  int count = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
    struct order ord;

    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }

    sscanf(line,
           "%f,%f,%20[^,],%f,%10[^,],%8[^,],%f,%f, "
           "%10[^,],%20[^,],\"%100[^\"]\",%50[^\n]",
           &ord.pizza_id, &ord.order_id, ord.pizza_name_id, &ord.quantity,
           ord.order_date, ord.order_time, &ord.unit_price, &ord.total_price,
           ord.pizza_size, ord.pizza_category, ord.pizza_ingredients,
           ord.pizza_name);

    // Reallocate memory for orders array
    temp_orders = realloc(temp_orders, (count + 1) * sizeof(struct order));
    if (temp_orders == NULL) {
      printf("Memory allocation failed.\n");
      fclose(file);
      return;
    }

    // Copy the current order to the orders array
    temp_orders[count] = ord;
    count++;
  }

  *orders = temp_orders;
  *size = count;

  fclose(file);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <filename> <command1> <command2> ...\n", argv[0]);
    return 1;
  }

  const char *fileName = argv[1];
  struct order *orders;
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

  // Free dynamically allocated memory
  free(orders);

  return 0;
}
