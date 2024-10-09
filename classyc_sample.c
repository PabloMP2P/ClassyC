
// classyc_sample.c - A sample program to showcase the ClassyC library 2.0

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Some (optional) macros can be used to configure the library naming conventions
// In this case, we are using the default ones: CLASS, CLASS_ and I_, so the
// following lines are not needed and are just for demonstration purposes.
#define CLASSYC_CLASS_NAME CLASS
#define CLASSYC_CLASS_IMPLEMENT CLASS_
#define CLASS_OBJECT(Base, Interface, Data, Event, Method, Override)
#define CLASSYC_INTERFACE_DECLARATION I_

#include "ClassyC.h"

// Interfaces 
#define I_Moveable(Data, Event, Method) \
    Data(int, position) \
    Event(on_move, int distance_moved) \
    Method(void, move, int speed, int distance)
CREATE_INTERFACE(Moveable)                        

#define I_Sellable(Data, Event, Method) \
    Data(int, id) \
    Method(int, estimate_price)
CREATE_INTERFACE(Sellable)

// Classes 
#undef CLASS
#define CLASS Vehicle
#define CLASS_Vehicle(Base, Interface, Data, Event, Method, Override)\
    Base(OBJECT) Interface(Sellable) Interface(Moveable) \
    Data(int, id) \
    Data(int, position) \
    Event(on_move, int distance_moved) \
    Method(int, estimate_price) \
    Method(void, move, int speed, int distance)

CONSTRUCTOR() END_CONSTRUCTOR
DESTRUCTOR() END_DESTRUCTOR

METHOD(int, estimate_price)
    return 1000;
END_METHOD
METHOD(void, move, int speed, int distance)
    printf("Moving vehicle %d units at %d speed\n", distance, speed);
    self->position += distance;
    RAISE_EVENT(self, on_move, distance);
END_METHOD


#undef CLASS
#define CLASS Car
#define CLASS_Car(Base, Interface, Data, Event, Method, Override) \
    Base(Vehicle) \
    Data(int, km_total) \
    Data(int, km_since_last_fuel) \
    Event(on_need_fuel, int km_to_collapse) \
    Override(int, estimate_price) \
    Override(void, move, int speed, int distance)

CONSTRUCTOR(int km_total_when_bought)
    INIT_BASE();
    self->position = 0;
    self->km_total = km_total_when_bought;
    self->km_since_last_fuel = 0;
END_CONSTRUCTOR                     
DESTRUCTOR() END_DESTRUCTOR
METHOD(void, move, int speed, int distance)
    printf("Moving car %d units at %d speed\n", distance, speed);
    self->position += distance;
    self->km_since_last_fuel += distance;
    int km_to_collapse = 400 - self->km_since_last_fuel;
    if (km_to_collapse < 100) {
        RAISE_EVENT(self, on_need_fuel, km_to_collapse);
    }
END_METHOD
METHOD(int, estimate_price)
    return 15000;
END_METHOD

#undef CLASS
#define CLASS Elephant
#define CLASS_Elephant(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) Interface(Moveable) \
    Data(int, position) \
    Event(on_move, int distance_moved) \
    Method(void, move, int speed, int distance)

CONSTRUCTOR() END_CONSTRUCTOR
DESTRUCTOR() END_DESTRUCTOR
METHOD(void, move, int speed, int distance)
    self->position += distance;
END_METHOD


#undef CLASS
#define CLASS TinyClass
#define CLASS_TinyClass(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT)
CONSTRUCTOR() END_CONSTRUCTOR
DESTRUCTOR() END_DESTRUCTOR


// Interfaces can be used as types, and they contain pointers to the object members
void swap_movables_position(Moveable object1, Moveable object2) {
    // interface members are pointers and must be dereferenced to access the actual values.
    printf(" Moveable object1 -> %d in address %p \n", *object1.position, object1.position);
    printf(" Moveable object2 -> %d in address %p \n", *object2.position, object2.position);
    int distance_moved = abs(*object1.position - *object2.position);
    int temp = *object1.position;
    *object1.position = *object2.position;
    *object2.position = temp;
    RAISE_INTERFACE_EVENT(object1, on_move, distance_moved);
    RAISE_INTERFACE_EVENT(object2, on_move, distance_moved);
}

// Event handlers are functions that handle events 
EVENT_HANDLER(Car, on_need_fuel, mycar_lowfuel, int km_to_collapse)
    if (km_to_collapse < 10) {
        printf("Fuel level critical! Need to refuel in less than %d km!\n", km_to_collapse);
    }
END_EVENT_HANDLER

int main(void){
    Car *my_car = CREATE(Car, 10000);  // Create a Car object
    if (!my_car) {
        fprintf(stderr, "Failed to create Car object.\n");
        return EXIT_FAILURE;
    }
    REGISTER_EVENT(Car, on_need_fuel, mycar_lowfuel, my_car); // Register the event handler
    my_car->km_total = 10000; // Access to fields using the data pointer
    my_car->move(my_car, 100, 395); // This will fire the on_need_fuel event

    my_car->id=79;
    printf("\nAccessing the same id from different contexts\n");
    printf("my_car->id %d\n", my_car->id);
    printf("*my_car->as_Sellable.id %d\n", *my_car->as_Sellable.id);
    printf("(Vehicle *)my_car->id %d\n", ((Vehicle *)my_car)->id);
    printf("*((Vehicle *)my_car)->as_Sellable.id %d\n", *((Vehicle *)my_car)->as_Sellable.id); 

    printf("\nAccessing the same method from different contexts\n");
    Vehicle *my_car_as_vehicle = (Vehicle *)my_car; // Casting works as expected: same methods, same fields
    printf("estimate_price:  my_car: %d\n", my_car->estimate_price(my_car));
    printf("estimate_price:  my_car_as_vehicle: %d\n", my_car_as_vehicle->estimate_price(my_car_as_vehicle));
    printf("estimate_price:  *my_car->as_Sellable %d\n", my_car->as_Sellable.estimate_price(my_car->as_Sellable.self));
    printf("estimate_price:  *my_car_as_vehicle->as_Sellable %d\n", my_car_as_vehicle->as_Sellable.estimate_price(my_car_as_vehicle->as_Sellable.self));
   
    printf("\nUsing the interface for polymorphism\n");
    Elephant *my_elephant = CREATE(Elephant);
    if (!my_elephant) {
        fprintf(stderr, "Failed to create Elephant object.\n");
        return EXIT_FAILURE;
    }
    my_elephant->position = 24;
    printf("Positions ->  my_car: %d, my_elephant: %d\n", my_car->position, my_elephant->position);
    swap_movables_position(my_car->as_Moveable, my_elephant->as_Moveable); // Using the interface for polymorphism
    printf("Positions ->  my_car: %d, my_elephant: %d\n", my_car->position, my_elephant->position);
    
    printf("\nUsing the interface for polymorphism in casted context\n");
    printf("Positions ->  (Vehicle *)my_car: %d, my_elephant: %d\n", ((Vehicle *)my_car)->position, my_elephant->position);
    swap_movables_position(((Vehicle *)my_car)->as_Moveable, my_elephant->as_Moveable); // Using the interface for polymorphism
    printf("Positions ->  (Vehicle *)my_car: %d, my_elephant: %d\n", ((Vehicle *)my_car)->position, my_elephant->position);

    printf("\nCreating a large number of objects\n");
    clock_t start_time = clock();
    size_t num_objects = 1000;
    int iterations = 100;
    for (int iter = 0; iter < iterations; iter++) {
        TinyClass **tiny_objects = (TinyClass **)malloc(num_objects * sizeof(TinyClass *));
        if (tiny_objects == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }
        for (size_t i = 0; i < num_objects; i++) {
            tiny_objects[i] = CREATE(TinyClass);
            if (!tiny_objects[i]) {
                fprintf(stderr, "Failed to create TinyClass object.\n");
                return EXIT_FAILURE;
            }
        }
        for (size_t i = 0; i < num_objects; i++) {
            DESTROY(tiny_objects[i]);
            tiny_objects[i] = NULL;
        }
        free(tiny_objects);
    }

    clock_t end_time = clock();
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Time taken to create and destroy %zu objects of size %zu: %f seconds. (%.2f objects/second)\n", num_objects * iterations, sizeof(TinyClass), cpu_time_used, (num_objects * iterations) / cpu_time_used);

    DESTROY(my_car);
    my_car = NULL;
    DESTROY(my_elephant);
    my_elephant = NULL;

    printf("Program finished.\n");
    return 0;
}