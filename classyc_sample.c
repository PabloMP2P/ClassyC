/* classyc_sample.c - A sample program to showcase the ClassyC library */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Some (optional) macros can be used to configure the library naming conventions
 * In this case, we are using the default ones: CLASS, CLASS_ and I_, so the
 * following lines are not needed and are just for demonstration purposes:
 * #define CLASSYC_CLASS_NAME CLASS
 * #define CLASSYC_CLASS_IMPLEMENT CLASS_
 * #define CLASS_OBJECT(Base, Interface, Data, Event, Method, Override) \
 *     Data(void, DESTRUCTOR_FUNCTION_POINTER)
 * #define CLASSYC_INTERFACE_DECLARATION I_
 */
  
#include "ClassyC.h"

/* Number of objects to create and destroy in the speed test */
#define TEST_NUM_OBJECTS 100000ull
/* We'll count instances created and destroyed to show how auto-destruction works to prevent memory leaks */
size_t num_objects_created = 0;
size_t num_objects_destroyed = 0;

/* Interfaces */
#define I_Moveable(Data, Event, Method) \
    Data(int, position) \
    Event(on_move, int distance_moved) \
    Method(void, move, int speed, int distance)
CREATE_INTERFACE(Moveable)                        

#define I_Sellable(Data, Event, Method) \
    Data(int, id) \
    Method(int, estimate_price)
CREATE_INTERFACE(Sellable)

/* Classes */
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


#undef CLASS
#define CLASS AsyncClass
#define CLASS_AsyncClass(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) \
    Data(int, clock_id) \
    Data(int, total_seconds_elapsed) \
    Data(bool, clock_is_running) \
    Data(bool, continue_running) \
    Event(on_second_elapsed) \
    Method(thrd_t, start_clock, void *arg) \
    Method(void, stop_clock)
CONSTRUCTOR() 
    static int clock_id_counter = 0;
    if (!is_base) num_objects_created++;
    self->clock_id = ++clock_id_counter;
    self->clock_is_running = false;
    self->continue_running = true;
    self->total_seconds_elapsed = 0;
END_CONSTRUCTOR
DESTRUCTOR()
    if (!is_base) num_objects_destroyed++;
END_DESTRUCTOR
ASYNC_METHOD(thrd_t, start_clock, void* arg)
    printf("Starting clock %d, with parameter %d...\n", self->clock_id, *(int *)arg);
    self->clock_is_running = true;
    self->continue_running = true;
    while (self->continue_running) {
        /* Wait for 1 second */
        clock_t start_time = clock();
        while (clock() - start_time < 1 * CLOCKS_PER_SEC) {
            /* Wait for 1 second unless the clock is stopped */
            if (!self->continue_running) break;
        }
        if (!self->continue_running) break;
        self->total_seconds_elapsed++;
        RAISE_EVENT(self, on_second_elapsed);

        if (self->total_seconds_elapsed >= *((int *)arg)) {
            printf("Clock %d reached %d seconds, stopping...\n", self->clock_id, *((int *)arg));
            break;
        }
#if CLASSYC_THREADS_SUPPORTED == 0
        /* 
         * The compiler doesn't support threads, and the tinycthread library is not available,
         * You can use CLASSYC_THREADS_SUPPORTED to change the program's behavior
         */
#endif
    }
    printf("Clock %d stopped\n", self->clock_id);
    self->continue_running = true;
    self->clock_is_running = false;
END_ASYNC_METHOD

METHOD(void, stop_clock)
    if (self->clock_is_running) {
        printf("Stopping clock %d...\n", self->clock_id);
        self->continue_running = false;
    }
END_METHOD

/* Interfaces can be used as types, and they contain pointers to the object members */
void swap_movables_position(Moveable object1, Moveable object2) {
    /* interface members are pointers and must be dereferenced to access the actual values */
    int distance_moved = abs(*object1.position - *object2.position);
    int temp = *object1.position;
    *object1.position = *object2.position;
    *object2.position = temp;
    /* Events can be raised through interfaces as well. If no event handler is registered, it will have no effect. */
    RAISE_INTERFACE_EVENT(object1, on_move, distance_moved);
    RAISE_INTERFACE_EVENT(object2, on_move, distance_moved);
}

/* Event handlers are functions that handle events */
EVENT_HANDLER(Car, on_need_fuel, mycar_lowfuel, int km_to_collapse)
    if (km_to_collapse < 10) {
        printf("Fuel level critical! Need to refuel in less than %d km!\n", km_to_collapse);
    }
END_EVENT_HANDLER

EVENT_HANDLER(Car, on_move, mycar_move, int distance_moved)
    printf("EVENT HANDLER: Car moved %d units\n", distance_moved);
END_EVENT_HANDLER

EVENT_HANDLER(AsyncClass, on_second_elapsed, myasyncclass_second_elapsed)
    printf("Tick! Event handler called, %d seconds elapsed in clock %d\n", self->total_seconds_elapsed, self->clock_id);
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
    /* This function is used to test the automatic destruction of objects when they go out of scope */
    
    AUTODESTROY_PTR(Car) *local_heap_car = NEW_ALLOC(Car, 200);
    
    AUTODESTROY(Car) local_stack_car;
    NEW_INPLACE(Car, &local_stack_car, 200);

    printf("Inside function, the objects have been created.\n"); 

#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0
    /* Compiler doesn't support auto-destruction; destroy the objects manually */
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

    if (!my_car) {
        fprintf(stderr, "Failed to create Car object.\n");
        return EXIT_FAILURE;
    }
    REGISTER_EVENT(Car, on_need_fuel, mycar_lowfuel, my_car); /* Register the event handler */
    REGISTER_EVENT(Car, on_move, mycar_move, my_car); /* Register the event handler */
    printf("Event registered\n");
    my_car->km_total += 120; /* Access to data fields */
    my_car->move(my_car, 100, 395); /* This will fire the on_need_fuel event */

    my_car->id=79;
    printf("\nAccessing the same id from different contexts\n");
    printf("%d in my_car->id\n", my_car->id);
    printf("%d in *my_car->to_Sellable(my_car).id\n", *my_car->to_Sellable(my_car).id);
    printf("%d in (Vehicle *)my_car->id\n", ((Vehicle *)my_car)->id);
    printf("%d in *((Vehicle *)my_car)->to_Sellable((Vehicle *)my_car).id\n", *((Vehicle *)my_car)->to_Sellable((Vehicle *)my_car).id); 

    printf("\nAccessing the same method from different contexts\n");
    Vehicle *my_car_as_vehicle = (Vehicle *)my_car; /* Casting works as expected: same methods, same fields */
    printf("%d from my_car->estimate_price(my_car)\n", my_car->estimate_price(my_car));
    printf("%d from my_car_as_vehicle->estimate_price(my_car_as_vehicle)\n", my_car_as_vehicle->estimate_price(my_car_as_vehicle));
    printf("%d from my_car->to_Sellable(my_car).estimate_price(my_car)\n", my_car->to_Sellable(my_car).estimate_price(my_car));
    printf("%d from my_car_as_vehicle->to_Sellable(my_car_as_vehicle).estimate_price(my_car_as_vehicle)\n", my_car_as_vehicle->to_Sellable(my_car_as_vehicle).estimate_price(my_car_as_vehicle));
   
    /* Elephant object allocated on the stack. We'll use it without any indirection for demonstration purposes. */
    printf("\nCreating an Elephant object on the stack\n");
    AUTODESTROY(Elephant) my_elephant; 
    NEW_INPLACE(Elephant, &my_elephant);

    my_elephant.position = 24; /* Note that as my_elephant is not a pointer, we access the fields with the dot operator. */
    Elephant *my_elephant_ptr = &my_elephant;
    my_elephant_ptr->position = 24; /* If we have a pointer to the object, access to the members is done with the arrow operator. */

    printf("\nUsing the interface for polymorphism\n");
    printf("Positions ->  my_car: %d, my_elephant: %d\n", my_car->position, my_elephant.position);
    swap_movables_position(my_car->to_Moveable(my_car), my_elephant.to_Moveable(&my_elephant)); /* Using the interface for polymorphism */
    printf("Positions ->  my_car: %d, my_elephant: %d\n", my_car->position, my_elephant.position);
    
    printf("\nUsing the interface for polymorphism in casted context\n");
    printf("Positions ->  (Vehicle *)my_car: %d, my_elephant: %d\n", ((Vehicle *)my_car)->position, my_elephant.position);
    swap_movables_position(((Vehicle *)my_car)->to_Moveable((Vehicle *)my_car), my_elephant.to_Moveable(&my_elephant)); /* Using the interface for polymorphism */
    printf("Positions ->  (Vehicle *)my_car: %d, my_elephant: %d\n", ((Vehicle *)my_car)->position, my_elephant.position);

    printf("\nCreating a large number of objects in the heap and the stack\n");
    clock_t start_time = clock();
    size_t iter;
    for (iter = 0; iter < TEST_NUM_OBJECTS; iter++) {
        AUTODESTROY_PTR(TinyClass) *tiny_heap_object = NEW_ALLOC(TinyClass);
        if (tiny_heap_object == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }
#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0 
        /* Compiler doesn't support auto-destruction; destroy the objects manually */
        DESTROY_FREE(tiny_heap_object);
#endif
    }
    clock_t end_time = clock();
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Time taken to create and destroy %llu objects of size %d in the heap: %f seconds. (%.2f objects/second)\n", TEST_NUM_OBJECTS, (int)sizeof(TinyClass), cpu_time_used, TEST_NUM_OBJECTS / cpu_time_used);

    start_time = clock();
    for (iter = 0; iter < TEST_NUM_OBJECTS; iter++) {
        AUTODESTROY(TinyClass) tiny_stack_object;
        NEW_INPLACE(TinyClass, &tiny_stack_object);

#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0
        /* Compiler doesn't support auto-destruction; destroy the objects manually */
        DESTROY(tiny_stack_object);
#endif
    }
    end_time = clock();
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Time taken to create and destroy %llu objects of size %d in the stack: %f seconds. (%.2f objects/second)\n", TEST_NUM_OBJECTS, (int)sizeof(TinyClass), cpu_time_used, TEST_NUM_OBJECTS / cpu_time_used);

#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0
    /* Compiler doesn't support auto-destruction; destroy the objects manually */
    DESTROY_FREE(my_car);
    DESTROY(my_elephant);
#endif

    printf("\nTesting asynchronous methods\n");
    AUTODESTROY_PTR(AsyncClass) *my_async_class1 = NEW_ALLOC(AsyncClass);
    AUTODESTROY_PTR(AsyncClass) *my_async_class2 = NEW_ALLOC(AsyncClass);
    AUTODESTROY_PTR(AsyncClass) *my_async_class3 = NEW_ALLOC(AsyncClass);
    REGISTER_EVENT(AsyncClass, on_second_elapsed, myasyncclass_second_elapsed, my_async_class1);
    REGISTER_EVENT(AsyncClass, on_second_elapsed, myasyncclass_second_elapsed, my_async_class2);
    REGISTER_EVENT(AsyncClass, on_second_elapsed, myasyncclass_second_elapsed, my_async_class3);
    printf("Main function - starting clocks...\n");
    int arg1 = 1, arg2 = 2, arg4 = 4;
    my_async_class1->start_clock(my_async_class1, &arg1);
    my_async_class2->start_clock(my_async_class2, &arg2);
    my_async_class3->start_clock(my_async_class3, &arg4);
    
    printf("Main function - waiting for 3 seconds...\n");
    int wait_count;
    for (wait_count = 1; wait_count <= 3; wait_count++) {
        clock_t start_last_wait = clock();
        while (clock() - start_last_wait < 1 * CLOCKS_PER_SEC) {
            /* Wait for threads to finish */
        }
        printf("Main function - %d seconds elapsed\n", wait_count);
    }
    printf("Main function - stopping clocks...\n");
    my_async_class1->stop_clock(my_async_class1);
    my_async_class2->stop_clock(my_async_class2);
    my_async_class3->stop_clock(my_async_class3);
    
    printf("Main function - waiting for all async methods to finish...\n");
    do {
        /* Wait for threads to finish */
    } while (my_async_class1->clock_is_running || my_async_class2->clock_is_running || my_async_class3->clock_is_running);
    printf("Main function - all async methods finished\n");
    printf("Main function - total seconds elapsed in clock 1: %d\n", my_async_class1->total_seconds_elapsed);
    printf("Main function - total seconds elapsed in clock 2: %d\n", my_async_class2->total_seconds_elapsed);
    printf("Main function - total seconds elapsed in clock 3: %d\n", my_async_class3->total_seconds_elapsed);
    printf("Starting clock 1 with AWAIT...\n");
    AWAIT(my_async_class1->start_clock(my_async_class1, &arg1));
    printf("Main function - total seconds elapsed in clock 1: %d\n", my_async_class1->total_seconds_elapsed);

#if CLASSYC_AUTO_DESTROY_SUPPORTED == 0
    /* Compiler doesn't support auto-destruction; destroy the objects manually */
    DESTROY_FREE(my_async_class1);
    DESTROY_FREE(my_async_class2);
    DESTROY_FREE(my_async_class3);
#endif


    printf("\nTrying auto-destructors: Entering scope of create_objects_inside_function\n"); 
    create_objects_inside_function();
    printf("Scope of create_objects_inside_function finished.\n"); 
    printf("Main function finished.\n");
    return 0;
}