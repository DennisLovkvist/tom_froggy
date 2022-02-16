#include <box2d/box2d.h>
#include <vector>
#include <algorithm>


class my_contact_listener : public b2ContactListener {

public:
    
    my_contact_listener();
    
    virtual void BeginContact(b2Contact* contact);
    virtual void EndContact(b2Contact* contact);
};