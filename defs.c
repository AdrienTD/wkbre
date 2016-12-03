char *CLASS_str[64] = {
"DECLARE_ITEM",
"DEFINE_VALUE",
"APPEARANCE_TAG",
"ANIMATION_TAG",
"USER",
"LEVEL",
"PLAYER",
"CITY",
"TOWN",
"BUILDING",
"CHARACTER",
"CONTAINER",
"FORMATION",
"MARKER",
"MISSILE",
"PROP",
"ARMY",
"TERRAIN_ZONE",
"COMMAND",
"ACTION_SEQUENCE",
"CHARACTER_LADDER",
"USER_EXTENSION",
"LEVEL_EXTENSION",
"PLAYER_EXTENSION",
"CITY_EXTENSION",
"TOWN_EXTENSION",
"BUILDING_EXTENSION",
"CHARACTER_EXTENSION",
"CONTAINER_EXTENSION",
"FORMATION_EXTENSION",
"MARKER_EXTENSION",
"MISSILE_EXTENSION",
"PROP_EXTENSION",
"ARMY_EXTENSION",
"DECLARE_ALIAS",
"ASSOCIATE_CATEGORY",
"DECLARE_ASSOCIATE_CATEGORY",
"ORDER",
"TASK",
"REACTION",
"DIPLOMATIC_LADDER",
"DEFAULT_DIPLOMATIC_STATUS",
"INDEV_EQUATION",
"INDEV_OBJECT_FINDER_DEFINITION",
"INDEV_ACTION_SEQUENCE",
"EQUATION",
"OBJECT_FINDER_DEFINITION",
"TYPE_TAG",
"OBJECT_CREATION",
"GAME_EVENT",
"PACKAGE_RECEIPT_TRIGGER",
"VALUE_TAG",
"DEFAULT_VALUE_TAG_INTERPRETATION",
"ORDER_ASSIGNMENT",
"ORDER_CATEGORY",
"TASK_CATEGORY",
"PACKAGE",
"GAME_TEXT_WINDOW",
"3D_CLIP",
"CAMERA_PATH",
"WORK_ORDER",
"COMMISSION",
"PLAN",
"CONDITION",
};
char *CCOMMAND_str[5] = {
"BUTTON_ENABLED",
"BUTTON_DEPRESSED",
"BUTTON_HIGHLIGHTED",
"BLUEPRINT_TOOLTIP",
"START_SEQUENCE",
};
char *CBLUEPRINT_str[18] = {
"ITEM",
"STARTS_WITH_ITEM",
"BLUEPRINT_TOOLTIP",
"HAS_DYNAMIC_SHADOW",
"HAS_STATIC_SHADOW",
"MAP_SOUND_TAG",
"OFFERS_COMMAND",
"PHYSICAL_SUBTYPE",
"SCALE_APPEARANCE",
"MOVEMENT_BAND",
"INHERITS_FROM",
"MAP_TYPE_TAG",
"INTRINSIC_REACTION",
"INTERPRET_VALUE_TAG_AS",
"REPRESENT_AS",
"BUILDING_TYPE",
"CAN_SPAWN",
"MOVEMENT_SPEED_EQUATION",
};
char *VALUE_str[45] = {
"CONSTANT",
"DEFINED_VALUE",
"ITEM_VALUE",
"NUM_OBJECTS",
"EQUATION_RESULT",
"OBJECT_TYPE",
"OBJECT_CLASS",
"OBJECT_ID",
"IS_SUBSET_OF",
"WATER_BENEATH",
"BLUEPRINT_ITEM_VALUE",
"DISTANCE_BETWEEN",
"DIPLOMATIC_STATUS_AT_LEAST",
"CURRENTLY_DOING_ORDER",
"TOTAL_ITEM_VALUE",
"HAS_APPEARANCE",
"CAN_REACH",
"SAME_PLAYER",
"ARE_ASSOCIATED",
"IS_IDLE",
"IS_DISABLED",
"IS_MUSIC_PLAYING",
"VALUE_TAG_INTERPRETATION",
"TILE_ITEM",
"NUM_REFERENCERS",
"IS_ACCESSIBLE",
"HAS_DIRECT_LINE_OF_SIGHT_TO",
"FINDER_RESULTS_COUNT",
"NUM_ASSOCIATES",
"NUM_ASSOCIATORS",
"INDEXED_ITEM_VALUE",
"ANGLE_BETWEEN",
"CURRENTLY_DOING_TASK",
"IS_VISIBLE",
"IS_DISCOVERED",
"WITHIN_FORWARD_ARC",
"BUILDING_TYPE",
"BUILDING_TYPE_OPERAND",
"AI_CONTROLLED",
"MAP_WIDTH",
"MAP_DEPTH",
"GRADIENT_IN_FRONT",
"CAN_AFFORD_COMMISSION",
"DISTANCE_BETWEEN_INCLUDING_RADIUS",
"COULD_REACH",
};
char *ENODE_str[27] = {
"ADDITION",
"SUBTRACTION",
"MULTIPLICATION",
"DIVISION",
"ABSOLUTE_VALUE",
"MAX",
"MIN",
"AND",
"OR",
"NOT",
"IS_ZERO",
"IS_POSITIVE",
"IS_NEGATIVE",
"EQUALS",
"LESS_THAN",
"LESS_THAN_OR_EQUAL_TO",
"GREATER_THAN",
"GREATER_THAN_OR_EQUAL_TO",
"RANDOM_UP_TO",
"RANDOM_INTEGER",
"RANDOM_RANGE",
"IF_THEN_ELSE",
"IS_BETWEEN",
"FRONT_BACK_LEFT_RIGHT",
"TRUNC",
"NEGATE",
"ROUND",
};
char *FINDER_str[38] = {
"SPECIFIC_ID",
"SELF",
"SEQUENCE_EXECUTOR",
"TARGET",
"PLAYER",
"CONTROLLER",
"CREATOR",
"CANDIDATE",
"ALIAS",
"ASSOCIATES",
"ASSOCIATORS",
"PLAYERS",
"LEVEL",
"PACKAGE_SENDER",
"TILE_RADIUS",
"METRE_RADIUS",
"NEAREST_TO_SATISFY",
"DISABLED_ASSOCIATES",
"GRADE_SELECT",
"GRADE_SELECT_CANDIDATES",
"REFERENCERS",
"ORDER_GIVER",
"CHAIN_ORIGINAL_SELF",
"PACKAGE_RELATED_PARTY",
"BEING_TRANSFERRED_TO_ME",
"DISCOVERED_UNITS",
"USER",
"AG_SELECTION",
"UNION",
"INTERSECTION",
"CHAIN",
"ALTERNATIVE",
"RESULTS",
"SUBORDINATES",
"FILTER",
"FILTER_FIRST",
"FILTER_CANDIDATES",
"NEAREST_CANDIDATE",
};
char *POSITION_str[15] = {
"CENTRE_OF_MAP",
"LOCATION_OF",
"OUT_AT_ANGLE",
"NEAREST_VALID_POSITION_FOR",
"NEAREST_ATTACHMENT_POINT",
"ABSOLUTE_POSITION",
"SPAWN_TILE_POSITION",
"THIS_SIDE_OF",
"THE_OTHER_SIDE_OF",
"AWAY_FROM",
"IN_FRONT_OF",
"FIRING_ATTACHMENT_POINT",
"NEAREST_VALID_STAMPDOWN_POS",
"OFFSET_FROM",
"DESTINATION_OF",
};
char *ACTION_str[110] = {
"REMOVE",
"CREATE_OBJECT",
"EXECUTE_SEQUENCE",
"REPEAT_SEQUENCE",
"SET_ITEM",
"INCREASE_ITEM",
"DECREASE_ITEM",
"UPON_CONDITION",
"CREATE_OBJECT_VIA",
"SEND_EVENT",
"CLEAR_ALIAS",
"ASSIGN_ALIAS",
"UNASSIGN_ALIAS",
"DISABLE",
"ENABLE",
"CLEAR_ASSOCIATES",
"REGISTER_ASSOCIATES",
"DEREGISTER_ASSOCIATES",
"EXECUTE_ONE_AT_RANDOM",
"TRACE_VALUE",
"TRACE_FINDER_RESULTS",
"TERMINATE_THIS_TASK",
"TRACE",
"ASSIGN_ORDER_VIA",
"TERMINATE_TASK",
"TERMINATE_ORDER",
"TERMINATE_THIS_ORDER",
"CANCEL_ORDER",
"TRANSFER_CONTROL",
"SWITCH_APPEARANCE",
"CONVERT_TO",
"CONVERT_ACCORDING_TO_TAG",
"EXECUTE_SEQUENCE_AFTER_DELAY",
"EXECUTE_SEQUENCE_OVER_PERIOD",
"REPEAT_SEQUENCE_OVER_PERIOD",
"ADD_REACTION",
"REMOVE_REACTION",
"CHANGE_REACTION_PROFILE",
"SET_SCALE",
"TERMINATE",
"SINK_AND_REMOVE",
"SEND_PACKAGE",
"SET_SELECTABLE",
"SET_TARGETABLE",
"SET_RENDERABLE",
"SWITCH_CONDITION",
"SWITCH_HIGHEST",
"DISPLAY_GAME_TEXT_WINDOW",
"HIDE_GAME_TEXT_WINDOW",
"HIDE_CURRENT_GAME_TEXT_WINDOW",
"PLAY_CLIP",
"SNAP_CAMERA_TO_POSITION",
"FACE_TOWARDS",
"IDENTIFY_AND_MARK_CLUSTERS",
"STORE_CAMERA_POSITION",
"SNAP_CAMERA_TO_STORED_POSITION",
"SET_INDEXED_ITEM",
"INCREASE_INDEXED_ITEM",
"DECREASE_INDEXED_ITEM",
"SET_RECONNAISSANCE",
"ENABLE_DIPLOMATIC_REPORT_WINDOW",
"DISABLE_DIPLOMATIC_REPORT_WINDOW",
"ENABLE_TRIBUTES_WINDOW",
"DISABLE_TRIBUTES_WINDOW",
"COPY_FACING_OF",
"TELEPORT",
"LOCK_TIME",
"UNLOCK_TIME",
"REMOVE_MULTIPLAYER_PLAYER",
"REEVALUATE_TASK_TARGET",
"CREATE_FORMATION",
"CREATE_FORMATION_REFERENCE",
"CHANGE_DIPLOMATIC_STATUS",
"DISBAND_FORMATION",
"LEAVE_FORMATION",
"STOP_SOUND",
"PLAY_SOUND",
"PLAY_SOUND_AT_POSITION",
"PLAY_SPECIAL_EFFECT",
"PLAY_SPECIAL_EFFECT_BETWEEN",
"PLAY_ANIMATION_IF_IDLE",
"ATTACH_SPECIAL_EFFECT",
"ATTACH_LOOPING_SPECIAL_EFFECT",
"DETACH_LOOPING_SPECIAL_EFFECT",
"REVEAL_FOG_OF_WAR",
"COLLAPSING_CIRCLE_ON_MINIMAP",
"SHOW_BLINKING_DOT_ON_MINIMAP",
"ENABLE_GAME_INTERFACE",
"DISABLE_GAME_INTERFACE",
"PLAY_CAMERA_PATH",
"STOP_CAMERA_PATH_PLAYBACK",
"INTERPOLATE_CAMERA_TO_POSITION",
"SET_ACTIVE_MISSION_OBJECTIVES",
"SHOW_MISSION_OBJECTIVES_ENTRY",
"SHOW_MISSION_OBJECTIVES_ENTRY_INACTIVE",
"HIDE_MISSION_OBJECTIVES_ENTRY",
"TRACK_OBJECT_POSITION_FROM_MISSION_OBJECTIVES_ENTRY",
"STOP_INDICATING_POSITION_OF_MISSION_OBJECTIVES_ENTRY",
"FORCE_PLAY_MUSIC",
"ADOPT_APPEARANCE_FOR",
"ADOPT_DEFAULT_APPEARANCE_FOR",
"ACTIVATE_COMMISSION",
"DEACTIVATE_COMMISSION",
"SWITCH_ON_INTENSITY_MAP",
"FADE_STOP_MUSIC",
"DECLINE_DIPLOMATIC_OFFER",
"SEND_CHAT_MESSAGE",
"MAKE_DIPLOMATIC_OFFER",
"SET_CHAT_PERSONALITY",
"UPDATE_USER_PROFILE",
};
char *OBJCREATE_str[6] = {
"TYPE_TO_CREATE",
"MAPPED_TYPE_TO_CREATE",
"MATCH_APPEARANCE_OF",
"CONTROLLER",
"CREATE_AT",
"POST_CREATION_SEQUENCE",
};
char *SAVEGAME_str[19] = {
"DELAYED_SEQUENCE_EXECUTION",
"EXECUTE_SEQUENCE_OVER_PERIOD",
"REPEAT_SEQUENCE_OVER_PERIOD",
"GAME_SET",
"NEXT_UNIQUE_ID",
"PREDEC",
"LEVEL",
"GAME_TYPE",
"PART_OF_CAMPAIGN",
"SERVER_NAME",
"NUM_HUMAN_PLAYERS",
"UPDATE_ID",
"NEXT_UPDATE_TIME_STAMP",
"CURRENT_TIME",
"PREVIOUS_TIME",
"ELAPSED_TIME",
"PAUSED",
"LOCK_COUNT",
"CLIENT_STATE",
};
char *GAMEOBJ_str[40] = {
"POSITION",
"ORIENTATION",
"ITEM",
"SUBTYPE",
"APPEARANCE",
"COLOUR_INDEX",
"ORDER_CONFIGURATION",
"SCALE",
"PLAYER",
"CITY",
"TOWN",
"BUILDING",
"CHARACTER",
"CONTAINER",
"FORMATION",
"MARKER",
"MISSILE",
"PROP",
"ARMY",
"TERRAIN_ZONE",
"MAP",
"RECONNAISSANCE",
"FOG_OF_WAR",
"RECTANGLE",
"ALIAS",
"ASSOCIATE",
"TERMINATED",
"PLAYER_TERMINATED",
"DISABLE_COUNT",
"INDIVIDUAL_REACTION",
"DIPLOMATIC_STATUS_BETWEEN",
"NAME",
"PARAM_BLOCK",
"NEXT_UNIQUE_ID",
"TILES",
"SELECTABLE",
"TARGETABLE",
"RENDERABLE",
"AI_CONTROLLER",
"DIPLOMATIC_OFFER",
};
char *OBJTYPE_str[14] = {
"USER",
"LEVEL",
"PLAYER",
"CITY",
"TOWN",
"BUILDING",
"CHARACTER",
"CONTAINER",
"FORMATION",
"MARKER",
"MISSILE",
"PROP",
"ARMY",
"TERRAIN_ZONE",
};
char *SETTING_str[21] = {
"GAME_DIR",
"FAR_Z_VALUE",
"HARDWARE_VERTEX_PROCESSING",
"OCCLUSION_RATE",
"VSYNC",
"TEXTURE_COMPRESSION",
"MAP_MAX_PART_SIZE",
"FOG",
"FONT",
"HARDWARE_CURSOR",
"FULLSCREEN",
"SCREEN_SIZE",
"MULTI_BCP",
"RENDERER",
"USE_MAP_TEXTURE_DATABASE",
"MESH_BATCHING",
"ANIMATED_MODELS",
"VERTICAL_FOV",
"PRELOAD_ALL_MODELS",
"ENABLE_GAMEPLAY_SHORTCUTS",
"SHOW_TIME_OBJ_INFO",
};
char *STASK_str[17] = {
"TRIGGER",
"TARGET",
"PROXIMITY",
"PROXIMITY_SATISFIED",
"LAST_DESTINATION_VALID",
"FIRST_EXECUTION",
"TRIGGERS_STARTED",
"PROCESS_STATE",
"TASK_ID",
"START_SEQUENCE_EXECUTED",
"SPAWN_BLUEPRINT",
"COST_DEDUCTED",
"DESTINATION",
"FACE_TOWARDS",
"START_TIME",
"INITIAL_POSITION",
"INITIAL_VELOCITY",
};
char *SORDER_str[6] = {
"TASK",
"PROCESS_STATE",
"CYCLED",
"ORDER_ID",
"UNIQUE_TASK_ID",
"CURRENT_TASK",
};
char *PDEVENT_str[7] = {
"ON_STAMPDOWN",
"ON_LEVEL_START",
"ON_CONTROL_TRANSFERRED",
"ON_CONVERSION_END",
"ON_TERMINATION",
"ON_DESTRUCTION",
"ON_SPAWN",
};
char* PDEVENT_tab0[7] = {
"On Stampdown",
"On Level Start",
"On Control Transferred",
"On Conversion End",
"On Termination",
"On Destruction",
"On Spawn",
};
char *ORDTSKTYPE_str[10] = {
"OBJECT_REFERENCE",
"SPAWN",
"UPGRADE",
"MOVE",
"FACE_TOWARDS",
"MISSILE",
"ATTACK",
"BUILD",
"REPAIR",
"FORMATION_CREATE",
};
char *CORDER_str[10] = {
"USE_TASK",
"FLAG",
"CLASS_TYPE",
"IN_ORDER_CATEGORY",
"INITIALISATION_SEQUENCE",
"START_SEQUENCE",
"SUSPENSION_SEQUENCE",
"RESUMPTION_SEQUENCE",
"CANCELLATION_SEQUENCE",
"TERMINATION_SEQUENCE",
};
char *CTASK_str[21] = {
"CLASS_TYPE",
"IN_TASK_CATEGORY",
"TASK_TARGET",
"USE_PREVIOUS_TASK_TARGET",
"PROXIMITY_REQUIREMENT",
"FLAG",
"INITIALISATION_SEQUENCE",
"START_SEQUENCE",
"SUSPENSION_SEQUENCE",
"RESUMPTION_SEQUENCE",
"CANCELLATION_SEQUENCE",
"TERMINATION_SEQUENCE",
"PROXIMITY_SATISFIED_SEQUENCE",
"PROXIMITY_DISSATISFIED_SEQUENCE",
"TRIGGER",
"SYNCH_ANIMATION_TO_FRACTION",
"REJECT_TARGET_IF_IT_IS_TERMINATED",
"TERMINATE_ENTIRE_ORDER_IF_NO_TARGET",
"IDENTIFY_TARGET_EACH_CYCLE",
"PLAY_ANIMATION",
"PLAY_ANIMATION_ONCE",
};
char *ORDERASSIGNMODE_str[3] = {
"FORGET_EVERYTHING_ELSE",
"DO_FIRST",
"DO_LAST",
};
char *CORDERASSIGN_str[3] = {
"ORDER_ASSIGNMENT_MODE",
"ORDER_TO_ASSIGN",
"ORDER_TARGET",
};
char *TASKTRIGGER_str[6] = {
"TIMER",
"ANIMATION_LOOP",
"UNINTERRUPTIBLE_ANIMATION_LOOP",
"ATTACHMENT_POINT",
"COLLISION",
"STRUCK_FLOOR",
};
char *DISTCALCMODE_str[3] = {
"3D",
"HORIZONTAL",
"VERTICAL",
};
char *OBJFINDCOND_str[9] = {
"SAME_PLAYER_UNITS",
"ALLIED_UNITS",
"ENEMY_UNITS",
"ORIGINAL_SAME_PLAYER_UNITS",
"ORIGINAL_ALLIED_UNITS",
"ORIGINAL_ENEMY_UNITS",
"BUILDINGS_ONLY",
"CHARACTERS_ONLY",
"CHARACTERS_AND_BUILDINGS_ONLY",
};
char *PACKAGE_ITEM_MOD_str[3] = {
"REPLACE",
"INCREASE",
"REDUCE",
};
char *GTW_BUTTON_WINDOW_ACTION_str[4] = {
"CLOSE_WINDOW",
"MOVE_PREVIOUS_PAGE",
"MOVE_NEXT_PAGE",
"MOVE_FIRST_PAGE",
};
char *SREQSTATE_str[4] = {
"COMPLETE",
"ACTIVE",
"STALLED",
"BLOCKED",
};
char *SREQDETAILEDSTATE_str[14] = {
"OK",
"WAIT_CONDITION_FAILED",
"RESOURCE_REQUIRED_RESERVED",
"IMPOSSIBLE_CONDITION_FAILED",
"NO_SPAWN_LOCATION_FOUND",
"NO_SPAWN_LOCATIONS_IDLE",
"NO_UPGRADE_LOCATION_FOUND",
"NO_UPGRADE_LOCATIONS_IDLE",
"CANT_AFFORD",
"UNPROCESSED",
"SPAWN_ORDER_ASSIGNMENT_FAILED",
"UPGRADE_ORDER_ASSIGNMENT_FAILED",
"FOUNDATION_CREATION_FAILED",
"NO_VALID_POSITION",
};
char *SREQUIREMENTCLASS_str[3] = {
"BUILDING_REQUIREMENT",
"CHARACTER_REQUIREMENT",
"UPGRADE_REQUIREMENT",
};
char *BUILDINGTYPE_str[12] = {
"GATEHOUSE",
"WALL",
"WALL_CROSSROADS",
"WALL_CORNER_IN",
"WALL_CORNER_OUT",
"RURAL_CENTRE",
"CIVIC_CENTRE",
"RURAL",
"CIVIC",
"TOWER",
"TOWER_CORNER_IN",
"TOWER_CORNER_OUT",
};
