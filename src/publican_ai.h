#pragma once

#define DRINK_TIME 10000000L
#define SERVE_TIME 1000000L
#define TIOLET_TIME 8000000L
#define CHAT_TIME 1000000L

#define LOITER_TIME 500000L

enum body_type {
	body_thin,
	body_med,
	body_fat	
};

enum dress_type {
	dress_1,	
	dress_2,	
	dress_3,	
};

enum hair_type {
	hair_1,	
	hair_2,	
	hair_3,	
	hair_4,	
	hair_5,	
	hair_6,	
	hair_7,	
	hair_8,	
	hair_9,	
};

enum face_type {
	face_1,	
	face_2,	
	face_3,	
};

enum race_type {
	racedir_northull,
	race_human,
	race_elf,	
};

enum trait_type {
	traitdir_northull,
	trait_mediator,
	trait_troublemaker,	
};

enum job_type {
	jobdir_northull,
	job_owner,
	job_staff,
	job_blacksmith,
	job_builder,
	job_shopkeep,
	job_labourer,
	job_housewife,
	job_unemployed,
	job_militia,
	job_police,
	job_vagrant,
};

enum opinion_mod {
	opiniondir_northull,
	opinion_friend,
	opinion_enemy,
	opinion_fancies,
};

struct opinion {
	enum opinion_mod mod;
	uint32_t target;	
};

enum entity_action {
	action_sit,
	action_drink,
	act_move,		
	
	actdir_northull
};

enum entity_intent {
	// all intent
	intent_null,
	
	intent_tiolet,
	
	// punter intents
	intent_purchase,
	intent_drink,
	
	// staff intents
	intent_serve,		
	intent_nothing,		
};

enum tiolet_stages {
	tiolet_init,
	tiolet_walk,
	tiolet_defecate
};

struct intent_tiolet {
	enum tiolet_stages stage;	
};

enum drink_stages {
	drink_init,
	drink_walk,
	drink_drink,
};

struct intent_drink {
	enum drink_stages stage;	
};

enum serve_stages {
	serve_init,
	serve_walk,
	serve_wait,
	serve_sell,
};

struct intent_serve {
	enum serve_stages stage;	
};

enum purchase_stages {
	purchase_init,
	purchase_walk,
	purchase_wait,
	purchase_buy,
};

struct intent_purchase {
	enum purchase_stages stage;
	bool interaction; 	
};

enum nothing_stages {
	nothing_init,
	nothing_loiter,
	nothing_wander	
};

struct intent_nothing {
	enum nothing_stages stage;	
};

struct char_intent {
	enum entity_intent type;	
	union {
		struct intent_purchase purchase;
		struct intent_serve serve;
		struct intent_drink drink;
		struct intent_tiolet tiolet;
		struct intent_nothing nothing;
	};
};

extern inline struct char_intent SET_NULLINTENT(void)
{
	struct char_intent nullIntent = {};
	return(nullIntent);
}

struct chara_attributes {
	uint8_t strength;
	uint8_t charisma;
	uint8_t intelligence;
	uint8_t dexterity;
	uint8_t fortitude;
	uint8_t perception;
};

struct chara_stats {
	char firstName[32];
	char lastName[32];
	enum race_type race;
	enum job_type job;
	int32_t age;
	
	enum dress_type dress;
	enum hair_type hair;
	enum face_type face;
	enum body_type body;
	
	enum trait_type traits[32];
	
	bool isStaff;	
	bool gender;
	
	float drunk;
	float bladder;
	float thirst;
	float social;
	float fun;
	
	struct item inv[4];
	int32_t itemCount;
	struct item *useItem; 
	
	uint32_t money;
	struct chara_attributes attribs;
	
	struct social_relation *relations;
	int32_t relationCount;
};
