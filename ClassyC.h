/* 
  ClassyC.h 2.0.0 - (c) Pablo Soto under MIT License

# ClassyC
A library for OOP in C that allows simple syntax for creating and using classes with polymorphism, inheritance, interfaces, events, and automatic method registration. ClassyC is an experimental and recreational project. It is not intended for production use.

## Creating a class
1. **Include `ClassyC.h`.**
2. **Define CLASS with the name of the class.** To avoid redefinition compiler warnings, use `#undef CLASS` before every new class.
   ```c
   #undef CLASS
   #define CLASS Car
   ```
3. **Define the x-macro `CLASS_class_name(Base, Interface, Data, Event, Method, Override)`** to declare the base class, interfaces, data members, events, methods, and overrides.
   - This macro must be defined with one `Base` and zero or more of each `Interface`, `Data`, `Event`, `Method` and `Override` entries.
   - Interfaces, data members, events, and methods are inherited from the base class (and its base classes, recursively).
   - Classes implement interfaces. When a class declares that it uses an interface, it must also declare and define all the members (data, events, and methods) of that interface, or inherit them from a parent class.
   - Overridden methods must maintain the signatures (return type, number of parameters, and parameter types) of the original method.
   - Users should not redeclare any members or interfaces already declared in base classes (collision will occur).
   Syntax:
   - `Base(base_class_name)` - To declare the base class (use `OBJECT` if it has no base class).
   - `Interface(interface_name)` - To declare an interface.
   - `Data(member_type, member_name)` - To declare a data member.
   - `Event(event_name[, args])` - To declare an event.
   - `Method(ret_type, method_name[, args])` - To declare a new method.
   - `Override(ret_type, method_name[, args])` - To declare an overridden method.
   ```c
   #define CLASS_Car(Base, Interface, Data, Event, Method, Override) \
     Base(Vehicle) Interface(Refuelable) \
     Data(int, km_total) \
     Data(int, km_since_last_fuel) \
     Event(on_need_fuel, int km_to_collapse) \
     Method(void, park) \
     Override(int, estimate_price)
   ```
4. **Use `CONSTRUCTOR(optional_parameters)` macro** and include any code to execute when a new instance (available as `self` in the constructor) is created.
   - Optionally, call `INIT_BASE([optional_parameters]);` to run the user-defined code in the `CONSTRUCTOR` of the base class.
   - If used, `INIT_BASE` should be called inside the `CONSTRUCTOR` body and before any custom initialization code.
   - Close with `END_CONSTRUCTOR`.
   ```c
   CONSTRUCTOR() END_CONSTRUCTOR
   ```
   ```c
   CONSTRUCTOR(int km_total_when_bought)
     INIT_BASE();
     self->position = 0;
     self->km_total = km_total_when_bought;
     self->km_since_last_fuel = 0;
   END_CONSTRUCTOR
   ```
5. **Use `DESTRUCTOR()` macro** and include cleanup code before the `END_DESTRUCTOR` macro. Instance is available as `self`.   
   ```c
   DESTRUCTOR() END_DESTRUCTOR
   ```
6. **Use `METHOD(ret_type, method_name, ...)` macro** to implement every method declared in the `CLASS_class_name` macro.
   - Within methods, the current object is accessed using the `self` pointer.
   - Close the method implementation with `END_METHOD`.
   ```c
   METHOD(void, move, int speed, int distance)
     self->position += distance;
     self->km_since_last_fuel += distance;
     int km_to_collapse = 400 - self->km_since_last_fuel;
     if (km_to_collapse < 100) {
       RAISE_EVENT(self, on_need_fuel, km_to_collapse);
     }
   END_METHOD
   ```
7. **Raise events from any method using `RAISE_EVENT(object, event_name[, args])`**. If the event has a registered handler, it will be called.

## Using a class
1. **Call `CREATE(class_name, optional_constructor_parameters)`** to create a new object.
   ```c
   Car *my_car = CREATE(Car, 10000);
   ```
2. **Access data members directly (`object->member_name = value;`).**
   ```c
   my_car->km_total = 10000;
   ```
3. **Call methods adding the instance as the first argument, before any other arguments the method may need (`object->method_name(object, ...)`).**
   - Methods REQUIRE the instance to be passed explicitly as the first parameter. A `CALL` macro is provided for convenience.
   - To call methods using the `CALL` macro use `CALL(instance, method[, args])`.
   - There is no need to cast the object; the method will cast to the appropriate type and provide the correctly casted `self` pointer inside the method.
   - All methods, inherited or new (or interface-based), follow this calling convention.
   ```c
   my_car->move(my_car, 100, 200);
   ```
   ```c
   CALL(my_car, move, 100, 200);  // Alternative syntax, expands to the above
   ```
4. **Define event handlers using `EVENT_HANDLER(class_name, event_name, handler_ID, ...) [code] END_EVENT_HANDLER`** in the global scope (outside of any function). `handler_ID` is a unique ID for the event handler (letters, numbers, `_`).
   - Within event handlers, the instance is accessed using the `self` pointer.
   ```c
   EVENT_HANDLER(Car, on_need_fuel, mycar_lowfuel, int km_to_collapse)
     if (km_to_collapse < 10) {
       printf("Alert! Last refuel was %d km ago. Need to refuel in less than %d km!\n", self->km_since_last_fuel, km_to_collapse);
     }
   END_EVENT_HANDLER
   ```
5. **Register an event handler with an object using: `REGISTER_EVENT(class_name, event_name, handler_ID, object)`**.
   - Only one handler can be registered per event and object. Subsequent calls to `REGISTER_EVENT` for the same event and object will overwrite the previous handler.
   ```c
   REGISTER_EVENT(Car, on_need_fuel, mycar_lowfuel, my_car);
   ```
6. **To cast the object to the desired type, use `(cast_class *)object`.**
   - Available methods and data members will be the subset available in the cast class.
   - Methods will be the most derived versions.
   ```c
   Vehicle *my_car_as_vehicle = (Vehicle *)my_car;
   ```
7. **Call `DESTROY(object)` to free the memory allocated for the object.** It is recommended to nullify the pointer after `DESTROY`ing.
   ```c
   DESTROY(my_car);
   my_car = NULL;
   ```
## Creating and using interfaces
1. **Define the x-macro `I_interface_name(Data, Event, Method)` to declare a new interface and all its members.**
   - Syntax:
     - `Data(member_type, member_name)` - To declare a data member.
     - `Event(event_name[, args])` - To declare an event.
     - `Method(ret_type, method_name[, args])` - To declare a method.
   ```c
   #define I_Moveable(Data, Event, Method) \
     Data(int, position) \
     Event(on_move, int distance_moved) \
     Method(void, move, int speed, int distance)
   ```
2. **Call `CREATE_INTERFACE(interface_name)` once right after the interface declaration.**
   - This creates a new type of interface struct with the pointers to the members declared in the interface and a self pointer to the class instance.
   - It is required to call this macro after the interface declaration and before any class that implements the interface.
   ```c
   CREATE_INTERFACE(Moveable)
   ```
3. **To implement the interface, make sure the class declares or inherits all the members and includes the interface name in its `CLASS_class_name` interfaces list.**
   ```c
   #define CLASS_Vehicle(Base, Interface, Data, Event, Method, Override)\
     Base(OBJECT) Interface(Sellable) Interface(Moveable) \
     Data(int, id) \
     Data(int, position) \
     Event(on_move, int distance_moved) \
     Method(int, estimate_price) \
     Method(void, move, int speed, int distance)
   ```
4. **The resulting interface accessor for an interface will be named `as_` + `interface_name` and will be accessible in this way.**
   - To use the interface, access the `as_interface_name` member of any object of a class that implements the interface.
   - `as_interface_name` is a struct of type `interface_name`, which contains pointers to all the interface members in the object and the object itself.
   - Interface struct data members are pointers to the actual data members in the object. When accessing them, you need to dereference the pointers.
   - Interface structs should be handled carefully to avoid shallow copies leading to unintended side effects.
   ```c
   void swap_movables_position(Moveable object1, Moveable object2) {
     int distance_moved = abs(*object1.position - *object2.position);
     int temp = *object1.position;
     *object1.position = *object2.position;
     *object2.position = temp;
     RAISE_INTERFACE_EVENT(object1, on_move, distance_moved);
     RAISE_INTERFACE_EVENT(object2, on_move, distance_moved);
   }
   
   int main(void) {
     Car *my_car = CREATE(Car, 10000);
     Elephant *my_elephant = CREATE(Elephant);
     swap_movables_position(my_car->as_Moveable, my_elephant->as_Moveable);
     DESTROY(my_car);
     my_car = NULL;
     DESTROY(my_elephant);
     my_elephant = NULL;
   }
   ```
5. **To raise an event from an interface, use `RAISE_INTERFACE_EVENT(as_interface_obj, event_name[, args])`.**
   - The `as_interface_obj` is the interface struct, which contains the pointers to the interface members in the object.
   - `RAISE_INTERFACE_EVENT` will handle the additional level of indirection due to the interface's structure.
   ```c
   RAISE_INTERFACE_EVENT(movable_struct, on_move, distance_moved);
   ```
## ClassyC configuration macros
These macros can be defined before including this header to customize some of the library's naming conventions and error checking.
- **CLASSYC_PREFIX**: Prefix for the global scope identifiers. Default: `#define CLASSYC_PREFIX ClassyC_`
- **CLASSYC_CLASS_NAME**: Used to define the macro holding the class name, by default it is set to `CLASS` but can be changed to any other name to avoid conflicts.
  ```c
  #define CLASSYC_CLASS_NAME NEW_CLASS_NAME
  #define NEW_CLASS_NAME Aircraft
  ```
- **CLASSYC_CLASS_IMPLEMENT**: Used to define the prefix of the macro holding the class implementation. Default: `#define CLASSYC_CLASS_IMPLEMENT CLASS_`
  If you redefine `CLASSYC_CLASS_IMPLEMENT`, you must also define an empty macro for the `OBJECT` class with the same prefix.
  ```c
  #define CLASSYC_CLASS_IMPLEMENT DECLARE_CLASS_
  #define DECLARE_CLASS_OBJECT(Base, Interface, Data, Event, Method, Override)
  #define DECLARE_CLASS_Aircraft(Base, Interface, Data, Event, Method, Override)
  ```
  ```c
  #define CLASSYC_CLASS_IMPLEMENT CUSTOM_CLASS_
  #define CUSTOM_CLASS_OBJECT(Base, Interface, Data, Event, Method, Override)
  #define CUSTOM_CLASS_Aircraft(Base, Interface, Data, Event, Method, Override)
  ```
- **CLASSYC_INTERFACE_DECLARATION**: The name of the macro that declares the interface. Default: `#define CLASSYC_INTERFACE_DECLARATION I_`
  ```c
  #define CLASSYC_INTERFACE_DECLARATION NEW_INTERFACE_
  #define NEW_INTERFACE_Moveable(Data, Event, Method)
  ```
- **CLASSYC_DISABLE_RUNTIME_CHECKS**: Disable runtime checks for inheritance depth. Default: not defined.
- **CLASSYC_ENABLE_COMPILE_TIME_CHECKS**: Enable compile-time checks for inheritance depth. Default: not defined.
## Additional notes
- Class definitions must be at the global scope. Objects can be declared at any scope, but can't be instantiated outside a function. Interfaces are declared in the top-level scope, before any class that uses them.
- A class inherits all the methods, events, data members, and interfaces of its base class and, recursively, its base classes.
- Inherited methods with no new implementation don't need to be included in `CLASS_class_name` or with `METHOD`; they are automatically inherited and available.
- All methods — new, inherited, or overridden — self-register internally in the class constructor: no need to assign pointers or call register.
- `CONSTRUCTOR` and `DESTRUCTOR` are mandatory: must be explicitly defined even if no actions are needed.
- `CONSTRUCTOR` accepts user-defined parameters; `DESTRUCTOR` doesn't.
- The `CONSTRUCTOR` macro must be used after the class definitions and before any methods or the `DESTRUCTOR`.
- The `DESTRUCTOR` can be declared after the `CONSTRUCTOR` and before or after methods, but must not be declared before the `CONSTRUCTOR`.
- `INIT_BASE` requires the arguments to match the base `CONSTRUCTOR` parameters. It should be called before the rest of the `CONSTRUCTOR` code.
- `METHOD`, `CONSTRUCTOR`, `DESTRUCTOR`, and `EVENT_HANDLER` need to be used in the global scope.
- No curly braces are needed around the body of the methods/events/constructors/destructors but can be used.
- Interfaces declared in base classes are automatically available in derived classes, including them again in the derived class will cause collisions.
- Interfaces can overlap in data members, events, and methods without causing conflicts.
- Casting an object to any base class will give access to the subset of data members and methods present in that base class.
  The methods in the casted object will still point to the most derived implementation of the method in the inheritance chain.
- All the valid casts of the object will access the same versions of the data, events, and methods.
- Interface `as_interface_name` structs contain pointers to all the interface members in the object.
  This allows access to members and passing the interface object to functions as value.
- Interface event members are pointers to function pointers to handle dynamic event handler registration.
- All method pointers are set to the most derived version of the method in the inheritance chain.
- Methods and events have only one level of indirection; the pointers are not in virtual tables.
- The library is optimized to reduce levels of indirection and data overhead.
- Ensure that for every `CREATE`, there is a corresponding `DESTROY` to prevent memory leaks.
- Make sure to nullify all pointers to the instance after `DESTROY`ing to avoid dangling pointers.
- The `RECURSIVE_CLASS_MEMBER_DECLARATION` macro limits the inheritance depth to 9 levels.
  Compile-time checks are available in C11 and later and can be enabled by defining `CLASSYC_ENABLE_COMPILE_TIME_CHECKS` before including the header.
  Runtime checks are enabled by default, but can be disabled by defining `CLASSYC_DISABLE_RUNTIME_CHECKS` before including the header.
  To support deeper inheritance hierarchies, you can extend the macro definitions by adding `RECURSIVE_CLASS_MEMBER_DECLARATION_10`, `RECURSIVE_CLASS_MEMBER_DECLARATION_11`, and so on, making sure that each macro expands to the next one.


    Full example:

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

*/


#ifndef ClassyC_H
#define ClassyC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>


/* Prefix for function names to avoid polluting the global namespace. */
/* To personalize it define it before including this file. */
#ifndef CLASSYC_PREFIX
#define CLASSYC_PREFIX ClassyC_
#endif


/* HELPER MACROS */
#define CONCAT_HELPER(a, b) a##b
#define CONCAT(a, b) CONCAT_HELPER(a, b)
#define TRICAT_HELPER(a, b, c) a##b##c
#define TRICAT(a, b, c) TRICAT_HELPER(a, b, c)
#define ADD_PREFIX(str) CONCAT(CLASSYC_PREFIX, str)
#define PREFIXCONCAT(a, b) TRICAT(CLASSYC_PREFIX, a, b)
#define QUOTE_HELPER(str) #str
#define QUOTE(str) QUOTE_HELPER(str)
/* WRITE_NOTHING is used to expand x-macros selectively, by setting WRITE_NOTHING as the macro you want to omit */
#define WRITE_NOTHING(...)


/* The name of the macro that declares the class name: by default it is CLASS_ */
#ifndef CLASSYC_CLASS_NAME
#define CLASSYC_CLASS_NAME CLASS
#endif

/* The name of the macro that declares the class declaration prefix: by default it is CLASS_ (resulting in CLASS_class_name) */
/* If this is defined, the empty prefix_OBJECT(Base, Interface, Data, Event, Method, Override) macro must also be defined */
#ifndef CLASSYC_CLASS_IMPLEMENT
#define CLASSYC_CLASS_IMPLEMENT CLASS_
#define CLASS_OBJECT(Base, Interface, Data, Event, Method, Override) 
#endif
#define GET_IMPLEMENTS(class_name)CONCAT(CLASSYC_CLASS_IMPLEMENT, class_name)

/* The name of the macro that declares the interface declaration prefix: by default it is I_ (resulting in I_interface_name) */
#ifndef CLASSYC_INTERFACE_DECLARATION
#define CLASSYC_INTERFACE_DECLARATION I_
#endif
#define GET_INTERFACE(interface_name)CONCAT(CLASSYC_INTERFACE_DECLARATION, interface_name)

/* Inheritance depth runtime check */
#ifdef CLASSYC_DISABLE_RUNTIME_CHECKS
    #define CLASSYC_CHECK_INHERITANCE_DEPTH
#else
    #define CLASSYC_CHECK_INHERITANCE_DEPTH \
        int ADD_PREFIX(inheritance_depth) = GET_INHERITANCE_LEVEL(CLASSYC_CLASS_NAME); \
        if (ADD_PREFIX(inheritance_depth) > 9) { \
            fprintf(stderr, QUOTE(CLASSYC_CLASS_NAME) " inheritance depth (%d) exceeds the maximum supported limit (9 levels)\n", ADD_PREFIX(inheritance_depth)); \
            return NULL; \
        } 
#endif

/* Static assertion to ensure the inheritance depth does not exceed the maximum limit */ \
/* Available in C11 and later: disabled by default */ \
#ifdef CLASSYC_ENABLE_COMPILE_TIME_CHECKS
    #define CLASSYC_CHECK_INHERITANCE_DEPTH_CT \
        _Static_assert(GET_INHERITANCE_LEVEL(CLASSYC_CLASS_NAME) <= 9, "Inheritance depth exceeds the maximum supported limit (9 levels)");
#else
    #define CLASSYC_CHECK_INHERITANCE_DEPTH_CT
#endif


/* Header of the class struct */
/* Declare a class struct and its type name */
#define STRUCT_HEADER(struct_name) \
    typedef struct struct_name struct_name; \
    struct struct_name

/* OBJECT class: the base class of all classes. It has no base class, no interfaces, no data, no methods, no events */
/* OBJECT class struct */
STRUCT_HEADER(OBJECT) {
    /* Having a pointer to the destructor function helps DESTROY calling it without knowing the class name */
    void (*_destructor)(void *self_void);
};
/* Prototypes for OBJECT class functions */
static void *ADD_PREFIX(OBJECT_constructor)(void *self_void);
static void ADD_PREFIX(OBJECT_interfaces_constructor)(void *self_void);
static void *ADD_PREFIX(OBJECT_user_constructor)(int run_init, void *self_void);
static void ADD_PREFIX(OBJECT_destructor)(void *self_void);
/* OBJECT class constructor function: only sets the destructor functionpointer */
static void *ADD_PREFIX(OBJECT_constructor)(void *self_void) { 
    OBJECT *self = (OBJECT *)self_void; 
    self->_destructor = ADD_PREFIX(OBJECT_destructor); 
    /* Should not be called directly */ 
    return self; 
}
static void ADD_PREFIX(OBJECT_interfaces_constructor)(void *self_void) { /* OBJECT interfaces constructor does nothing */ }
static void *ADD_PREFIX(OBJECT_user_constructor)(int run_init, void *self_void) {
    if (run_init) {
        self_void = ADD_PREFIX(OBJECT_constructor)(self_void);
        ADD_PREFIX(OBJECT_interfaces_constructor)(self_void);
    }
    return self_void;
}
/* OBJECT class destructor function */
static void ADD_PREFIX(OBJECT_destructor)(void *self_void) { /* OBJECT destructor does nothing */ }


/* BASIC MACROS */
/* Get the name of the base class from the IMPLEMENTS macro */
#define WRITE_BASE_NAME(base_to_call)base_to_call
#define X_GET_BASE_NAME(class) GET_IMPLEMENTS(class)(WRITE_BASE_NAME, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)

/* Set the pointers to the Basic class functions (_destructor) */
#define WRITE_SET_BASIC_CLASS_FUNC_POINTERS(class_name) \
    /* Constructor not needed, we set the destructor */ \
    self->_destructor = PREFIXCONCAT(class_name, _destructor); 

/* Set the method pointers to the functions of the class */
#define SET_METHOD_PTR(ret_type, method_name, ...) \
    ((CLASSYC_CLASS_NAME *)self_void)->method_name = PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name); 

/* COMPONENTS OF THE CLASS STRUCT */
#define WRITE_METHOD_POINTER(ret_type, method_name, ...) \
    ret_type (*method_name)(void *self_void, ##__VA_ARGS__);
#define WRITE_DATA_MEMBER(type, member_name) \
    type member_name;
#define WRITE_EVENT_MEMBER(event_name, ...) \
    void (*event_name)(void *self_void, ##__VA_ARGS__);
#define WRITE_INTERFACE_INSTANCE(interface_name) \
    interface_name CONCAT(as_, interface_name);

/* Write ALL the interface instances, data members and methods (searching only for new ones) the class implements in the class struct */
/* by recursively crossing the inheritance tree*/
#define WRITE_CLASS_STRUCT_MEMBERS(class) \
    GET_IMPLEMENTS(class)(WRITE_NOTHING, WRITE_INTERFACE_INSTANCE, WRITE_DATA_MEMBER, WRITE_EVENT_MEMBER, WRITE_METHOD_POINTER, WRITE_NOTHING)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_9(class)  \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_8(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_9, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_7(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_8, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_6(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_7, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_5(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_6, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_4(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_5, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_3(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_4, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_2(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_3, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION_1(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_2, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)
#define RECURSIVE_CLASS_MEMBER_DECLARATION(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_1, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_STRUCT_MEMBERS(class)

/* Macros to count inheritance levels to help with static assertions */
#define INHERITANCE_LEVEL_1(class) +1
#define INHERITANCE_LEVEL_2(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_1, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
#define INHERITANCE_LEVEL_3(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_2, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
#define INHERITANCE_LEVEL_4(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_3, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
#define INHERITANCE_LEVEL_5(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_4, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
#define INHERITANCE_LEVEL_6(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_5, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
#define INHERITANCE_LEVEL_7(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_6, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
#define INHERITANCE_LEVEL_8(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_7, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
#define INHERITANCE_LEVEL_9(class) + 1 GET_IMPLEMENTS(class)(INHERITANCE_LEVEL_8, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)
/* Use the appropriate INHERITANCE_LEVEL macro based on the depth */
#define GET_INHERITANCE_LEVEL(class) ( 0 INHERITANCE_LEVEL_9(class))




/* New and overridden method function prototypes */
#define WRITE_METHOD_FUNC_PROTOTYPE(ret_type, method_name, ...) \
    static ret_type PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name)(void *self_void, ##__VA_ARGS__); 
#define X_METHOD_FUNC_PROTOTYPES(class_name) \
    /* Writes the method prototypes for the class: new methods and overridden methods */ \
    GET_IMPLEMENTS(class_name)(WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_METHOD_FUNC_PROTOTYPE, WRITE_METHOD_FUNC_PROTOTYPE) 

/* INTERFACES */
#define WRITE_I_METHOD_PTR(ret_type, method_name, ...) \
    ret_type (*method_name)(void *self_void, ##__VA_ARGS__);
#define WRITE_I_DATA_MEMBER(type, member_name) \
    /* A pointer to the data member */\
    type *member_name;
#define WRITE_I_EVENT_MEMBER(event_name, ...) \
    /* Not a pointer to the function, but a pointer to the function pointer */\
    /* That way, if method handlers are registered later, we don't have to reassign the pointer */\
    void (**event_name)(void *self_void, ##__VA_ARGS__);
#define WRITE_INTERFACE_STRUCT(interface_name) \
    GET_INTERFACE(interface_name)(WRITE_I_DATA_MEMBER, WRITE_I_EVENT_MEMBER, WRITE_I_METHOD_PTR)
#define CREATE_INTERFACE(interface_name) \
   typedef struct interface_name interface_name; \
   struct interface_name { \
   void *self; \
   WRITE_INTERFACE_STRUCT(interface_name) \
   };
#define WRITE_INTERFACE_INSTANCE(interface_name) \
    interface_name CONCAT(as_, interface_name);
#define WRITE_I_METHOD_PTR_INITIALIZER(ret_type, method_name, ...) \
    /* Copies the method pointer to the interface */\
    .method_name = self->method_name,
#define WRITE_I_DATA_MEMBER_INITIALIZER(type, member_name) \
    /* Copies the pointer to the data member to the interface */\
    .member_name = &self->member_name,
#define WRITE_I_EVENT_MEMBER_INITIALIZER(event_name, ...) \
    /* Stores the address of the function pointer in the interface */\
    .event_name = &self->event_name,
#define WRITE_INTERFACE_INITIALIZER(interface_name) \
    self->CONCAT(as_, interface_name) = (interface_name){ \
    GET_INTERFACE(interface_name)(WRITE_I_DATA_MEMBER_INITIALIZER, WRITE_I_EVENT_MEMBER_INITIALIZER, WRITE_I_METHOD_PTR_INITIALIZER) \
    .self = self \
    };
#define INITIALIZE_INTERFACES_PTRS(class_name) \
    GET_IMPLEMENTS(class_name)(WRITE_NOTHING, WRITE_INTERFACE_INITIALIZER, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)

/* CONSTRUCTOR, DESTRUCTOR, INIT... */
#define CONSTRUCTOR(...)\
    /* Compile-time assertion (available in C11 and later) to ensure the inheritance depth does not exceed the maximum limit */ \
    CLASSYC_CHECK_INHERITANCE_DEPTH_CT\
    /* Declare the class struct */ \
    STRUCT_HEADER(CLASSYC_CLASS_NAME) { \
        /* Having a pointer to the user destructor code helps DESTROY calling it without knowing the class name */\
        void (*_destructor)(void *self_void); \
        /* Include all the members of the class struct */ \
        RECURSIVE_CLASS_MEMBER_DECLARATION(CLASSYC_CLASS_NAME) \
    }; \
    /* Prototypes for the _destructor and _constructor class functions */ \
    static void PREFIXCONCAT(CLASSYC_CLASS_NAME, _destructor)(void *self_void); \
    static void * PREFIXCONCAT(CLASSYC_CLASS_NAME, _constructor)(void *self_void); \
    static void PREFIXCONCAT(CLASSYC_CLASS_NAME, _interfaces_constructor)(void *self_void); \
    static void * PREFIXCONCAT(CLASSYC_CLASS_NAME, _user_constructor)(int run_init, void * self_void, ##__VA_ARGS__); \
    /* Write the prototypes for new and overridden methods */ \
    X_METHOD_FUNC_PROTOTYPES(CLASSYC_CLASS_NAME) \
    /* Constructor function */ \
    static void* PREFIXCONCAT(CLASSYC_CLASS_NAME, _constructor)(void * self_void) { \
        CLASSYC_CHECK_INHERITANCE_DEPTH\
        CLASSYC_CLASS_NAME *self; \
        if (!self_void) { \
            self = (CLASSYC_CLASS_NAME *)calloc(1, sizeof(CLASSYC_CLASS_NAME)); \
            if (!self) { \
                /* Allocation failure */ \
                return NULL; \
            } \
            self_void = self; \
        } else { \
            self = (CLASSYC_CLASS_NAME *)self_void; \
        } \
        /* Call the base class constructor */ \
        PREFIXCONCAT(X_GET_BASE_NAME(CLASSYC_CLASS_NAME), _constructor)(self); \
        /* Set the basic class function pointers (_destructor) to the functions of the class */ \
        WRITE_SET_BASIC_CLASS_FUNC_POINTERS(CLASSYC_CLASS_NAME) \
        /* Set method pointers to the functions of the class */ \
        /* as constructors are executed in the order of inheritance, overridden methods are set last */ \
        GET_IMPLEMENTS(CLASSYC_CLASS_NAME)(WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, SET_METHOD_PTR, SET_METHOD_PTR) \
        return self; \
    } \
    static void PREFIXCONCAT(CLASSYC_CLASS_NAME, _interfaces_constructor)(void *self_void) { \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void; \
        /* Initialize the interfaces pointers to the members of the class struct */ \
        INITIALIZE_INTERFACES_PTRS(CLASSYC_CLASS_NAME) \
        /* Call the base class interfaces constructor */ \
        PREFIXCONCAT(X_GET_BASE_NAME(CLASSYC_CLASS_NAME), _interfaces_constructor)(self); \
    } \
    static void* PREFIXCONCAT(CLASSYC_CLASS_NAME, _user_constructor)(int run_init, void * self_void, ##__VA_ARGS__) { \
        if (run_init) { \
            /* Only for the instanced objects, not for the base classes */\
             self_void = PREFIXCONCAT(CLASSYC_CLASS_NAME, _constructor)(self_void); \
             if (!self_void) { \
                /* Allocation failure */ \
                return NULL; \
             } \
             PREFIXCONCAT(CLASSYC_CLASS_NAME, _interfaces_constructor)(self_void); \
        } \
        CLASSYC_CLASS_NAME * self = (CLASSYC_CLASS_NAME *)self_void; \
        /* User constructor code follows, it will be executed even if run_init is 0 */ \

#define END_CONSTRUCTOR \
        return self; \
    }

/* Destructor macro */
#define DESTRUCTOR() \
     /* Contains the destructor code for the class. Then on END_DESTRUCTOR invokes the base class destructor */\
    static void PREFIXCONCAT(CLASSYC_CLASS_NAME, _destructor)(void *self_void) { \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void; \
        /* User destructor code */

#define END_DESTRUCTOR \
        PREFIXCONCAT(X_GET_BASE_NAME(CLASSYC_CLASS_NAME), _destructor)(self); \
    }

/* METHOD CREATION */
#define METHOD(ret_type, method_name, ...) \
    static ret_type PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name)(void *self_void, ##__VA_ARGS__) { \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void; \
        /* User method code */
#define END_METHOD \
    }

/* EVENTS */
#define GET_EVENT_FUNC_NAME(class_name, event_name, handler_ID) \
    CONCAT(TRICAT(ClassyC_, class_name, _), TRICAT(event_name, _hdlr_, handler_ID))
/* Define an event handler for an instance. Handler_ID is a unique ID for the event handler (letters, numbers, _). */
#define EVENT_HANDLER(class_name, event_name, handlerID, ...) \
    static void GET_EVENT_FUNC_NAME(class_name, event_name, handlerID)(void *self_void, ##__VA_ARGS__); \
    static void GET_EVENT_FUNC_NAME(class_name, event_name, handlerID)(void *self_void, ##__VA_ARGS__) { \
        class_name *self = (class_name *)self_void; \
        /* User code for the event */ 
#define END_EVENT_HANDLER }
/* Register an event handler for an instance. Handler_ID is a unique ID for the defined event handler (letters, numbers, _). */
#define REGISTER_EVENT(class_name, event_name, handler_ID, instance_name) \
    instance_name->event_name = GET_EVENT_FUNC_NAME(class_name, event_name, handler_ID);
/* Raise an event: use inside any function: RAISE_EVENT(self, event_name[, args]) */
#define RAISE_EVENT(instance_name, event_name, ...) \
    do { \
        if (instance_name->event_name) instance_name->event_name ((void *)instance_name, ##__VA_ARGS__); \
    } while (0)
/* Raise an event from an interface: use inside any function: RAISE_INTERFACE_EVENT(interface_struct, event_name[, args]) */
/* As functions manipulating the interface struct may not be aware of the actual class implementing the interface, we need this extra macro. */
#define RAISE_INTERFACE_EVENT(interface_struct, event_name, ...) \
    /* We need to dereference the pointer to the function pointer stored in the interface */ \
    do { \
        if ((*interface_struct.event_name)) (*interface_struct.event_name)(interface_struct.self, ##__VA_ARGS__); \
    } while (0)

/* CREATE macro to instantiate an object */
#define CREATE(class_name, ...) \
    (class_name *)PREFIXCONCAT(class_name, _user_constructor)(1, NULL, ##__VA_ARGS__)

/* Initialize the base class: call this to run the user-defined code in the 'CONSTRUCTOR' of the base class. */
#define INIT_BASE(...) \
    PREFIXCONCAT(X_GET_BASE_NAME(CLASSYC_CLASS_NAME), _user_constructor)(0, self, ##__VA_ARGS__)

/* DESTROY macro to free the object */
#define DESTROY(obj_name) \
    do { \
        if (obj_name && obj_name->_destructor)  { \
            obj_name->_destructor(obj_name); \
            free(obj_name); \
        } \
    } while (0)


#define CALL(instance, method, ...) \
    instance->method(instance, ##__VA_ARGS__)

#endif /* ClassyC_H */

/* MIT License. Copyright (c) Pablo Soto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/