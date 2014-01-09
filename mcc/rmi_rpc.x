#ifdef RPC_XDR
%#include "rmi_rpc.h"
#endif

enum RMI1_type {
     RMI1_TRUE = 0,
     RMI1_FALSE = 1,
     RMI1_NUMBER = 2,
     RMI1_STRING = 3,
     RMI1_ARRAY = 4,
     RMI1_OBJECT = 5
};

typedef struct RMI1_data *RMI1_value;

typedef struct RMI1_field RMI1_object<>;

union RMI1_data switch (RMI1_type type) {
case RMI1_TRUE:
    void;
case RMI1_FALSE:
    void;
case RMI1_NUMBER:
    string _number<>;
case RMI1_STRING:
    string _string <>;
case RMI1_OBJECT:
    RMI1_object _object;
case RMI1_ARRAY:
    RMI1_value _array<>;
};

struct RMI1_field {
    string name<>;
    RMI1_value value;
};

struct RMI1_CALLarg {
    string oid<>;
    string func<>;
    RMI1_value args<>;
};

union RMI1_CALLres switch (int error) {
case 0:
    RMI1_value result;
default:
    string detail<>;
};

struct RMI1_discovery {
    string oid<>;
    string type<>;
    u_long vers;
    string detail<>;
};

typedef RMI1_discovery RMI1_DISCOVERres<>;

program RMI_PROGRAM {
    version RMI_V1 {
        RMI1_CALLres RMI1_CALL(RMI1_CALLarg) = 1;
        RMI1_DISCOVERres RMI1_DISCOVER(void) = 2;
    } = 1;
} = 1;

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
