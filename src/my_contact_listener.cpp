#include "my_contact_listener.h"
#include "common.h"
#include <iostream>

my_contact_listener::my_contact_listener() 
{

}

void my_contact_listener::BeginContact(b2Contact* contact) 
{
    if(contact->GetFixtureA()->IsSensor() && contact->GetFixtureB()->IsSensor())
    {
        UserData* uda = (UserData*)(contact->GetFixtureA()->GetUserData().pointer);
        UserData* udb = (UserData*)(contact->GetFixtureB()->GetUserData().pointer);
        
        if(uda->name == "player")
        {
            if(udb->name == "platform")
            {
                uda->collsisions ++;
            }
        }
        else
        {
            if(udb->name == "player")
            {
                if(uda->name == "platform")
                {
                    udb->collsisions ++;
                }
            }
        }
    }
        UserData* uda = (UserData*)(contact->GetFixtureA()->GetUserData().pointer);
        UserData* udb = (UserData*)(contact->GetFixtureB()->GetUserData().pointer);    

        if(uda != nullptr && udb != nullptr)
        {

            if(uda->name == "rope_end")
            {                
                if(udb->name == "platform")
                {
                    uda->connected_platform = contact->GetFixtureB()->GetBody();
                    uda->connect = true;
                }
            }
            else if(udb->name == "rope_end")
            {            
                if(uda->name == "platform")
                {
                    udb->connected_platform = contact->GetFixtureA()->GetBody();
                    udb->connect = true;
                }                
            }
        }
    
}

void my_contact_listener::EndContact(b2Contact* contact) 
{
    if(contact->GetFixtureA()->IsSensor() && contact->GetFixtureB()->IsSensor())
    {
        UserData* uda = (UserData*)(contact->GetFixtureA()->GetUserData().pointer);
        UserData* udb = (UserData*)(contact->GetFixtureB()->GetUserData().pointer);
        
        if(uda->name == "player")
        {
            if(udb->name == "platform")
            {
                uda->collsisions--;
            }
        }
        else
        {
            if(udb->name == "player")
            {
                if(uda->name == "platform")
                {
                   udb->collsisions--;
                }
            }
        }
    }
}