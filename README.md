# ClassyC
A library for OOP in C that allows simple syntax for creating and using classes with polymorphism, inheritance, interfaces, events, and automatic method registration. ClassyC is an experimental and recreational project. It is not intended for production use.

## Creating a class
1. **Include `ClassyC.h`.**
2. **Define CLASS with the name of the class.** To avoid redefinition compiler warnings, use `#undef CLASS` before every new class.
   Example:
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
   Example:
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
   Examples:
   ```c
   CONSTRUCTOR() END_CONSTRUCTOR

   CONSTRUCTOR(int km_total_when_bought)
     INIT_BASE();
     self->position = 0;
     self->km_total = km_total_when_bought;
     self->km_since_last_fuel = 0;
   END_CONSTRUCTOR
   ```
5. **Use `DESTRUCTOR()` macro** and include cleanup code before the `END_DESTRUCTOR` macro. Instance is available as `self`.   
   Example:
   ```c
   DESTRUCTOR() END_DESTRUCTOR
   ```
6. **Use `METHOD(ret_type, method_name, ...)` macro** to implement every method declared in the `CLASS_class_name` macro.
   - Within methods, the current object is accessed using the `self` pointer.
   - Close the method implementation with `END_METHOD`.
   Example:
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
   Example:
   ```c
   Car *my_car = CREATE(Car, 10000);
   ```
2. **Access data members directly (`object->member_name = value;`).**
   Example:
   ```c
   my_car->km_total = 10000;
   ```
3. **Call methods adding the instance as the first argument, before any other arguments the method may need (`object->method_name(object, ...)`).**
   - Methods REQUIRE the instance to be passed explicitly as the first parameter. A `CALL` macro is provided for convenience.
   - To call methods using the `CALL` macro use `CALL(instance, method[, args])`.
   - There is no need to cast the object; the method will cast to the appropriate type and provide the correctly casted `self` pointer inside the method.
   - All methods, inherited or new (or interface-based), follow this calling convention.
   Examples:
   ```c
   my_car->move(my_car, 100, 200);
   CALL(my_car, move, 100, 200);  // Alternative syntax, expands to the above
   ```
4. **Define event handlers using `EVENT_HANDLER(class_name, event_name, handler_ID, ...) [code] END_EVENT_HANDLER`** in the global scope (outside of any function). `handler_ID` is a unique ID for the event handler (letters, numbers, `_`).
   - Within event handlers, the instance is accessed using the `self` pointer.
   Example:
   ```c
   EVENT_HANDLER(Car, on_need_fuel, mycar_lowfuel, int km_to_collapse)
     if (km_to_collapse < 10) {
       printf("Alert! Last refuel was %d km ago. Need to refuel in less than %d km!\n", self->km_since_last_fuel, km_to_collapse);
     }
   END_EVENT_HANDLER
   ```
5. **Register an event handler with an object using: `REGISTER_EVENT(class_name, event_name, handler_ID, object)`**.
   - Only one handler can be registered per event and object. Subsequent calls to `REGISTER_EVENT` for the same event and object will overwrite the previous handler.
   Example:
   ```c
   REGISTER_EVENT(Car, on_need_fuel, mycar_lowfuel, my_car);
   ```
6. **To cast the object to the desired type, use `(cast_class *)object`.**
   - Available methods and data members will be the subset available in the cast class.
   - Methods will be the most derived versions.
   Example:
   ```c
   Vehicle *my_car_as_vehicle = (Vehicle *)my_car;
   ```
7. **Call `DESTROY(object)` to free the memory allocated for the object.** It is recommended to nullify the pointer after `DESTROY`ing.
   Example:
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
   Example:
   ```c
   #define I_Moveable(Data, Event, Method) \
     Data(int, position) \
     Event(on_move, int distance_moved) \
     Method(void, move, int speed, int distance)
   ```
2. **Call `CREATE_INTERFACE(interface_name)` once right after the interface declaration.**
   - This creates a new type of interface struct with the pointers to the members declared in the interface and a self pointer to the class instance.
   - It is required to call this macro after the interface declaration and before any class that implements the interface.
   Example:
   ```c
   CREATE_INTERFACE(Moveable)
   ```
3. **To implement the interface, make sure the class declares or inherits all the members and includes the interface name in its `CLASS_class_name` interfaces list.**
   Example:
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
   Example:
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
   Example:
   ```c
   RAISE_INTERFACE_EVENT(movable_struct, on_move, distance_moved);
   ```
## ClassyC configuration macros
These macros can be defined before including this header to customize some of the library's naming conventions and error checking.
- **CLASSYC_PREFIX**: Prefix for the global scope identifiers. Default: `#define CLASSYC_PREFIX ClassyC_`
- **CLASSYC_CLASS_NAME**: Used to define the macro holding the class name, by default it is set to `CLASS` but can be changed to any other name to avoid conflicts.
  Example:
  ```c
  #define CLASSYC_CLASS_NAME NEW_CLASS_NAME
  #define NEW_CLASS_NAME Aircraft
  ```
- **CLASSYC_CLASS_IMPLEMENT**: Used to define the prefix of the macro holding the class implementation. Default: `#define CLASSYC_CLASS_IMPLEMENT CLASS_`
  If you redefine `CLASSYC_CLASS_IMPLEMENT`, you must also define an empty macro for the `OBJECT` class with the same prefix.
  Example:
  ```c
  #define CLASSYC_CLASS_IMPLEMENT DECLARE_CLASS_
  #define DECLARE_CLASS_OBJECT(Base, Interface, Data, Event, Method, Override)
  #define DECLARE_CLASS_Aircraft(Base, Interface, Data, Event, Method, Override)
  ```
  Example:
  ```c
  #define CLASSYC_CLASS_IMPLEMENT CUSTOM_CLASS_
  #define CUSTOM_CLASS_OBJECT(Base, Interface, Data, Event, Method, Override)
  #define CUSTOM_CLASS_Aircraft(Base, Interface, Data, Event, Method, Override)
  ```
- **CLASSYC_INTERFACE_DECLARATION**: The name of the macro that declares the interface. Default: `#define CLASSYC_INTERFACE_DECLARATION I_`
  Example:
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
