#pragma once
#include "CoordinateSystem.h"
#include "World.h"
#include "ShadowBox.h"
#include "Player.h"




class Game
{

private:
	const std::string ROD_FILENAME = "assets/Models/Rod.obj";
	const std::string RING_FILENAME = "assets/Models/Ring.obj";
	const std::string SKYBOX_FILENAME = "assets/Models/Skybox.obj";
	const std::string WORLD_FOLDER = "assets/Worlds/";

	//Game object models for drawing
	ModelWithShader skybox_model;
	ModelWithShader rod_model;
	ModelWithShader ring_model;

	//Game Objects
	World world;
	Player player;
	PickupManager pickup_manager;

	//Camerars for behind the player and overview
	CoordinateSystem player_camera;
	CoordinateSystem overview_camera;
	CoordinateSystem* active_camera = &player_camera;

	//For Rotating around player with the camera
	bool camera_float = false;
	
	//For Swapping between Levels
	//Current level
	unsigned int level = 0;
	//list of level file names to load
	std::vector<std::string> levels;

	//Used for Lighting and Shadows
	ShadowBox shadow_box;
	glm::mat4x4 light_view_matrix;

public:
	//Empty Constructor, Init must be used to initialize game
	Game() = default;

	//Initialize The models, player,world, pickup manager, shadow box
	//lighting and camera.
	void init();

	//Update the game based on inputs using fixed delta time
	//Updates player and pickups
	void update(double fixed_delta_time);

	//Updates animations for the player using variable delta time
	void updateAnimations(double delta_time);

	void display();

	//Destory the current world and creates a new one from the list of levels
	void destroyIntoNextWorld();
	
private:
	void renderToDepthTexture(glm::mat4& depth_vp);
	//Player functions
	void playerAccelerateForward(float delta_time);
	void playerAccelerateBackward(float delta_time);
	void playerAccelerateLeft(float delta_time);
	void playerAccelerateRight(float delta_time);
	void playerTurnLeft(float delta_time);
	void playerTurnRight(float delta_time);
	void playerJump(float delta_time);




};






