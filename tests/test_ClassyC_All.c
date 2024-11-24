// test_ClassyC_All.c
#include "unity.h"
#include "../ClassyC.h"
#include <stdlib.h>







/* Test Case: Class Creation and Destruction */
#undef CLASS
#define CLASS TestObject
#define CLASS_TestObject(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) \
    Data(int, value) \
    Method(int, get_value) \
    Method(void, set_value, int)

CONSTRUCTOR(int initial_value)
    self->value = initial_value;
END_CONSTRUCTOR

DESTRUCTOR()
END_DESTRUCTOR

METHOD(int, get_value)
    return self->value;
END_METHOD

METHOD(void, set_value, int new_value)
    self->value = new_value;
END_METHOD

void test_ClassCreation(void) {
    AUTODESTROY_PTR(TestObject) *obj = NEW_ALLOC(TestObject, 10);
    AUTODESTROY(TestObject) obj2;
    NEW_INPLACE(TestObject, &obj2, 20);
    TEST_ASSERT_NOT_NULL(obj);
    TEST_ASSERT_NOT_NULL(&obj2);
    TEST_ASSERT_EQUAL_INT(10, obj->value);
    TEST_ASSERT_EQUAL_INT(20, obj2.value);
    DESTROY_FREE(obj);
    DESTROY(obj2);
}

void test_ClassMethods(void) {
    AUTODESTROY_PTR(TestObject) *obj = NEW_ALLOC(TestObject, 20);
    TEST_ASSERT_NOT_NULL(obj);
    TEST_ASSERT_EQUAL_INT(20, obj->get_value(obj));

    obj->set_value(obj, 30);
    TEST_ASSERT_EQUAL_INT(30, obj->get_value(obj));

    DESTROY_FREE(obj);
}







/* Test Case: Inheritance */
#undef CLASS
#define CLASS BaseClass
#define CLASS_BaseClass(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) \
    Data(int, base_value) \
    Method(int, get_base_value) \
    Method(int, get_overridable_value) \
    Method(int, get_incremental_value)

CONSTRUCTOR(int base_initial)
    self->base_value = base_initial;
END_CONSTRUCTOR

DESTRUCTOR()
END_DESTRUCTOR

METHOD(int, get_base_value)
    return self->base_value;
END_METHOD

METHOD(int, get_overridable_value)
    return 1;
END_METHOD

METHOD(int, get_incremental_value)
    return 1;
END_METHOD


#undef CLASS
#define CLASS DerivedClass
#define CLASS_DerivedClass(Base, Interface, Data, Event, Method, Override) \
    Base(BaseClass) \
    Data(int, derived_value) \
    Method(int, get_derived_value) \
    Override(int, get_overridable_value) \
    Override(int, get_incremental_value)

CONSTRUCTOR(int base_initial, int derived_initial)
    INIT_BASE(base_initial);
    self->derived_value = derived_initial;
END_CONSTRUCTOR

DESTRUCTOR()
END_DESTRUCTOR

METHOD(int, get_derived_value)
    return self->derived_value;
END_METHOD

METHOD(int, get_overridable_value)
    return 2;
END_METHOD

METHOD(int, get_incremental_value)
    return BASE_METHOD(get_incremental_value) + 2;
END_METHOD

void test_Inheritance(void) {
    AUTODESTROY_PTR(DerivedClass) *obj = NEW_ALLOC(DerivedClass, 100, 200);
    BaseClass *base_obj = (BaseClass *)obj;
    TEST_ASSERT_NOT_NULL(obj);
    TEST_ASSERT_EQUAL_INT(100, obj->get_base_value(obj));
    TEST_ASSERT_EQUAL_INT(200, obj->get_derived_value(obj));
    TEST_ASSERT_EQUAL_INT(2, obj->get_overridable_value(obj));
    TEST_ASSERT_EQUAL_INT(2, base_obj->get_overridable_value(base_obj));
    TEST_ASSERT_EQUAL_INT(3, obj->get_incremental_value(obj));
    TEST_ASSERT_EQUAL_INT(3, base_obj->get_incremental_value(base_obj));
    DESTROY_FREE(obj);
}







/* Test Case: Polymorphism */
#define I_Printable(Data, Event, Method) \
    Method(void, print)
CREATE_INTERFACE(Printable)

#undef CLASS
#define CLASS BasePrintable
#define CLASS_BasePrintable(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) \
    Interface(Printable) \
    Method(void, print) \
    Data(int, base_num)

CONSTRUCTOR(int num)
    self->base_num = num;
END_CONSTRUCTOR

DESTRUCTOR()
END_DESTRUCTOR

METHOD(void, print)
    printf("BasePrintable number: %d\n", self->base_num);
END_METHOD

#undef CLASS
#define CLASS DerivedPrintable
#define CLASS_DerivedPrintable(Base, Interface, Data, Event, Method, Override) \
    Base(BasePrintable) \
    Data(int, derived_num)

CONSTRUCTOR(int base_num, int derived_num)
    INIT_BASE(base_num);
    self->derived_num = derived_num;
END_CONSTRUCTOR

DESTRUCTOR()
END_DESTRUCTOR

METHOD(void, print)
    printf("DerivedPrintable numbers: %d and %d\n", self->base_num, self->derived_num);
END_METHOD

void test_Polymorphism(void) {
    AUTODESTROY_PTR(BasePrintable) *base_obj = NEW_ALLOC(BasePrintable, 50);
    TEST_ASSERT_NOT_NULL(base_obj);

    AUTODESTROY_PTR(DerivedPrintable) *derived_obj = NEW_ALLOC(DerivedPrintable, 100, 150);
    TEST_ASSERT_NOT_NULL(derived_obj);

    Printable base_printable = base_obj->to_Printable(base_obj);
    Printable derived_printable = derived_obj->to_Printable(derived_obj);

    TEST_ASSERT_NOT_NULL(base_printable.print);
    TEST_ASSERT_NOT_NULL(derived_printable.print);

    DESTROY_FREE(base_obj);
    DESTROY_FREE(derived_obj);
}







/* Test Case: Events */
#undef CLASS
#define CLASS EventClass
#define CLASS_EventClass(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT) \
    Event(on_event_triggered)

CONSTRUCTOR()
END_CONSTRUCTOR

DESTRUCTOR()
END_DESTRUCTOR

static int event_triggered = 0;
EVENT_HANDLER(EventClass, on_event_triggered, handler1)
    event_triggered = 1;
END_EVENT_HANDLER

void test_Events(void) {
    event_triggered = 0;

    AUTODESTROY_PTR(EventClass) *obj = NEW_ALLOC(EventClass);
    TEST_ASSERT_NOT_NULL(obj);

    REGISTER_EVENT(EventClass, on_event_triggered, handler1, obj);

    RAISE_EVENT(obj, on_event_triggered);

    TEST_ASSERT_TRUE(event_triggered);

    DESTROY_FREE(obj);
}




/* Test Case: Auto-destruction and manual destruction of stack and heap allocated objects */
#undef CLASS
#define CLASS AutoDestruct
#define CLASS_AutoDestruct(Base, Interface, Data, Event, Method, Override) \
    Base(OBJECT)

CONSTRUCTOR()
END_CONSTRUCTOR

static int auto_destruct_calls = 0;
DESTRUCTOR()
    auto_destruct_calls++;
END_DESTRUCTOR

void CreateObjectsAndLeave(void) {
    
    AUTODESTROY_PTR(AutoDestruct) *obj1 = NEW_ALLOC(AutoDestruct);
    AUTODESTROY_PTR(AutoDestruct) *obj2 = NEW_ALLOC(AutoDestruct);
    AUTODESTROY_PTR(AutoDestruct) *obj3 = NEW_ALLOC(AutoDestruct);
    
    
    
    AUTODESTROY(AutoDestruct) obj4; NEW_INPLACE(AutoDestruct, &obj4);
    AUTODESTROY(AutoDestruct) obj5; NEW_INPLACE(AutoDestruct, &obj5);
    AUTODESTROY(AutoDestruct) obj6; NEW_INPLACE(AutoDestruct, &obj6);
}

void test_AutoDestructionManualDestruct(void) {
    auto_destruct_calls = 0;

    AUTODESTROY_PTR(AutoDestruct) *obj = NEW_ALLOC(AutoDestruct);
    AUTODESTROY(AutoDestruct) obj2; NEW_INPLACE(AutoDestruct, &obj2);
    DESTROY_FREE(obj);
    DESTROY(obj2);


    CreateObjectsAndLeave();
    TEST_ASSERT_EQUAL_INT(8, auto_destruct_calls);

}





/* ==========================
   Unity Setup
   ========================== */

void setUp(void) {}
void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_ClassCreation);
    RUN_TEST(test_ClassMethods);
    RUN_TEST(test_Inheritance);
    RUN_TEST(test_Polymorphism);
    RUN_TEST(test_Events);
    RUN_TEST(test_AutoDestructionManualDestruct);

    return UNITY_END();
}
