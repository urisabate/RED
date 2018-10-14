#include "j1App.h"
#include "j1Collision.h"
#include "j1Input.h"
#include "p2Point.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Map.h"
#include "p2Log.h"
#include "j1Player.h"


j1Player::j1Player()
{
	name.create("player");
}


j1Player::~j1Player()
{

}

bool j1Player::Awake(pugi::xml_node & config)
{
	pugi::xml_node player_node = config;

	//LVL 1 INITIAL POSITION
	init_pos1.x = player_node.child("lvl1").attribute("x").as_float();
	init_pos1.y = player_node.child("lvl1").attribute("y").as_float();

	////PLAYER RECT DIMENSIONS
	player_rect.w = player_node.child("rect").attribute("width").as_uint();
	player_rect.h = player_node.child("rect").attribute("height").as_uint();	
	

	//SCROLL AND JUMPSPEED (CONST)
	speed.x = player_node.child("speed").attribute("scrollspeed").as_float();
	speed.y = player_node.child("speed").attribute("jumpspeed").as_float();
	def_anim_speed = player_node.child("speed").attribute("defaultAnimationSpeed").as_float();

	gravity = player_node.child("gravity").attribute("value").as_float();

	// Parsing animations ----------------
	pugi::xml_node textureAtlas = player_node.child("TextureAtlas");
	texture_path = (textureAtlas.attribute("imagePath").as_string());

	SDL_Rect r;
	float node_speed = -1;

	// IDLE
	pugi::xml_node n = textureAtlas.child("idle");
	for (n; n; n = n.next_sibling("idle")) {
		r.x = n.attribute("x").as_int();
		r.y = n.attribute("y").as_int();
		r.w = n.attribute("width").as_int();
		r.h = n.attribute("height").as_int();
		idle.PushBack(r);
	}
	node_speed = n.attribute("speed").as_float();
	idle.speed = (node_speed <= 0) ? def_anim_speed : node_speed;

	// WALK
	n = textureAtlas.child("walk");
	for (n; n; n = n.next_sibling("walk")) {
		r.x = n.attribute("x").as_int();
		r.y = n.attribute("y").as_int();
		r.w = n.attribute("width").as_int();
		r.h = n.attribute("height").as_int();
		walk.PushBack(r);
	}
	node_speed = n.attribute("speed").as_float();
	walk.speed = (node_speed <= 0) ? def_anim_speed : node_speed;

	// JUMP
	n = textureAtlas.child("jump");
	for (n; n; n = n.next_sibling("jump")) {
		r.x = n.attribute("x").as_int();
		r.y = n.attribute("y").as_int();
		r.w = n.attribute("width").as_int();
		r.h = n.attribute("height").as_int();
		jump.PushBack(r);
	}
	node_speed = n.attribute("speed").as_float();
	jump.speed = (node_speed <= 0) ? def_anim_speed : node_speed;
	// End parsing animations -----------------

	//LOG("%d  %d", player_rect.h, player_rect.w);
	LOG("%d  %d", speed.x, speed.y);
	//PL. COLLIDER
	player_collider = App->collision->AddCollider(player_rect, COLLIDER_PLAYER, this);
	return true;
}

bool j1Player::Start()
{
	graphics = App->tex->Load(texture_path.GetString());

	//PLACING PLAYER AT INITIAL POS
	position.x = App->map->start_collider->rect.x;
	position.y = App->map->start_collider->rect.y;

	current_animation = &idle;

	max_speed_y = speed.y;
	level_finished = false;
	on_floor = false;
	is_jumping = false;

	r = 255;
	g = 0;
	b = 0;
	
	return true;
}

bool j1Player::Update(float dt)
{
	if (level_finished) App->NextLevel();
	if (!godmode)
	{
		Move();
	}
	else 
	{
		MoveFree();
	}

	if (player_collider != nullptr) {
		player_collider->rect.w = current_animation->GetCurrentFrame().w;
		player_collider->rect.h = current_animation->GetCurrentFrame().h;
		player_collider->SetPos(position.x, position.y);
	}
		Draw();
	return true;
}

bool j1Player::PostUpdate()
{
	App->render->MoveCamera(-dx, -dy);
	return true;
}

bool j1Player::CleanUp()
{
	App->tex->UnLoad(graphics);
	if (player_collider) player_collider->to_delete = true;
	return true;
}

void j1Player::Draw()
{
	App->render->Blit(graphics, position.x, position.y, &idle.GetCurrentFrame());
	g = 0;

}

////////////////////////////////////////////////////////////////////////////////////////////
void j1Player::Move()
{
	if (!have_collided && on_floor) on_floor = false;
	if (!have_collided && on_wall) on_wall = false;
	dx = 0;
	dy = 0;
	
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) 
	{
		dx += speed.x;
	}

	
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		dx -= speed.x;
	}

	/*if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		if (!on_floor)
		{
			position.y += (speed.y + 2);
			speed.y += gravity;
		}
	}*/


	if (is_jumping)
	{
		is_jumping = Jump();
		//LOG("JUMPING");
	}
	else if (!on_floor)
	{
		dy += gravity;
		//on_floor = false;
	}
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN && on_floor)
	{
		Jump();
		LOG("JUMP");

	}

	position.x += dx;
	position.y += dy;

	player_rect.x = position.x;
	player_rect.y = position.y;

	//player_collider->SetPos(position.x, position.y);
	//shade_collider->SetPos(position.x, position.y+ player_collider->rect.h);

	//have_collided = false;


//	/if (is_jumping && !move_left && !move_right)
//	/{
//	/	position.y -= aux_speed_y;
//	/	aux_speed_y -= 0.2;
//	/	if (aux_speed_y == 0)
//	/	{
//	/		on_top = true;
//	/		position.y += aux_speed_y;
//	/		aux_speed_y += 0.2;
//	/		if (aux_speed_y == speed.y)
//	/		{
//	/			position.y += aux_speed_y;
//	/		}
//  /
//	/	}
//  /
//	/}
//	/if (is_jumping && move_left && !move_right)
//	/{
//	/	position.x -= speed.x;
//	/	position.y -= aux_speed_y;
//	/	aux_speed_y -= 0.2;
//	/	if (aux_speed_y == 0)
//	/	{
//	/		on_top = true;
//	/		position.y += aux_speed_y;
//	/		aux_speed_y += 0.1;
//	/		if (aux_speed_y == speed.y)
//	/		{
//	/			position.y += aux_speed_y;
//	/		}
//  /
//	/	}
//	/}
//	/if (is_jumping && !move_left && move_right)
//	/{
//	/	position.x -= speed.x;
//	/	position.y -= aux_speed_y;
//	/	aux_speed_y -= 0.1;
//	/	if (aux_speed_y == 0)
//	/	{
//	/		on_top = true;
//	/		position.y += aux_speed_y;
//	/		aux_speed_y += 0.1;
//	/		if (aux_speed_y == speed.y)
//	/		{
//	/			position.y += aux_speed_y;
//	/		}
//  /
//	/	}
//	/}
//	/if (is_jumping && move_left && move_right)
//	/{
//	/	position.y -= aux_speed_y;
//	/	aux_speed_y -= 0.1;
//	/	if (aux_speed_y == 0)
//	/	{
//	/		on_top = true;
//	/		position.y += aux_speed_y;
//	/		aux_speed_y += 0.1;
//	/	}
//	/	if (aux_speed_y == speed.y)
//	/	{
//	/		position.y += aux_speed_y;
//	/	}
//  /
//  /
//	///if (!is_jumping && !on_floor)
//	///{
//	///	on_top = true;
//	///	position.y += aux_speed_y;
//	///	aux_speed_y += 0.1;
//	///	if (aux_speed_y == speed.y)
//	///	{
//	///		position.y += aux_speed_y;
//	///	}
//	///}
//  //
}

bool j1Player::Jump()
{
	if (!is_jumping)
	{
		is_jumping = true;
		jumpspeed = 5;
		on_floor = false;
		return true;
	}
	else {
		dy -= jumpspeed;
		jumpspeed -= 0.2;

	}
	return (jumpspeed>=0);
}


void j1Player::MoveFree()
{

	dx = 0;
	dy = 0;

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		dx += speed.x;
	}

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		dx -= speed.x;
		
	}

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		dy += speed.y;
	}

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		dy -= speed.y;
	}

	position.x += dx;
	position.y += dy;

	player_rect.x = position.x;
	player_rect.y = position.y;

	have_collided = false;

}


void j1Player::OnCollision(Collider * c1, Collider * c2)
{
	have_collided = true;
	if (c1->type == COLLIDER_PLAYER && c2->type == COLLIDER_FLOOR && !on_floor) {

		on_floor = true;
		//last_collision = COLLIDER_FLOOR;
	}
	if (c1->type == COLLIDER_PLAYER && c2->type == COLLIDER_WALL) {

		on_wall = true;
	}
	if (c1->type == COLLIDER_PLAYER && c2->type == COLLIDER_END && !level_finished) {
		level_finished = true;
	}

}

void j1Player::OnCollisionLine(Collider * c, int x1, int y1, int x2, int y2)
{
	LOG("COLLISION LINE");
	g = 255;
}

bool j1Player::Load(pugi::xml_node & node)
{
	LOG("Loading PLAYER");

	position.x = node.child("player").child("position").attribute("x").as_int();
	position.y = node.child("player").child("position").attribute("y").as_int();

	return true;
}


bool j1Player::Save(pugi::xml_node & node)
{
	LOG("Saving PLAYER");

	pugi::xml_node pl_node = node.append_child("position");

	pl_node.append_attribute("x") = position.x;
	pl_node.append_attribute("y") = position.y;

	//pl_node.append_attribute("x") = init_pos1.x;
	//pl_node.append_attribute("y") = init_pos1.y;

	LOG("playerX: %d - %d \n playerY: %d - %d",position.x, pl_node.attribute("x").as_int(), position.y, pl_node.attribute("y").as_int());

	return true;
}