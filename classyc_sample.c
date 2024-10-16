// classyc_sample.c - A sample program to showcase the ClassyC library

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Some (optional) macros can be used to configure the library naming conventions
// In this case, we are using the default ones: CLASS, CLASS_ and I_, so the
// following lines are not needed and are just for demonstration purposes.
// #define CLASSYC_CLASS_NAME CLASS
// #define CLASSYC_CLASS_IMPLEMENT CLASS_
// #define CLASS_OBJECT(Base, Interface, Data, Event, Method, Override) \
//     Data(void, DESTRUCTOR_PTR)
// #define CLASSYC_INTERFACE_DECLARATION I_

#include "ClassyC.h"

// Number of objects to create and destroy in the speed test
#define TEST_NUM_OBJECTS 100000ull
// We'll count instances created and destroyed to show how auto-destruction works to prevent memory leaks
size_t num_objects_created = 0;
size_t num_objects_destroyed = 0;

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
CONSTRUCTOR() 
    if (!is_base) num_objects_created++;
END_CONSTRUCTOR
DESTRUCTOR() 
    if (!is_base) num_objects_destroyed++;
END_DESTRUCTOR
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
    Method(void, park) \
    Override(int, estimate_price) \
    Override(void, move, int speed, int distance)
CONSTRUCTOR(int km_total_when_bought)
    INIT_BASE();
    self->position = 0;
    self->km_total = km_total_when_bought;
    self->km_since_last_fuel = 0;
    if (!is_base) num_objects_created++;
END_CONSTRUCTOR                     
DESTRUCTOR() 
    if (!is_base) num_objects_destroyed++;
END_DESTRUCTOR
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
METHOD(void, park)
    self->position = 0;
END_METHOD

#undef CLASS
#define CLASS Elephant
#define CLASS_Elephant(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) Interface(Moveable) \
    Data(int, position) \
    Event(on_move, int distance_moved) \
    Method(void, move, int speed, int distance)
CONSTRUCTOR() 
    if (!is_base) num_objects_created++;
END_CONSTRUCTOR
DESTRUCTOR()
    if (!is_base) num_objects_destroyed++;
END_DESTRUCTOR
METHOD(void, move, int speed, int distance)
    self->position += distance;
END_METHOD

#undef CLASS
#define CLASS TinyClass
#define CLASS_TinyClass(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) 
CONSTRUCTOR() 
    if (!is_base) num_objects_created++;
END_CONSTRUCTOR
DESTRUCTOR() 
    if (!is_base) num_objects_destroyed++;
END_DESTRUCTOR

// Interfaces can be used as types, and they contain pointers to the object members
void swap_movables_position(Moveable object1, Moveable object2) {
    // interface members are pointers and must be dereferenced to access the actual values.
    int distance_moved = abs(*object1.position - *object2.position);
    int temp = *object1.position;
    *object1.position = *object2.position;
    *object2.position = temp;
    // Events can be raised through interfaces as well. If no event handler is registered, it will have no effect.
    RAISE_INTERFACE_EVENT(object1, on_move, distance_moved);
    RAISE_INTERFACE_EVENT(object2, on_move, distance_moved);
}

// Event handlers are functions that handle events 
EVENT_HANDLER(Car, on_need_fuel, mycar_lowfuel, int km_to_collapse)
    if (km_to_collapse < 10) {
        printf("Fuel level critical! Need to refuel in less than %d km!\n", km_to_collapse);
    }
END_EVENT_HANDLER

EVENT_HANDLER(Car, on_move, mycar_move, int distance_moved)
    printf("EVENT HANDLER: Car moved %d units\n", distance_moved);
END_EVENT_HANDLER

void successful_exit(void) {
    printf("\nAt program exit: %zu objects created, %zu objects destroyed.\n", num_objects_created, num_objects_destroyed);
    if (num_objects_created != num_objects_destroyed) {
        printf("%zu objects remaining, memory leaks present.\n", num_objects_created - num_objects_destroyed);
    } else {
        printf("Program finished successfully.\n");
    }
}

void create_objects_inside_function(void) {
    // This function is used to test the automatic destruction of objects when they go out of scope 
    CREATE_HEAP(Car, local_heap_car, 200);
    CREATE_STACK(Car, local_stack_car, 200);

    printf("Inside function, the objects have been created.\n"); 

#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0
    // Compiler doesn't support auto-destruction; destroy the objects manually
    DESTROY_FREE(local_heap_car);
    DESTROY(local_stack_car);
#endif
    printf("Leaving function scope.\n");
    return;
}

int main(void){
    atexit(successful_exit);
    printf("Creating a Car object on the heap\n");

    AUTODESTROY_PTR(Car) *my_car = NEW_ALLOC(Car, 10000);
    // Alternatively, we could have used CREATE_HEAP(Car, my_car, 10000); 

    if (!my_car) {
        fprintf(stderr, "Failed to create Car object.\n");
        return EXIT_FAILURE;
    }
    REGISTER_EVENT(Car, on_need_fuel, mycar_lowfuel, my_car); // Register the event handler
    REGISTER_EVENT(Car, on_move, mycar_move, my_car); // Register the event handler
    printf("Event registered\n");
    my_car->km_total += 120; // Access to data fields
    my_car->move(my_car, 100, 395); // This will fire the on_need_fuel event

    my_car->id=79;
    printf("\nAccessing the same id from different contexts\n");
    printf("%d in my_car->id\n", my_car->id);
    printf("%d in *my_car->to_Sellable(my_car).id\n", *my_car->to_Sellable(my_car).id);
    printf("%d in (Vehicle *)my_car->id\n", ((Vehicle *)my_car)->id);
    printf("%d in *((Vehicle *)my_car)->to_Sellable((Vehicle *)my_car).id\n", *((Vehicle *)my_car)->to_Sellable((Vehicle *)my_car).id); 

    printf("\nAccessing the same method from different contexts\n");
    Vehicle *my_car_as_vehicle = (Vehicle *)my_car; // Casting works as expected: same methods, same fields
    printf("%d from my_car->estimate_price(my_car)\n", my_car->estimate_price(my_car));
    printf("%d from my_car_as_vehicle->estimate_price(my_car_as_vehicle)\n", my_car_as_vehicle->estimate_price(my_car_as_vehicle));
    printf("%d from my_car->to_Sellable(my_car).estimate_price(my_car)\n", my_car->to_Sellable(my_car).estimate_price(my_car));
    printf("%d from my_car_as_vehicle->to_Sellable(my_car_as_vehicle).estimate_price(my_car_as_vehicle)\n", my_car_as_vehicle->to_Sellable(my_car_as_vehicle).estimate_price(my_car_as_vehicle));
   
    // Elephant object allocated on the stack. We'll use it without any indirection for demonstration purposes.
    printf("\nCreating an Elephant object on the stack\n");
    AUTODESTROY(Elephant) my_elephant; 
    NEW_INPLACE(Elephant, &my_elephant);
    // Alternatively, we could have used CREATE_STACK(Elephant, my_elephant);
    my_elephant.position = 24; // Note that as my_elephant is not a pointer, we access the fields with the dot operator.
    Elephant *my_elephant_ptr = &my_elephant;
    my_elephant_ptr->position = 24; // If we have a pointer to the object, access to the members is done with the arrow operator.

    printf("\nUsing the interface for polymorphism\n");
    printf("Positions ->  my_car: %d, my_elephant: %d\n", my_car->position, my_elephant.position);
    swap_movables_position(my_car->to_Moveable(my_car), my_elephant.to_Moveable(&my_elephant)); // Using the interface for polymorphism
    printf("Positions ->  my_car: %d, my_elephant: %d\n", my_car->position, my_elephant.position);
    
    printf("\nUsing the interface for polymorphism in casted context\n");
    printf("Positions ->  (Vehicle *)my_car: %d, my_elephant: %d\n", ((Vehicle *)my_car)->position, my_elephant.position);
    swap_movables_position(((Vehicle *)my_car)->to_Moveable((Vehicle *)my_car), my_elephant.to_Moveable(&my_elephant)); // Using the interface for polymorphism
    printf("Positions ->  (Vehicle *)my_car: %d, my_elephant: %d\n", ((Vehicle *)my_car)->position, my_elephant.position);

    printf("\nCreating a large number of objects in the heap and the stack\n");
    clock_t start_time = clock();
    for (size_t iter = 0; iter < TEST_NUM_OBJECTS; iter++) {
        CREATE_HEAP(TinyClass, tiny_heap_object);
        if (tiny_heap_object == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }
#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0 
        // Compiler doesn't support auto-destruction; destroy the objects manually
        DESTROY_FREE(tiny_heap_object);
#endif
    }
    clock_t end_time = clock();
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Time taken to create and destroy %llu objects of size %d in the heap: %f seconds. (%.2f objects/second)\n", TEST_NUM_OBJECTS, (int)sizeof(TinyClass), cpu_time_used, TEST_NUM_OBJECTS / cpu_time_used);

    start_time = clock();
    for (size_t iter = 0; iter < TEST_NUM_OBJECTS; iter++) {
        CREATE_STACK(TinyClass, tiny_stack_object);
#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0
        // Compiler doesn't support auto-destruction; destroy the objects manually
        DESTROY(tiny_stack_object);
#endif
    }
    end_time = clock();
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Time taken to create and destroy %llu objects of size %d in the stack: %f seconds. (%.2f objects/second)\n", TEST_NUM_OBJECTS, (int)sizeof(TinyClass), cpu_time_used, TEST_NUM_OBJECTS / cpu_time_used);

#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0
    // Compiler doesn't support auto-destruction; destroy the objects manually
    DESTROY_FREE(my_car);
    DESTROY(my_elephant);
#endif

    printf("\nTrying auto-destructors: Entering scope of create_objects_inside_function\n"); 
    create_objects_inside_function();
    printf("Scope of create_objects_inside_function finished.\n"); 
    printf("Main function finished.\n");
    return 0;
}