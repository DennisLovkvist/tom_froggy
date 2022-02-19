/*
 * Author: Dennis Lövkvist
 * Version: 1.0 (2021-10-18)
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFML/Audio.hpp>
#include <stdio.h>
#include <box2d/box2d.h>
#include <random>
#include <string>
#include "common.h"
#include "my_contact_listener.h"
#include <time.h> 
#include <math.h>


const int PLATFORMS = 36;
const int ROPE_NODES = 15;

b2Body* InitPlatform(b2World &world,UserData* user_data_ptr)
{
	b2BodyDef definition_body;
	definition_body.position.Set(0, 0);
	b2Body* body = world.CreateBody(&definition_body);
	b2PolygonShape box;
	box.SetAsBox(5.0f, 1.0f);
	
	b2FixtureDef definition_fixture;
	definition_fixture.shape = &box;	
	definition_fixture.filter.categoryBits = 1;
	definition_fixture.filter.maskBits = 2;
	body->CreateFixture(&definition_fixture);


	b2PolygonShape sensor;
	sensor.SetAsBox(4.4f, 0.8f);
	sensor.m_vertices[0].y -= 0.5f;
	sensor.m_vertices[1].y -= 0.5f;
	sensor.m_vertices[2].y -= 0.5f;
	sensor.m_vertices[3].y -= 0.5f;
	definition_fixture;
	definition_fixture.shape = &sensor;
	definition_fixture.isSensor = true;
	definition_fixture.density = 0;
	definition_fixture.userData.pointer = reinterpret_cast<uintptr_t>(user_data_ptr);
	
	body->CreateFixture(&definition_fixture);

	return body;
}
b2Body* InitPlayer(b2World &world,UserData* user_data_ptr)
{
	b2BodyDef definition_body;
	definition_body.type = b2_dynamicBody;
	definition_body.position.Set(3.9f, -1.0f);	
	definition_body.angularVelocity = 1.0;
	b2Body* body = world.CreateBody(&definition_body);

	b2PolygonShape dynamic_box;
	dynamic_box.SetAsBox(1.0f, 1.0f);
	b2FixtureDef definition_fixture;
	definition_fixture.shape = &dynamic_box;
	definition_fixture.density = 0.00001f;
	definition_fixture.friction = 0.6f;	
	definition_fixture.filter.categoryBits = 2;
	definition_fixture.filter.maskBits = 1;
	body->CreateFixture(&definition_fixture);

	b2PolygonShape sensor;
	sensor.SetAsBox(1.5f, 1.5f);
	definition_fixture;
	definition_fixture.shape = &sensor;
	definition_fixture.isSensor = true;
	definition_fixture.density = 0;
	definition_fixture.userData.pointer = reinterpret_cast<uintptr_t>(user_data_ptr);

	body->CreateFixture(&definition_fixture);
	return body;
}
void DisableRope(b2Body *rope[ROPE_NODES], bool flag)
{
	for (size_t i = 0; i < ROPE_NODES; i++)
	{
		rope[i]->SetEnabled(flag);
	}
	
}
int main()
{

	sf::RenderWindow window(sf::VideoMode(1280,720), "SFML works!", sf::Style::Close);

	sf::View view = window.getDefaultView();
	view.move(-1280/2,-720/2);
	sf::Clock clock;
	float dt = 1.f / 60.f;
	float accumulator = 0.f;
	float t = 0;

	b2Vec2 gravity(0.0f, 150.0f);
	b2World world(gravity);

	b2Body* platforms[PLATFORMS];

	UserData* platform_user_data = new UserData();
	platform_user_data->name = "platform";

	for (size_t i = 0; i < PLATFORMS ; i++)
	{
		platforms[i] = InitPlatform(world,platform_user_data);
	}

	srand (time(NULL));
	int spawn_y = view.getCenter().y + 30;
	int n = 0;
	for (size_t i = 1; i < PLATFORMS ; i++)
	{
		int x = 1 + rand() % (( 160 + 1 ) - 1)-80;
		int y = spawn_y;
		platforms[i]->SetTransform(b2Vec2(x,y),platforms[i]->GetAngle());

		if(n > 2)
		{
			spawn_y -= 10;
			n = 0;
		}
		else
		{
			n++;
		}
		
	}

	platforms[0]->SetTransform(b2Vec2(view.getCenter().x,view.getCenter().y + 20),platforms[0]->GetAngle());
	
	
	

	UserData* player_user_data = new UserData();
	player_user_data->name = "player";

	b2Body* player = InitPlayer(world,player_user_data);
	player->SetTransform(b2Vec2(view.getCenter().x,	view.getCenter().y + 5),player->GetAngle());

	b2Body* rope[ROPE_NODES];
	b2DistanceJoint* joints[ROPE_NODES+1];

	UserData* rope_user_data = new UserData();
	rope_user_data->name = "rope";
	rope_user_data->connected_platform = platforms[0];
	rope_user_data->connect = false;

	UserData* rope_end_user_data = new UserData();
	rope_end_user_data->name = "rope_end";
	rope_end_user_data->connected_platform = platforms[0];
	rope_end_user_data->connect = false;
	



  


	int y = 0;
	for (size_t i = 0; i < ROPE_NODES; i++)
	{
		b2BodyDef definition_body;
		definition_body.type = b2_dynamicBody;
		definition_body.position.Set(1.0f, view.getCenter().y  + y++);	
		definition_body.angularVelocity = 1.0;
		rope[i] = world.CreateBody(&definition_body);

		b2CircleShape circle;
		circle.m_p.Set(0.0f, 0.0f);
		circle.m_radius = (i == ROPE_NODES-1) ? 1.0 : 0.5;
		b2FixtureDef definition_fixture;
		definition_fixture.filter.groupIndex = 3;
		definition_fixture.shape = &circle;
		definition_fixture.density = (i == ROPE_NODES-1) ? 0.000001f : 0.00001f;
		definition_fixture.friction = 0.6f;
		definition_fixture.filter.categoryBits = 3;
		definition_fixture.filter.maskBits = 1;
		definition_fixture.isSensor = (i == ROPE_NODES-1);
		definition_fixture.userData.pointer = (i == ROPE_NODES-1) ? reinterpret_cast<uintptr_t>(rope_end_user_data) : reinterpret_cast<uintptr_t>(rope_user_data);
		rope[i]->CreateFixture(&definition_fixture);

		if(i != 0)
		{
			
			b2DistanceJointDef jd;
			jd.bodyA = rope[i-1];
			jd.bodyB = rope[i];
			jd.collideConnected = false;
			jd.length = 0.5f;
			jd.maxLength = 1.0f;
			joints[i] = (b2DistanceJoint*)world.CreateJoint(&jd);					
		}
	}

	b2DistanceJointDef jd;
	jd.bodyA = player;
	jd.bodyB = rope[0];
	jd.collideConnected = false;
	jd.length = 1.0f;
	jd.maxLength = 1.0f;
	joints[0] = (b2DistanceJoint*)world.CreateJoint(&jd);









	DisableRope(rope,false);



	my_contact_listener *contact_listener;
	contact_listener = new my_contact_listener();
	world.SetContactListener(contact_listener);	


	world.SetAllowSleeping(false);

	bool player_walk_left = false;
	bool player_walk_right = false;

	bool player_jump = false;
	bool double_jumping_allowed = false;
	bool jumping_allowed = false;

	bool rope_key_down = false;
	bool cast_rope = false;
	bool cut_rope = false;


	bool jump_key_down;
	bool jump_key_down_prev;
	bool joint_exists = false;

	sf::Vertex lines[8];



	view.zoom(0.1f);


	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			if(event.type == sf::Event::KeyPressed)
			{
                if(event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
				if(event.key.code == sf::Keyboard::W && !jump_key_down_prev)
                {
					jump_key_down = true;
					player_jump = true;
                }
				if(event.key.code == sf::Keyboard::A)
                {
					player_walk_left = true;
                }
				if(event.key.code == sf::Keyboard::D)
                {
					player_walk_right = true;
                }
				if(event.key.code == sf::Keyboard::Space && !rope_key_down)
                {
					rope_key_down = true;
					cast_rope = true;
                }
			}
			if(event.type == sf::Event::KeyReleased)
			{
				if(event.key.code == sf::Keyboard::A)
                {
					player_walk_left = false;
                }
				if(event.key.code == sf::Keyboard::D)
                {
					player_walk_right = false;
                }
				if(event.key.code == sf::Keyboard::W)
                {
					jump_key_down = false;
                }
				if(event.key.code == sf::Keyboard::Space && rope_key_down)
                {
					cut_rope = true;
					rope_key_down = false;
                }
			}

			jump_key_down_prev = jump_key_down;
		}
		while (accumulator >= dt)
		{
			window.clear(sf::Color(40,40,40));
			float time_step = 1.0f / 60.0f;
			int32 velocity_iterations = 6;
			int32 position_iterations = 2;


			world.Step(time_step, velocity_iterations, position_iterations);

			//view.move(0,-0.09f);


			for (size_t i = 0; i < PLATFORMS ; i++)
			{


				float dx = 1;
				float dy = platforms[i]->GetPosition().y - view.getCenter().y;
				int dist = std::sqrt(dy*dy+dx*dx);

			
				if(dist > 40)
				{
					platforms[i]->SetTransform(b2Vec2(1 + rand() % (( 160 + 1 ) - 1)-80,view.getCenter().y - 40),platforms[i]->GetAngle());
				}
				
				
			}

			float dx = 1;
			float dy = player->GetPosition().y - view.getCenter().y;
			int dist = std::sqrt(dy*dy+dx*dx);


		
			if(dist > 40)
			{
				player->SetTransform(b2Vec2(1 + rand() % (( 160 + 1 ) - 1)-80,view.getCenter().y - 40),player->GetAngle());
			}



				

				UserData* ud = (UserData*)(player->GetFixtureList()[0].GetUserData().pointer);

				jumping_allowed = ud->collsisions > 0;

				if(player_walk_left)
				{
					player->ApplyForce(b2Vec2(-0.005f,0),player->GetPosition(),true);
				}
				if(player_walk_right)
				{
					player->ApplyForce(b2Vec2(0.005f,0),player->GetPosition(),true);
				}
				if(player_jump && jumping_allowed)
				{					
					player_jump = false;
					double_jumping_allowed = true;
                    player->ApplyForce(b2Vec2(0,-0.15f),player->GetPosition(),true);
				}
				if(player_jump && double_jumping_allowed)
				{
					double_jumping_allowed = false;
					player_jump = false;
                    player->ApplyForce(b2Vec2(0,-0.12f),player->GetPosition(),true);
				}

				if(rope[ROPE_NODES-1]->IsEnabled())
				{

					ud = (UserData*)(rope[ROPE_NODES-1]->GetFixtureList()[0].GetUserData().pointer);
					
					

					if(ud->connect)
					{
						if(ud->connected_platform != nullptr)
						{
							if(!joint_exists)
							{
								joint_exists = true;
								ud->connect = false;
								b2DistanceJointDef jd;
								jd.bodyA = ud->connected_platform;
								jd.bodyB = rope[ROPE_NODES-1];
								ud->connected_platform = nullptr;
								jd.collideConnected = false;
								jd.length = 1.0f;
								jd.maxLength = 1.0f;
								joints[ROPE_NODES] = (b2DistanceJoint*)world.CreateJoint(&jd);
							}
						}
					}
				}

				if(cast_rope)
				{
					cast_rope = false;		

					for (int i = ROPE_NODES-1; i >=0; i--)
					{
						rope[i]->SetTransform(player->GetPosition()-b2Vec2(0,i/2),0);
						rope[i]->ApplyForce(b2Vec2(0,-i/8),rope[i]->GetPosition(),true);
						
					}
								
					DisableRope(rope,true);
						

				}

				if(cut_rope)
				{
					cut_rope = false;
					if(joint_exists)
					{	
						world.DestroyJoint(joints[ROPE_NODES]);
						joint_exists = false;
					}
					DisableRope(rope,false);
				}
				

			b2Body * B = world.GetBodyList();
			while(B != NULL)
			{
				b2Fixture* F = B->GetFixtureList();             
				while(F != NULL)
				{
					switch (F->GetType())
					{
						case b2Shape::e_polygon:
						{


							b2PolygonShape* poly = (b2PolygonShape*) F->GetShape();

							float mul = 1;

							lines[0].position.x = B->GetWorldPoint(poly->m_vertices[0]).x*mul;	
							lines[0].position.y = B->GetWorldPoint(poly->m_vertices[0]).y*mul;

							lines[1].position.x = B->GetWorldPoint(poly->m_vertices[1]).x*mul;
							lines[1].position.y = B->GetWorldPoint(poly->m_vertices[1]).y*mul;

							
							lines[2].position.x = B->GetWorldPoint(poly->m_vertices[1]).x*mul;
							lines[2].position.y = B->GetWorldPoint(poly->m_vertices[1]).y*mul;							
							lines[3].position.x = B->GetWorldPoint(poly->m_vertices[2]).x*mul;
							lines[3].position.y = B->GetWorldPoint(poly->m_vertices[2]).y*mul;

													
							lines[4].position.x = B->GetWorldPoint(poly->m_vertices[2]).x*mul;
							lines[4].position.y = B->GetWorldPoint(poly->m_vertices[2]).y*mul;							
							lines[5].position.x = B->GetWorldPoint(poly->m_vertices[3]).x*mul;
							lines[5].position.y = B->GetWorldPoint(poly->m_vertices[3]).y*mul;

							
							lines[6].position.x = B->GetWorldPoint(poly->m_vertices[3]).x*mul;
							lines[6].position.y = B->GetWorldPoint(poly->m_vertices[3]).y*mul;							
							lines[7].position.x = B->GetWorldPoint(poly->m_vertices[0]).x*mul;
							lines[7].position.y = B->GetWorldPoint(poly->m_vertices[0]).y*mul;



							window.draw(lines, 8, sf::Lines);
						}
						break;
						case b2Shape::e_circle:
						{


							b2PolygonShape* poly = (b2PolygonShape*) F->GetShape();

							float mul = 1;

							sf::Vertex lines[16];

							float r = 0;
							float inc = (3.143*2)/8;
							float radius = F->GetShape()->m_radius;
							for (size_t i = 0; i < 15; i+=2)
							{
								lines[i].position.x = B->GetPosition().x + std::cos(r)*radius;
								lines[i].position.y = B->GetPosition().y + std::sin(r)*radius;
								r+=inc;
								lines[i+1].position.x = B->GetPosition().x + std::cos(r)*radius;
								lines[i+1].position.y = B->GetPosition().y + std::sin(r)*radius;
							}
							


							window.draw(lines, 16, sf::Lines);
						}
						break;
					}
					F = F->GetNext();
				}

				B = B->GetNext();
			}  

			window.setView(view);
			window.display();
			accumulator -= dt;
		};
		accumulator += clock.getElapsedTime().asSeconds();

		clock.restart();		
	}

	return 0;
}
