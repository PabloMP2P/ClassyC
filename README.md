# ClassyC
A portable library for OOP in C that allows simple syntax for creating and using classes with polymorphism, inheritance, interfaces, events, automatic method registration and automatic destruction of objects and freeing of memory.


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
   - All classes must inherit from another class, or the OBJECT class, so all objects inherit the OBJECT class members (a destructor).
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
   - Optionally, call `BASE_METHOD(method_name[, optional_parameters]);` to run the base class method code.
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
   ```c
   METHOD(void, move, int speed, int distance)
       BASE_METHOD(move, speed, distance);
       // Additional code specific to this class
       // ...
   END_METHOD
   ```
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
       Data(void, DESTRUCTOR_FUNCTION_POINTER)
   #define DECLARE_CLASS_Aircraft(Base, Interface, Data, Event, Method, Override)
   ```
   ```c
   #define CLASSYC_CLASS_IMPLEMENT CUSTOM_CLASS_
   #define CUSTOM_CLASS_OBJECT(Base, Interface, Data, Event, Method, Override) \
       Data(void, DESTRUCTOR_FUNCTION_POINTER)
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
- If you are using shared objects across multiple threads, ensure they are protected using mutexes or make sure other proper synchronization mechanisms are in place to avoid race conditions.


## Acknowledgements
- **Unity Test**: I used Unity Test to perform some tests on ClassyC: (https://github.com/ThrowTheSwitch/Unity).
