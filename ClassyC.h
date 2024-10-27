/* 
  ClassyC.h - (c) Pablo Soto under the MIT License

# ClassyC
A library for OOP in C that allows simple syntax for creating and using classes with polymorphism, inheritance, interfaces, events, sync and async methods, automatic method registration and automatic destruction of objects and freeing of memory.

ClassyC is an experimental and recreational library not intended for production use. Anything can change at any time. Use at your own risk.

## Creating a class
1. **Include `ClassyC.h`.**
2. **Define CLASS with the name of the class.** To avoid redefinition compiler warnings, use `#undef CLASS` before every new class, or at the end of a class (this might be useful if you are defining multiple classes in the same file).
   ```c
   #define CLASS Car
   ```
3. **Define the x-macro `CLASS_class_name(Base, Interface, Data, Event, Method, Override)`** to declare the base class, interfaces, data members, events, methods, and overrides.
   - This macro must be defined with one `Base` and zero or more of each `Interface`, `Data`, `Event`, `Method` and `Override` entries.
   - Interfaces, data members, events, and methods are inherited from the base class (and its base classes, recursively).
   - Classes implement interfaces. When a class declares that it uses an interface, it must also declare and define all the members (data, events, and methods) of that interface, or inherit them from a parent class.
   - Overridden methods must exactly match the signatures (return type, number of parameters, and parameter types) of the original method to ensure proper behavior.
   - Declarations should not redeclare any members already present in base classes (name collision will occur), except for Override methods.
   - All classes must inherit from another class, or the OBJECT class, so all objects inherit the OBJECT class members (a destructor and a mutex).
   Syntax:
   - `Base(base_class_name)` - To declare the base class (use `OBJECT` if it has no base class).
   - `Interface(interface_name)` - To declare an interface.
   - `Data(member_type, member_name)` - To declare a data member.
   - `Event(event_name[, args])` - To declare an event.
   - `Method(ret_type, method_name[, args])` - To declare a new method.
   - `Override(ret_type, method_name[, args])` - To declare an overridden method.
   ```c
   #define CLASS Car
   #define CLASS_Car(Base, Interface, Data, Event, Method, Override) \
       Base(Vehicle) \
       Data(int, km_total) \
       Data(int, km_since_last_fuel) \
       Event(on_need_fuel, int km_to_collapse) \
       Method(void, park) \
       Override(int, estimate_price) \
       Override(void, move, int speed, int distance)
   ```
4. **Use `CONSTRUCTOR(optional_parameters)` macro** and include any code to execute when a new instance (available as `self` in the constructor) is created.
   - Optionally, call `INIT_BASE([optional_parameters]);` to run the user-defined code in the `CONSTRUCTOR` of the base class.
   - If used, `INIT_BASE` should be called inside the `CONSTRUCTOR` body and before any custom initialization code.
   - The variable is_base is available here; a bool flag passed to the constructor (`false` for the most derived class and `true` for base classes during inheritance initialization).
   - Curly braces around the body content are not needed, as the braces are already included by the macros.
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
       if (!is_base) {
       // Initialization code specific to the most derived class, for example, counting the number of instances of the class
     }     
   END_CONSTRUCTOR
   ```
5. **Use `DESTRUCTOR()` macro** and include cleanup code before the `END_DESTRUCTOR` macro. Instance is available as `self`, and `is_base` reports if the destructor is being called by a derived class.
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
   - **Use `ASYNC_METHOD(thrd_t, method_name, void *arg)` macro** to implement asynchronous methods. Note that the method must have a return type of `thrd_t` and have a parameter of type and name `void *arg`.
   ```c
   ASYNC_METHOD(thrd_t, save_data, void *arg)
       // save data 
       write_data_to_file((my_data_struct *)arg);
       // ...
       self->finished = true;
   END_ASYNC_METHOD
   ```
   - Synchronous methods will block until they finish, and can have parameters and return values.
   - Asynchronous methods will return immediately and must have one parameter of type `void *` and name `arg`. Asynchronous methods return the thread ID, which can be used with the `AWAIT` macro to wait for the method to finish.
7. **Raise events from any method using `RAISE_EVENT(object, event_name[, args])`**. If the event has a registered handler, it will be called.

## Using a class

1. **Use `NEW_ALLOC(ClassName, [ConstructorArgs])`** to allocate and create a new object in the heap.
   You can add the use of `AUTODESTROY_PTR(ClassName)` to automatically destroy the object and free the memory when it goes out of scope.
   ```c
   // Simple syntax with automatic destruction
   AUTODESTROY_PTR(Car) *my_car = NEW_ALLOC(Car);
   // Alternative syntax without automatic destruction:
   Car *my_car = NEW_ALLOC(Car);
   ```
   Or use `NEW_INPLACE(ClassName, object_address)` to create a new object in the stack (or any other address).
   You can add the use of `AUTODESTROY(ClassName)` to automatically destroy the object (without freeing memory) when it goes out of scope.
   ```c
   // Simple syntax with automatic destruction:
   AUTODESTROY(Elephant) my_elephant; 
   NEW_INPLACE(Elephant, &my_elephant);
   // Alternative syntax without automatic destruction:
   Elephant my_elephant;
   NEW_INPLACE(Elephant, &my_elephant);
   ```
2. **Access data members directly (`object->member_name = value;`).**
   ```c
   my_car->km_total += 120;
   ```
3. **Call methods adding the instance as the first argument, before any other arguments the method may need `object->method_name(object, ...);`.**
   - Methods REQUIRE the instance to be passed explicitly as the first parameter: `object->method_name(object, ...);`.
   - There is no need to cast the object; the method will cast to the appropriate type and provide the correctly casted `self` pointer inside the method.
   - All methods, inherited or new (or interface-based), follow this calling convention.
   ```c
   my_car->move(my_car, 100, 200);
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
   - It is allowed only one event handler per event per object, but the same handler can be registered with multiple different objects.
   - Only the last registered handler per event and object is retained. Subsequent calls to `REGISTER_EVENT` for the same event and object will overwrite the previous handler.
   ```c
   REGISTER_EVENT(Car, on_need_fuel, mycar_lowfuel, my_car);
   ```
6. **To cast the object to the desired type, use `(cast_class *)object`.**
   - Available methods and data members will be the subset available in the cast class.
   - Methods will be the most derived versions.
   ```c
   Vehicle *my_car_as_vehicle = (Vehicle *)my_car;
   ```
7. **Destroy Objects.**
  - Automatic Destruction:
    - If your compiler supports automatic destruction via `__attribute__((__cleanup__))`, objects will be automatically destroyed when they go out of scope.
  - Manual Destruction:
    - You can manually destroy objects, automatic destruction will not take place in this case.
    - For heap-allocated objects, use `DESTROY_FREE(ObjectPtr)` to destroy the object and free memory.
    - For stack-allocated objects, use `DESTROY(Object)` to destroy the object without freeing memory.
    ```c
    DESTROY_FREE(my_car);     // For heap object
    DESTROY(my_elephant);     // For stack object
    // No need to set my_car to NULL; DESTROY_FREE already does that.
    ```
## Creating and using interfaces
Interfaces define contracts that implementing classes must fulfill. When implementing an interface, the class must declare and define all interface members unless they are inherited from a base class.
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
4. **Access the Interface**
The interface cast function `to_InterfaceName` is stored within the object. You can access the interface by calling this function as a member of the object, passing the instance as the first parameter. The function returns an interface struct with pointers to the interface members in the object.
```c
  InterfaceName interface_struct = object->to_InterfaceName(object);
```
The resulting interface accessor for an interface is a struct of type `interface_name`, which contains pointers to all the interface data, methods and events in the object and the object itself.
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
   // Usage: in this case we use two objects of different types.
   swap_movables_position(my_car->to_Moveable(my_car), my_elephant.to_Moveable(&my_elephant)); // Also note that my_elephant is in the stack.
   ```
5. **To raise an event from an interface, use `RAISE_INTERFACE_EVENT(as_interface_obj, event_name[, args])`.**
   - When working with interfaces, events are accessed through pointers to function pointers. Use RAISE_INTERFACE_EVENT to correctly handle the additional indirection.
   - The `as_interface_obj` is the interface struct, which contains the pointers to the interface members in the object.
   - `RAISE_INTERFACE_EVENT` will handle the additional level of indirection due to the interface's structure.
   ```c
   RAISE_INTERFACE_EVENT(movable_struct, on_move, distance_moved);
   ```
## Async methods
- A method can be defined as an asynchronous method if it has `thrd_t` return type, and has one parameter of type `void *` and name `arg`. When an asynchronous method is called, it will return the thread ID immediately and the method will run in a separate thread.
- The declaration in the CLASS_class_name macro of an asynchronous method is exactly the same as for a synchronous method.
- To make a method async simply use `ASYNC_METHOD` instead of `METHOD` in its body definition.
- The self pointer and the `void *arg` parameter are available in the asynchronous method body.
- To return from an asynchronous method beforehand, return the value of the `EXIT_ASYNC` macro: `return EXIT_ASYNC;` This ensures that the thread is detached and the memory allocated for the arguments structure is freed.
- Async methods are supported if the compiler provides <threads.h> support or if the TinyCThread library files tinycthread.h and tinycthread.c are present in the ClassyC directory (source available at: https://github.com/tinycthread/tinycthread).
- If the compiler doesn't support threads, and the TinyCThread library is not available, asynchronous methods will be disabled and the library will fall back to synchronous methods.
- The availability of async methods can be checked with the CLASSYC_THREADS_SUPPORTED macro.
   ```c
   #define CLASS_AsyncSave(Base, Interface, Data, Event, Method, Override) \
       Base(OBJECT) \
       Data(bool, finished) \
       Method(thrd_t, save_data, void *arg)
   ```
   ```c
   ASYNC_METHOD(thrd_t, save_data, void *arg)
       // save data   
       write_data_to_file((my_data_struct *)arg);
       // ...
       self->finished = true;
   END_ASYNC_METHOD
   ```
- Use the `AWAIT` macro to pause the current thread until the asynchronous method finishes. This is particularly useful when subsequent operations depend on the result of an asynchronous call.
- Use the thread identifier returned by the asynchronous method with the `AWAIT` macro.
   ```c
   AWAIT(my_car->save_data(my_car, data_arg));
   ```
   or
   ```c
   // Invoke an asynchronous method
   thrd_t thread_id = my_car->save_data(my_car, data_arg);
   // Wait for the asynchronous method to complete
   AWAIT(thread_id);
   // Continue with operations that depend on the completion of save_data
   if (my_car->finished) {
       printf("Data saved successfully.\n");
   }
   ```
- Use the `THREAD_SLEEP` macro to pause the current thread for a given duration.
   ```c
   ASYNC_METHOD(thrd_t, async_method, void *arg)
       // ...
       THREAD_SLEEP(1000); // Stop this method for 1000 milliseconds (1 second), then continue
       // ...
   END_ASYNC_METHOD
   ```
## Synchronization and object locking
- To ensure thread safety when working with shared resources across multiple threads, ClassyC provides an easy to use mutex support through predefined macros.
- Every object has a `mutex` data member of type `mtx_t`, which can be used to synchronize access to the object.
- Lock an object to read or write to it in a thread-safe way using the `LOCK_OBJECT` macro.
- Unlock an object using the `UNLOCK_OBJECT` macro.
   ```c
   LOCK_OBJECT(my_app);
   my_app->do_something();
   UNLOCK_OBJECT(my_app);
   ```
- Always acquire a lock using `LOCK_OBJECT(object_ptr)` before accessing or modifying shared data members. Release the lock immediately after the critical section using `UNLOCK_OBJECT(object_ptr)`.
- Minimize the scope of locked sections to reduce contention.
- Avoid deadlocks by maintaining a consistent locking order across different objects and methods.
- Consider using higher-level and/or lower-level synchronization mechanisms if necessary.
## ClassyC configuration macros
These macros can be defined before including this header to customize some of the library's naming conventions and error checking.
- **CLASSYC_PREFIX**: Prefix for the global scope identifiers. Default: `#define CLASSYC_PREFIX ClassyC_`
- **CLASSYC_CLASS_NAME**: Used to define the macro holding the class name, by default it is set to `CLASS` but can be changed to any other name to avoid conflicts.
  The resulting macro name will mark the syntax used to declare classes. If CLASSYC_CLASS_NAME is not defined, the default is `CLASS`.
  The library uses CLASSYC_CLASS_NAME internally to access the class name: CLASSYC_CLASS_NAME expands to `CLASS` (or the name defined to it) internally, which itself expands to the name of the class.
   ```c
   #define CLASSYC_CLASS_NAME NEW_CLASS_NAME
   #define NEW_CLASS_NAME Aircraft
   ```
   ```c
   #include "ClassyC.h"
   #define CLASS Aircraft
   ```
- **CLASSYC_CLASS_IMPLEMENT**: Used to define the prefix of the macro holding the class implementation. Default: `#define CLASSYC_CLASS_IMPLEMENT CLASS_`
  If you redefine `CLASSYC_CLASS_IMPLEMENT`, you must also define the x-macro for the `OBJECT` class with the same prefix and the `Data(void, DESTRUCTOR_FUNCTION_POINTER)` member. The (Base, Interface, Data, Event, Method, Override) parameter declaration is mandatory.
   ```c
   #define CLASSYC_CLASS_IMPLEMENT DECLARE_CLASS_
   #define DECLARE_CLASS_OBJECT(Base, Interface, Data, Event, Method, Override) \
       Data(void, DESTRUCTOR_FUNCTION_POINTER) Data(mtx_t, mutex)
   #define DECLARE_CLASS_Aircraft(Base, Interface, Data, Event, Method, Override)
   ```
   ```c
   #define CLASSYC_CLASS_IMPLEMENT CUSTOM_CLASS_
   #define CUSTOM_CLASS_OBJECT(Base, Interface, Data, Event, Method, Override) \
       Data(void, DESTRUCTOR_FUNCTION_POINTER) Data(mtx_t, mutex)
   #define CUSTOM_CLASS_Aircraft(Base, Interface, Data, Event, Method, Override)
   ```
- **CLASSYC_INTERFACE_DECLARATION**: The name of the macro that declares the interface. Default: `#define CLASSYC_INTERFACE_DECLARATION I_`
   ```c
   #define CLASSYC_INTERFACE_DECLARATION NEW_INTERFACE_
   #define NEW_INTERFACE_Moveable(Data, Event, Method)
   ```
- **CLASSYC_DISABLE_RUNTIME_CHECKS**: Disable runtime checks for inheritance depth. Default: not defined.
- **CLASSYC_ENABLE_COMPILE_TIME_CHECKS**: Enable compile-time checks for inheritance depth. Default: not defined.
- **CLASSYC_DISABLE_ASYNC_METHODS**: Disable asynchronous methods (and therefore not require thread support headers). Default: not defined.
- **CLASSYC_DISABLE_THREAD_SLEEP**: Disable thread sleep. In Windows, this avoids the need of inclusion of the windows.h header. Default: not defined.
## Additional notes
- ClassyC supports automatic destruction of objects when they go out of scope if the compiler supports the `__attribute__((__cleanup__))` attribute (e.g., GCC and Clang).
- Class definitions must be at the global scope. Objects can be declared at any scope, but can't be instantiated outside a function. Interfaces are declared in the top-level scope, before any class that uses them.
- A class inherits all the methods, events, data members, and interfaces of its base class and, recursively, its base classes.
- Inherited methods with no new implementation don't need to be included in `CLASS_class_name` or with `METHOD`; they are automatically inherited and available.
- All methods — new, inherited, or overridden — self-register internally in the class constructor: no need to assign funtion pointers or call register functions.
- A `self` pointer is available in all methods, constructors, destructors, and event handlers.
- `CONSTRUCTOR` and `DESTRUCTOR` are mandatory: must be explicitly defined even if no actions are needed.
- The `CONSTRUCTOR` can accept user-defined parameters. The `DESTRUCTOR` must be parameterless.
- The `CONSTRUCTOR` macro must be used after the class definitions and before any methods or the `DESTRUCTOR`.
- The `DESTRUCTOR` can be declared after the `CONSTRUCTOR` and before or after methods, but must not be declared before the `CONSTRUCTOR`.
- `INIT_BASE` requires the arguments to match the base `CONSTRUCTOR` parameters. It should be called before the rest of the `CONSTRUCTOR` code.
- The bool variable is_base is available in both CONSTRUCTOR and DESTRUCTOR to determine if the call is for a base class during inheritance initialization or cleanup.
- `METHOD`, `CONSTRUCTOR`, `DESTRUCTOR`, and `EVENT_HANDLER` need to be used in the global scope.
- The `NEW_INPLACE` macro will zero out the memory at `object_address`: this solves the issue generated by some compilers not setting initial value to 0 on nested anonymous structs.
- No curly braces are needed around the body of the methods, constructors, destructors, or event handlers but can be used for clarity. Not using them won't produce unexpected behavior and is recommended for brevity.
- Since methods are function pointers within the object, you must pass the instance explicitly when calling them.
- Interfaces declared in base classes are automatically available in derived classes, including them again in the derived class will cause name collisions.
- Interfaces can overlap in data members, events, and methods without causing conflicts. It is the class that implements the interface the responsible for the correct implementation (or inheritance) of all the interface members.
- Casting an object to any base class will give access to the subset of data members and methods present in that base class.
  The methods in the casted object will still point to the most derived implementation of the method in the inheritance chain.
- All the valid casts of the object will access the same versions of the data, events, and methods.
- Interface structs returned by `to_interface_name` functions contain pointers to all the interface members in the object.
  This allows access to members and passing the interface object to functions as value.
- Interface event members are pointers to function pointers to handle dynamic event handler registration.
- All method pointers are set to the most derived version of the method in the inheritance chain.
- Methods and events have only one level of indirection; the pointers are not in virtual tables.
- The OBJECT class has a unique implementation pattern: it is the only class that has no base class and is not defined with the `CLASS_` prefix.
- The OBJECT class is the base for all classes, and ensures that every object has the fundamental capabilities required for ClassyC's operation, such as proper destruction and synchronization.
- The library is optimized to reduce levels of indirection and data overhead.
- If the compiler doesn't support automatic destruction, ensure that for every `NEW_ALLOC`, there is a corresponding `DESTROY_FREE` to prevent memory leaks.
- Ensure that `DESTROY_FREE` is only used with heap-allocated objects.
- Make sure to nullify all pointers to the instance after calling `DESTROY_FREE` or `DESTROY` to avoid dangling pointers. The DESTROY_FREE macro for heap-allocated objects already sets the passed pointer to NULL.
- The recursive macros limit the inheritance depth to 9 levels.
  Compile-time checks are available in C11 and later and can be enabled by defining `CLASSYC_ENABLE_COMPILE_TIME_CHECKS` before including the header.
  Runtime checks are enabled by default, but can be disabled by defining `CLASSYC_DISABLE_RUNTIME_CHECKS` before including the header.
  To support deeper inheritance hierarchies, you can extend the recursive macros definitions by adding `RECURSIVE_CLASS_MEMBER_DECLARATION_10`, `RECURSIVE_CLASS_MEMBER_DECLARATION_11`, and so on, making sure that each macro expands to the next one.
- Asynchronous methods will release the thread and the resources when they finish.
- When using asynchronous methods, ensure the number of threads does not exceed the system's capacity.
- If you are using shared objects across multiple threads, ensure they are protected using LOCK_OBJECT and UNLOCK_OBJECT or make sure other proper synchronization mechanisms are in place to avoid race conditions.


## Acknowledgements
- **Unity Test**: I used Unity Test to perform some tests on ClassyC: (https://github.com/ThrowTheSwitch/Unity).
- **TinyCThread**: I used TinyCThread to provide portability for thread support for asynchronous methods: (https://github.com/tinycthread/tinycthread).

*/


#ifndef CLASSYC_H
#define CLASSYC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

/* Class flags for constructor and destructor */
#define IS_BASE_TRUE true
#define IS_BASE_FALSE false


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

#define WRITE_THIRD_ARG(a,b,c,...) c
/* Check if the compiler supports __VA_OPT__ */
#define VA_OPT_SUPPORTED_HELPER(...) WRITE_THIRD_ARG(__VA_OPT__(,),1,0,)
#define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_HELPER(?)
/* Check if the compiler supports ,##__VA_ARGS__ */
#define VA_ARGS_EXT_SUPPORTED_HELPER(...) WRITE_THIRD_ARG(,##__VA_ARGS__,0,1,)
#define VA_ARGS_EXT_SUPPORTED VA_ARGS_EXT_SUPPORTED_HELPER()

#if VA_OPT_SUPPORTED == 1
    #define WITHOUT_COMMA(...) __VA_OPT__(,) __VA_ARGS__
#elif VA_ARGS_EXT_SUPPORTED == 1
    #define WITHOUT_COMMA(...) ,##__VA_ARGS__
#else
    /* If neither __VA_OPT__ nor ,##__VA_ARGS__ is supported, we'll resort to ##__VA_ARGS__ */
    /* (Although it looks like the preprocessor is not able to remove the trailing comma, it's harmless) */
    #define WITHOUT_COMMA(...) ,##__VA_ARGS__
#endif

/* WRITE_NOTHING is used to expand x-macros selectively, by setting WRITE_NOTHING as the macro you want to omit */
#define WRITE_NOTHING(...)

/* The inline keyword was not supported in ANSI C, so we need to check if the compiler supports it */
#if __STDC_VERSION__ >= 199901L
    #define CLASSYC_INLINE inline
#else
    #define CLASSYC_INLINE
#endif


/* The name of the macro that declares the class name: by default it is CLASS */
#ifndef CLASSYC_CLASS_NAME
#define CLASSYC_CLASS_NAME CLASS
#endif

/* The name of the macro that declares the class declaration prefix: by default it is CLASS_ (resulting in CLASS_class_name) */
/* If this is defined, the empty prefix_OBJECT(Base, Interface, Data, Event, Method, Override) macro must also be defined with the same prefix and the Data(void, DESTRUCTOR_FUNCTION_POINTER) and Data(mtx_t, mutex) members.*/
/* The prefix_OBJECT macro is used when traversing the inheritance tree to declare a new class struct */
#ifndef CLASSYC_CLASS_IMPLEMENT
   #define CLASSYC_CLASS_IMPLEMENT CLASS_
   #define CLASS_OBJECT(Base, Interface, Data, Event, Method, Override) Data(void, DESTRUCTOR_FUNCTION_POINTER) Data(mtx_t, mutex)
#endif /* CLASSYC_CLASS_IMPLEMENT */

#define GET_IMPLEMENTS(class_name)CONCAT(CLASSYC_CLASS_IMPLEMENT, class_name)

/* The name of the macro that declares the interface declaration prefix: by default it is I_ (resulting in I_interface_name) */
#ifndef CLASSYC_INTERFACE_DECLARATION
#define CLASSYC_INTERFACE_DECLARATION I_
#endif
#define GET_INTERFACE(interface_name)CONCAT(CLASSYC_INTERFACE_DECLARATION, interface_name)

/* Runtime inheritance depth check */
#ifdef CLASSYC_DISABLE_RUNTIME_CHECKS
    #define CLASSYC_CHECK_INHERITANCE_DEPTH
#else
    #define CLASSYC_CHECK_INHERITANCE_DEPTH                                             \
        int ADD_PREFIX(inheritance_depth) = GET_INHERITANCE_LEVEL(CLASSYC_CLASS_NAME);  \
        if (ADD_PREFIX(inheritance_depth) > 9) {                                        \
            fprintf(stderr, QUOTE(CLASSYC_CLASS_NAME) " inheritance depth (%d) exceeds the maximum supported limit (9 levels)\n", ADD_PREFIX(inheritance_depth)); \
            return NULL;                                                                \
        } 
#endif

/* Static assertion to ensure the inheritance depth does not exceed the maximum limit */
/* Available in C11 and later: disabled by default */
#ifdef CLASSYC_ENABLE_COMPILE_TIME_CHECKS
    /* Check if the compiler supports C11 static assertions */
    #if __STDC_VERSION__ >= 201112L
        #define CLASSYC_CHECK_INHERITANCE_DEPTH_CT \
            _Static_assert(GET_INHERITANCE_LEVEL(CLASSYC_CLASS_NAME) <= 9, "Inheritance depth exceeds the maximum supported limit (9 levels)");
    #else
        #define CLASSYC_CHECK_INHERITANCE_DEPTH_CT
    #endif
#else
    #define CLASSYC_CHECK_INHERITANCE_DEPTH_CT
#endif


/* AUTOMATIC DESTRUCTION AND FREEING OF OBJECTS */
/* If the compiler supports cleanup attribute, auto-destruction of objects is provided when they go out of scope */
#ifdef __GNUC__
  #if (__GNUC__ >= 3)
    #define CLASSYC_AUTO_DESTROY_SUPPORTED 1
  #else
    #define CLASSYC_AUTO_DESTROY_SUPPORTED 0
  #endif
#else
  #define CLASSYC_AUTO_DESTROY_SUPPORTED 0
#endif

#ifndef CLASSYC_DISABLE_ASYNC_METHODS
    #define CLASSYC_DISABLE_ASYNC_METHODS 0
#endif

#ifndef CLASSYC_DISABLE_THREAD_SLEEP
    #define CLASSYC_DISABLE_THREAD_SLEEP 0
#endif

#if CLASSYC_DISABLE_ASYNC_METHODS == 0
    #define CLASSYC_THREADS_SUPPORTED 1
    #if defined(_WIN32) || defined(_WIN64) || __STDC_NO_THREADS__
        #ifndef __has_include
            /* No __has_include extension available, we're optimistically including the library */
            #include "tinycthread.h"
        #else
            /* __has_include extension available, we're including the library only if it exists */
            #if __has_include("tinycthread.h")
                #include "tinycthread.h"
            #else
                /* CLASSYC_DISABLE_ASYNC_METHODS was 0, but the compiler doesn't support threads and tinycthread.h is not available */
                /* Hopefully this redefine will also trigger a compiler warning */
                #define CLASSYC_THREADS_SUPPORTED 0 /* Warning: threads not supported! */
            #endif
        #endif
    #else
        #include <threads.h>
    #endif
#else
    #define CLASSYC_THREADS_SUPPORTED 0
#endif


#if CLASSYC_THREADS_SUPPORTED == 0
    /* Threads not supported: dummy definitions and return values to make sure no compilation errors are generated */
    typedef void *thrd_t;
    typedef int mtx_t;
    /* Thread return values */
    #define thrd_error    0 /**< The requested operation failed */
    #define thrd_success  1 /**< The requested operation succeeded */
    #define thrd_timedout 2 /**< The time specified in the call was reached without acquiring the requested resource */
    #define thrd_busy     3 /**< The requested operation failed because a resource requested by a test and return function is already in use */
    #define thrd_nomem    4 /**< The requested operation failed because it was unable to allocate memory */
    /* Mutex types */
    #define mtx_plain     0
    #define mtx_timed     1
    #define mtx_recursive 2
    #define CLASSYC_MUTEX_INIT(mutex, mutex_type) /* Unnecessary: mutex = 0 */
    #define CLASSYC_MUTEX_DESTROY(mutex) /* Unnecessary: mutex = 1 */
    #define CLASSYC_THREAD_CREATE(thread_id, method_thread_function, args_struct) \
        thread_id = &method_thread_function(args_struct)
    #define CLASSYC_THREAD_DETACH(thread_id)
    /* Macros called by the user */
    /* portable THREAD_SLEEP when threads are not supported */
    #if CLASSYC_DISABLE_THREAD_SLEEP == 0
        #ifdef _WIN32
            #include <windows.h>
            #define THREAD_SLEEP(milliseconds) Sleep(milliseconds)
        #elif __unix__
            #include <time.h>
            #define THREAD_SLEEP(milliseconds) do {                       \
                /* convert milliseconds to seconds and nanoseconds */     \
                long long seconds = milliseconds / 1000;                  \
                long long nanoseconds = (milliseconds % 1000) * 1000000;  \
                struct timespec duration = {seconds, nanoseconds};        \
                nanosleep(&duration, NULL);                               \
                } while (0)
        #else
            /* Unknown system and threads are not supported */
            #define THREAD_SLEEP(milliseconds) do { (void)(milliseconds); } while (0) 
        #endif
    #else
        /* Thread sleep is disabled by the user */
        #define THREAD_SLEEP(milliseconds) do { (void)(milliseconds); } while (0) 
    #endif
    #define LOCK_OBJECT(object_address) (object_address->mutex = 1)
    #define UNLOCK_OBJECT(object_address) (object_address->mutex = 0)
#elif CLASSYC_THREADS_SUPPORTED == 1
    #define CLASSYC_MUTEX_INIT(mutex, mutex_type) mtx_init(&mutex, mutex_type)
    #define CLASSYC_MUTEX_DESTROY(mutex) mtx_destroy(&mutex)
    #define CLASSYC_THREAD_CREATE(thread_id, method_thread_function, args_struct) \
        thrd_create(&thread_id, method_thread_function, args_struct)
    #define CLASSYC_THREAD_DETACH(thread_id) thrd_detach(thread_id)
    /* Macros called by the user */
    #define THREAD_SLEEP(milliseconds)                                \
        do {                                                          \
            /* convert milliseconds to seconds and nanoseconds */     \
            long long seconds = milliseconds / 1000;                  \
            long long nanoseconds = (milliseconds % 1000) * 1000000;  \
            struct timespec duration = {seconds, nanoseconds};        \
            thrd_sleep(&duration, NULL);                              \
        } while (0)
    #define LOCK_OBJECT(object_address) mtx_lock(&object_address->mutex)
    #define UNLOCK_OBJECT(object_address) mtx_unlock(&object_address->mutex)
    /* Structure to pass arguments to the method thread function: it'll be given to the user's method code extracting self and arg */
    typedef struct {
        void *self_void;
        void *arg;
        thrd_t thread_id;
    } ADD_PREFIX(AsyncArgs);    
#endif

/* Header of the class struct */
/* Declare a class struct and its type name */
#define STRUCT_HEADER(struct_name)             \
    typedef struct struct_name struct_name;    \
    struct struct_name
/* COMPONENTS OF THE CLASS STRUCT */
#define WRITE_METHOD_POINTER(ret_type, method_name, ...) ret_type (*method_name)(void *self_void WITHOUT_COMMA(__VA_ARGS__));
#define WRITE_DATA_MEMBER(type, member_name) type member_name;
#define WRITE_EVENT_MEMBER(event_name, ...) void (*event_name)(void *self_void WITHOUT_COMMA(__VA_ARGS__));
/* Interfaces create a function pointer to the interface cast function. */
/* The interface cast function will return an interface struct with pointers to the class members */
#define WRITE_INTERFACE_FUNCTION_POINTER(interface_name) interface_name (*CONCAT(to_, interface_name))(void *self_void);

/* Every class has a destructor function pointer and a mutex for thread safety */
/* They are introduced as data members of the OBJECT class struct that is inherited by all classes */
#define DESTRUCTOR_FUNCTION_POINTER (*_destructor)(void *self_void)

/* OBJECT class: the base class of all classes. It only contains the destructor function pointer and a mutex for thread safety. */
/* OBJECT is a fixed name and doesn't use the CLASSYC_CLASS_NAME macro */ 
/* OBJECT class struct */
STRUCT_HEADER(OBJECT) {
    /* Having a pointer to the destructor function helps DESTROY calling it without knowing the class name */
    /* It is inherited by all classes, so every class has a pointer to the destructor function. Returns void. */
    WRITE_DATA_MEMBER(void, DESTRUCTOR_FUNCTION_POINTER)
    /* Mutex for thread safety: every instance has its own mutex to automate locking and unlocking */
    WRITE_DATA_MEMBER(mtx_t, mutex)
};
/* Prototypes for OBJECT class functions */
static CLASSYC_INLINE void *PREFIXCONCAT(OBJECT, _constructor)(void *self_void);
static CLASSYC_INLINE void *PREFIXCONCAT(OBJECT, _user_constructor)(bool is_base, void *self_void);
static CLASSYC_INLINE void PREFIXCONCAT(OBJECT, _destructor)(void *self_void);
static CLASSYC_INLINE void PREFIXCONCAT(OBJECT, _user_destructor)(bool is_base, void *self_void);
/* OBJECT class constructor function: only sets the destructor function pointer */
static CLASSYC_INLINE void *PREFIXCONCAT(OBJECT, _constructor)(void *self_void) { 
    OBJECT *self = (OBJECT *)self_void; 
    self->_destructor = PREFIXCONCAT(OBJECT, _destructor); 
    /* Should not be called directly */ 
    return self; 
}
static CLASSYC_INLINE void *PREFIXCONCAT(OBJECT, _user_constructor)(bool is_base, void *self_void) {
    return self_void;
}
/* OBJECT class destructor function */
static CLASSYC_INLINE void PREFIXCONCAT(OBJECT, _destructor)(void *self_void) { /* OBJECT destructor does nothing */ }
static CLASSYC_INLINE void PREFIXCONCAT(OBJECT, _user_destructor)(bool is_base, void *self_void) { /* OBJECT destructor does nothing */ }

/* BASIC MACROS */
/* Get the name of the base class from the IMPLEMENTS macro */
#define WRITE_BASE_NAME(base_to_call)base_to_call
#define X_GET_BASE_NAME(class) GET_IMPLEMENTS(class)(WRITE_BASE_NAME, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING)

/* Set the _destructor pointer to the class destructor function */
/* (constructor pointer not needed, as construction is invoked by NEW_ALLOC or NEW_INPLACE calling the constructor function directly) */
#define WRITE_SET_DESTRUCTOR_FUNC_POINTER(class_name) \
    self->_destructor = PREFIXCONCAT(class_name, _destructor); 

/* Set the method pointers to the functions of the class */
#define SET_METHOD_PTR(ret_type, method_name, ...) \
    ((CLASSYC_CLASS_NAME *)self_void)->method_name = PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name); 

/* Write ALL the interface instances, data members and methods (searching only for new ones) the class implements in the class struct */
/* by recursively crossing the inheritance tree and creating nested anonymous structs */
#define WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) \
        GET_IMPLEMENTS(class)(WRITE_NOTHING, WRITE_INTERFACE_FUNCTION_POINTER, WRITE_DATA_MEMBER, WRITE_EVENT_MEMBER, WRITE_METHOD_POINTER, WRITE_NOTHING) \


#define RECURSIVE_CLASS_MEMBER_DECLARATION_9(class)  \
    struct { WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_8(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_9, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_7(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_8, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_6(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_7, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_5(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_6, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_4(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_5, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_3(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_4, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_2(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_3, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATION_1(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_2, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) } ; 
#define RECURSIVE_CLASS_MEMBER_DECLARATIONS(class)  \
    struct { GET_IMPLEMENTS(class)(RECURSIVE_CLASS_MEMBER_DECLARATION_1, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
             WRITE_CLASS_STRUCT_NESTED_MEMBERS(class) };

/* Macros to count inheritance levels to help with static assertions */
#define INHERITANCE_LEVEL_1(class) + 1
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
    static CLASSYC_INLINE ret_type PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name)(void *self_void WITHOUT_COMMA(__VA_ARGS__)); 
#define X_METHOD_FUNC_PROTOTYPES(class_name) \
    /* Writes the method prototypes for the class: new methods and overridden methods */ \
    GET_IMPLEMENTS(class_name)(WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_METHOD_FUNC_PROTOTYPE, WRITE_METHOD_FUNC_PROTOTYPE) 


/* INTERFACES */
/* Macros to create interface structs with its members on CREATE_INTERFACE */
/* Method pointer declaration in the interface struct */
#define WRITE_I_METHOD_PTR(ret_type, method_name, ...) \
    ret_type (*method_name)(void *self_void WITHOUT_COMMA(__VA_ARGS__));
/* Data member pointerdeclaration in the interface struct */
#define WRITE_I_DATA_MEMBER(type, member_name) \
    /* A pointer to the data member */\
    type *member_name;
/* Event pointer (to handler function pointer) declaration in the interface struct */
#define WRITE_I_EVENT_MEMBER(event_name, ...) \
    /* Not a pointer to the function, but a pointer to the function pointer */\
    /* That way, if event handlers are registered later, we don't have to reassign the pointer */\
    void (**event_name)(void *self_void WITHOUT_COMMA(__VA_ARGS__));
/* Write the interface struct members */
#define WRITE_INTERFACE_STRUCT(interface_name) \
    GET_INTERFACE(interface_name)(WRITE_I_DATA_MEMBER, WRITE_I_EVENT_MEMBER, WRITE_I_METHOD_PTR)
/* Interface struct definition */
#define CREATE_INTERFACE(interface_name)         \
   typedef struct interface_name interface_name; \
   struct interface_name {                       \
       void *self;                               \
       WRITE_INTERFACE_STRUCT(interface_name)    \
   };


/* Interface pointer initializers to fill the interface struct with pointers to the class members */
#define WRITE_I_METHOD_PTR_INITIALIZER(ret_type, method_name, ...) \
    /* Copies the method pointer to the interface */\
    .method_name = self->method_name,
#define WRITE_I_DATA_MEMBER_INITIALIZER(type, member_name) \
    /* Copies the pointer to the data member to the interface */\
    .member_name = &self->member_name,
#define WRITE_I_EVENT_MEMBER_INITIALIZER(event_name, ...) \
    /* Stores the address of the function pointer in the interface */\
    .event_name = &self->event_name,

/* INTERFACE CAST FUNCTIONS in the form class_name_to_interface_name */
#define WRITE_INTERFACE_CAST_FUNCTION(interface_name) \
    interface_name TRICAT(CLASSYC_CLASS_NAME, _to_, interface_name)(void *self_void);  \
    interface_name TRICAT(CLASSYC_CLASS_NAME, _to_, interface_name)(void *self_void) { \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void;                    \
        interface_name interface_struct;                                               \
        interface_struct = (interface_name){                                           \
            GET_INTERFACE(interface_name)(WRITE_I_DATA_MEMBER_INITIALIZER, WRITE_I_EVENT_MEMBER_INITIALIZER, WRITE_I_METHOD_PTR_INITIALIZER) \
            .self = self                                                               \
        };                                                                             \
        return interface_struct;                                                       \
    }
#define WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) \
    GET_IMPLEMENTS(class)(WRITE_NOTHING, WRITE_INTERFACE_CAST_FUNCTION, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \

#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_9(class)  \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_8(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_9, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_7(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_8, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_6(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_7, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_5(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_6, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_4(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_5, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_3(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_4, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_2(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_3, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS_1(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_2, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_INTERFACE_CAST_FUNCTIONS(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_INTERFACE_CAST_FUNCTIONS_1, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_INTERFACE_CAST_FUNCTIONS(class) 

/* REGISTER INTERFACE CAST FUNCTIONS */
#define WRITE_REGISTER_INTERFACE_CAST_FUNCTION(interface_name) \
    self->CONCAT(to_, interface_name) = TRICAT(CLASSYC_CLASS_NAME, _to_, interface_name);
#define WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class)  \
    GET_IMPLEMENTS(class)(WRITE_NOTHING, WRITE_REGISTER_INTERFACE_CAST_FUNCTION, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_9(class)  \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_8(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_9, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_7(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_8, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_6(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_7, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_5(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_6, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_4(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_5, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_3(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_4, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_2(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_3, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_1(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_2, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 
#define RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS(class)  \
    GET_IMPLEMENTS(class)(RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS_1, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING) \
    WRITE_CLASS_REGISTER_INTERFACE_CAST_FUNCTIONS(class) 



/* CONSTRUCTOR, DESTRUCTOR, INIT... */
/* Initialize the base class: call this within the 'CONSTRUCTOR' of the derived class to run the 'CONSTRUCTOR' of the base class. */
#define INIT_BASE(...) \
    PREFIXCONCAT(X_GET_BASE_NAME(CLASSYC_CLASS_NAME), _user_constructor)(IS_BASE_TRUE, self WITHOUT_COMMA(__VA_ARGS__))

/* Constructor macro, this is where most of the logic for class definition is implemented */
#define CONSTRUCTOR(...)\
    /* Compile-time assertion (available in C11 and later) to ensure the inheritance depth does not exceed the maximum limit */ \
    CLASSYC_CHECK_INHERITANCE_DEPTH_CT                                  \
    /* Declare the class struct */                                      \
    STRUCT_HEADER(CLASSYC_CLASS_NAME) {                                 \
        /* Include all the members of the class struct */               \
        RECURSIVE_CLASS_MEMBER_DECLARATIONS(CLASSYC_CLASS_NAME)         \
    } ;                                                                 \
    /* Prototypes for the destructor and constructor class functions */ \
    static CLASSYC_INLINE void PREFIXCONCAT(CLASSYC_CLASS_NAME, _destructor)(void *self_void); \
    static CLASSYC_INLINE void PREFIXCONCAT(CLASSYC_CLASS_NAME, _ptr_destructor)(CLASSYC_CLASS_NAME **self_ptr); \
    static CLASSYC_INLINE void PREFIXCONCAT(CLASSYC_CLASS_NAME, _user_destructor)(bool is_base, void *self_void); \
    static CLASSYC_INLINE void * PREFIXCONCAT(CLASSYC_CLASS_NAME, _constructor)(void *self_void); \
    static CLASSYC_INLINE void * PREFIXCONCAT(CLASSYC_CLASS_NAME, _user_constructor)(bool is_base, void *self_void WITHOUT_COMMA(__VA_ARGS__)); \
    /* Write the prototypes for new and overridden methods */           \
    X_METHOD_FUNC_PROTOTYPES(CLASSYC_CLASS_NAME)                        \
    /* Interface cast functions */                                      \
    RECURSIVE_INTERFACE_CAST_FUNCTIONS(CLASSYC_CLASS_NAME)              \
    /* Constructor function */                                          \
    static CLASSYC_INLINE void* PREFIXCONCAT(CLASSYC_CLASS_NAME, _constructor)(void * self_void) { \
        /* This function is used to allocate memory for the object (if needed) and set its basic function and method pointers */ \
        /* Runtime check for inheritance depth: disable by defining CLASSYC_DISABLE_RUNTIME_CHECKS */ \
        CLASSYC_CHECK_INHERITANCE_DEPTH                                 \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void;     \
        if (self_void == NULL) {                                        \
            /* No object pointer provided: allocate memory for the object in the heap */ \
            self = (CLASSYC_CLASS_NAME *)calloc(1, sizeof(CLASSYC_CLASS_NAME)); \
            self_void = self;                                           \
            if (self == NULL) {                                         \
                /* Allocation failure */                                \
                return NULL;                                            \
            }                                                           \
        } else {                                                        \
            /* Object pointer provided (no need to allocate memory for it): use it */ \
            self = (CLASSYC_CLASS_NAME *)self_void;                     \
        }                                                               \
        /* Call the base class constructor */                           \
        PREFIXCONCAT(X_GET_BASE_NAME(CLASSYC_CLASS_NAME), _constructor)(self); \
        /* Set destructor pointer to the class destructor function */   \
        WRITE_SET_DESTRUCTOR_FUNC_POINTER(CLASSYC_CLASS_NAME)           \
        /* Set method pointers to the functions of the class */         \
        /* as constructors are executed in the order of inheritance, overridden methods are set last */ \
        GET_IMPLEMENTS(CLASSYC_CLASS_NAME)(WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, WRITE_NOTHING, SET_METHOD_PTR, SET_METHOD_PTR) \
        /* Register interface cast functions */                         \
        RECURSIVE_REGISTER_INTERFACE_CAST_FUNCTIONS(CLASSYC_CLASS_NAME) \
        /* Initialize the mutex for thread safety */                    \
        CLASSYC_MUTEX_INIT(self->mutex, mtx_recursive);                 \
        return self;                                                    \
    }                                                                   \
    /* User constructor function */                                     \
    static CLASSYC_INLINE void* PREFIXCONCAT(CLASSYC_CLASS_NAME, _user_constructor)(bool is_base, void * self_void WITHOUT_COMMA(__VA_ARGS__)) { \
        if (!is_base) {                                                 \
            /* Only for the instanced objects, not for the base classes: run the 'real' constructor */\
             self_void = PREFIXCONCAT(CLASSYC_CLASS_NAME, _constructor)(self_void); \
             if (!self_void) {                                          \
                /* Failure, pointer to the object is NULL */            \
                return NULL;                                            \
             }                                                          \
        }                                                               \
        CLASSYC_CLASS_NAME * self = (CLASSYC_CLASS_NAME *)self_void;    \
        /* User constructor code follows, it will be executed even if is_base is true when INIT_BASE is called */ \

#define END_CONSTRUCTOR \
        return self; \
    }

/* Destructor macro */
#define DESTRUCTOR() \
     /* Contains the destructor code for the class. Then on END_DESTRUCTOR invokes the base class destructor */\
     /* _ptr_destructor is used when a pointer marked for auto-destruction gets out of scope */\
    static CLASSYC_INLINE void PREFIXCONCAT(CLASSYC_CLASS_NAME, _ptr_destructor)(CLASSYC_CLASS_NAME **self_ptr) { \
           /* Call the destructor for the class */                       \
           PREFIXCONCAT(CLASSYC_CLASS_NAME, _destructor)(*self_ptr);     \
           /* Free the memory allocated for the object and nullify ptr */\
           if (*self_ptr) {                                              \
               free(*self_ptr);                                          \
               *self_ptr = NULL;                                         \
           }                                                             \
    }                                                                    \
    static CLASSYC_INLINE void PREFIXCONCAT(CLASSYC_CLASS_NAME, _destructor)(void *self_void) { \
        /* In order to prevent multiple calls to the destructor, we use the _destructor pointer as a marker */ \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void;      \
        if (!self || !self->_destructor) {                               \
             /* Destructor already called: do nothing */                 \
             return;                                                     \
        } else {                                                         \
            /* Destructor called for the first time: run the user destructor */ \
        }                                                                \
        /* Call user destructor */                                       \
        PREFIXCONCAT(CLASSYC_CLASS_NAME, _user_destructor)(IS_BASE_FALSE, self); \
        /* Destroy the mutex for thread safety */                        \
        CLASSYC_MUTEX_DESTROY(self->mutex);                              \
    }                                                                    \
    /* User destructor function */                                       \
    static CLASSYC_INLINE void PREFIXCONCAT(CLASSYC_CLASS_NAME, _user_destructor)(bool is_base, void *self_void) { \
        if (!self_void) {                                                \
            /* NULL self_void can't be casted or processed */            \
            return;                                                      \
        }                                                                \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void;      \
        /* User destructor code follows */ 

#define END_DESTRUCTOR \
        /* Call the base class destructor (this will happen recursively upwards in the inheritance tree) */ \
        if (self) {                                                                                         \
            /* Call the base class destructor with is_base set to true */                                   \
            PREFIXCONCAT(X_GET_BASE_NAME(CLASSYC_CLASS_NAME), _user_destructor)(IS_BASE_TRUE, self);        \
            /* Mark the destructor as called by setting the function pointer to NULL. Destructors are called once. */\
            self->_destructor = NULL;                                                                       \
        }                                                                                                   \
    }

/* METHOD CREATION */
#define METHOD(ret_type, method_name, ...)                                                                    \
    static CLASSYC_INLINE ret_type PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name)(void *self_void WITHOUT_COMMA(__VA_ARGS__)) { \
        CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void;                                           \
        /* User method code */
#define END_METHOD \
    }

/* Threads support for asynchronous methods */
#if CLASSYC_THREADS_SUPPORTED
    #define ASYNC_METHOD(ret_type, method_name, void_ptr_arg) \
        /* Thread function prototype */                                                                     \
        static CLASSYC_INLINE int PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name##_thread)(void *void_arg_struct); \
        /* Wrapper function that starts the thread with arguments */                                        \
        static CLASSYC_INLINE thrd_t PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name)(void *self_void, void *arg) { \
            CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)self_void;                                     \
            /* Allocate and assign arguments */                                                             \
            ADD_PREFIX(AsyncArgs) *args_struct = calloc(1, sizeof(ADD_PREFIX(AsyncArgs)));                  \
            if (!args_struct) {                                                                             \
                fprintf(stderr, "Error: Failed to allocate memory for async method arguments '%s'.\n", #method_name); \
                return thrd_error;                                                                          \
            }                                                                                               \
            args_struct->self_void = self_void;                                                             \
            args_struct->arg = arg;                                                                         \
            /* Start the thread */                                                                          \
            if (CLASSYC_THREAD_CREATE(args_struct->thread_id, PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name##_thread), args_struct) != thrd_success) { \
                /* Handle thread creation error */                                                          \
                fprintf(stderr, "Error: Failed to create thread for method '%s'.\n", #method_name);         \
                free(args_struct); /* don't leave allocated memory on the heap */                           \
                return thrd_error;                                                                          \
            }                                                                                               \
            /* Return immediately */                                                                        \
            return args_struct->thread_id;                                                                  \
        }                                                                                                   \
        /* Thread function where the user's code will run */                                                \
        static CLASSYC_INLINE int PREFIXCONCAT(CLASSYC_CLASS_NAME, _##method_name##_thread)(void *void_arg_struct){ \
            ADD_PREFIX(AsyncArgs) *args_struct = (ADD_PREFIX(AsyncArgs) *)void_arg_struct;                  \
            CLASSYC_CLASS_NAME *self = (CLASSYC_CLASS_NAME *)args_struct->self_void;                        \
            thrd_t ADD_PREFIX(thread_id) = args_struct->thread_id;                                          \
            void *arg = args_struct->arg;                                                                   \
            /* User's code starts here */                                                                   \

    #define END_ASYNC_METHOD            \
            /* User's code ends here */ \
            return EXIT_ASYNC;          \
        } 

    #define EXIT_ASYNC                                                    \
        /* Free the arguments structure to prevent memory leak, */        \
        /* then detach the thread when the thread ends (at the end of the function) so it cleans up after itself, */ \
        /* and evaluate to the int return value of the detachment */      \
        (free(args_struct), CLASSYC_THREAD_DETACH(ADD_PREFIX(thread_id)))
        
    #define AWAIT(thread_id) \
        thrd_join(thread_id, NULL)
#else
/* Synchronous fallback if threads are not supported */
#define ASYNC_METHOD(ret_type, method_name, void_ptr_arg) \
    /* Asynchronous methods not supported by the compiler: Synchronous method fallback */ \
    METHOD(thrd_t, method_name, void_ptr_arg)             \
        /* User method code, with access to arg */

#define END_ASYNC_METHOD  \
        return (thrd_t)0; \
    END_METHOD

#define EXIT_ASYNC ((thrd_t)0)

#define AWAIT(thread_id)                                   \
    /* Threads not supported: dummy await that ensures the thread is called in case AWAIT(object->method(object, arg)) is used */ \
    do {                                                   \
        thrd_t ADD_PREFIX(dummy_await_thread) = thread_id; \
    } while (0)
#endif

/* EVENTS */
#define GET_EVENT_FUNC_NAME(class_name, event_name, handler_ID) \
    CONCAT(TRICAT(ClassyC_, class_name, _), TRICAT(event_name, _hdlr_, handler_ID))

/* Define an event handler for an instance. Handler_ID is a unique ID for the event handler (letters, numbers, _). */
#define EVENT_HANDLER(class_name, event_name, handler_ID, ...)                                                   \
    static CLASSYC_INLINE void GET_EVENT_FUNC_NAME(class_name, event_name, handler_ID)(void *self_void WITHOUT_COMMA(__VA_ARGS__));  \
    static CLASSYC_INLINE void GET_EVENT_FUNC_NAME(class_name, event_name, handler_ID)(void *self_void WITHOUT_COMMA(__VA_ARGS__)) { \
        class_name *self = (class_name *)self_void; \
        /* User code for the event */ 

#define END_EVENT_HANDLER }

/* Register an event handler for an instance. Handler_ID is a unique ID for the defined event handler (letters, numbers, _). */
/* If another event handler was registered, it is overwritten. */
#define REGISTER_EVENT(class_name, event_name, handler_ID, instance_name)                    \
    do {                                                                                     \
        instance_name->event_name = GET_EVENT_FUNC_NAME(class_name, event_name, handler_ID); \
    } while (0)

/* Raise an event: use inside any function: RAISE_EVENT(self, event_name[, args]) */
/* The event handler is executed if it has been registered (pointer not NULL). */
#define RAISE_EVENT(instance_name, event_name, ...)                                                      \
    do {                                                                                                 \
        if (instance_name->event_name) instance_name->event_name ((void *)instance_name WITHOUT_COMMA(__VA_ARGS__)); \
    } while (0)

/* Raise an event from an interface: use inside any function: RAISE_INTERFACE_EVENT(interface_struct, event_name[, args]) */
/* As functions manipulating the interface struct may not be aware of the actual class implementing the interface, we need this extra macro. */
#define RAISE_INTERFACE_EVENT(interface_struct, event_name, ...)                             \
    /* We need to dereference the pointer to the function pointer stored in the interface */ \
    do {                                                                                     \
        if (interface_struct.event_name && (*interface_struct.event_name)) {                 \
            (*interface_struct.event_name)(interface_struct.self WITHOUT_COMMA(__VA_ARGS__));            \
        }                                                                                    \
    } while (0)

/* MACROS FOR INSTANCE DECLARATION */
/* They provide automatic destruction of objects when they go out of scope if the compiler supports cleanup attribute */
/* Other than that, it might be clearer to declare with normal C syntax*/
#if CLASSYC_AUTO_DESTROY_SUPPORTED == 1
    #define CLEANUP_ATTRIBUTE(class_name, destructor_name) \
        __attribute__ ((__cleanup__(PREFIXCONCAT(class_name, destructor_name))))
#else
    /* Fallback to no __attribute__ ((__cleanup__)) support */
    #define CLEANUP_ATTRIBUTE(class_name, destructor_name)
#endif

/* MACROS FOR OBJECT ALLOCATION AND INITIALIZATION */

/* Macros for object creation syntax */
/* Heap allocation: AUTODESTROY_PTR(Class) *ptr = NEW_ALLOC(class_name, ...); */
/* Stack allocation: AUTODESTROY(Class) obj; NEW_INPLACE(class_name, &obj, ...); */
#define AUTODESTROY_PTR(class_name)  class_name CLEANUP_ATTRIBUTE(class_name, _ptr_destructor)
#define AUTODESTROY(class_name)      class_name CLEANUP_ATTRIBUTE(class_name, _destructor)
#define NEW_ALLOC(class_name, ...)   PREFIXCONCAT(class_name, _user_constructor)(IS_BASE_FALSE, NULL WITHOUT_COMMA(__VA_ARGS__))
#define NEW_INPLACE(class_name, object_address, ...)   \
    /* Sets the already allocated memory zero; needed to avoid undefined values in nested anonymous structs */ \
    memset(object_address, 0, sizeof(class_name));     \
    /* Call the constructor */                        \
    PREFIXCONCAT(class_name, _user_constructor)(IS_BASE_FALSE, object_address WITHOUT_COMMA(__VA_ARGS__))


/* MACROS FOR OBJECT DESTRUCTION */
/* DESTROY_FREE the object and free the allocated memory */
#define DESTROY_FREE(obj_name)                         \
    do {                                               \
        if ((obj_name) && ((obj_name)->_destructor)) { \
            (obj_name)->_destructor((obj_name));       \
            free((obj_name));                          \
            /* Nullifying the pointer to prevent auto-destructor to call free again */ \
            obj_name = NULL;                           \
        }                                              \
    } while (0)

/* DESTROY macro to call destructor of an object allocated without freeing the memory */
#define DESTROY(object_name)                                 \
    do {                                                     \
        if (object_name._destructor) {                       \
            object_name._destructor((void *)&(object_name)); \
            /* DESTROY doesn't free the memory.*/            \
            /* It also doesn't set the pointer to NULL, for it receives the object (stack or dereferenced heap) */ \
        }                                                    \
    } while (0)


#endif /* CLASSYC_H */

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