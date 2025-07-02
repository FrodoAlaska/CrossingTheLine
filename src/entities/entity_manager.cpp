#include "entity.h"
#include "levels/level.h"
#include "states/state.h"
#include "game_event.h"
#include "resource_database.h"
#include "sound_manager.h"

#include <nikola/nikola.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

/// ----------------------------------------------------------------------
/// EntityManager
struct EntityManager {
  Level* level_ref;

  Player player;
  Entity coin; 

  nikola::DynamicArray<Entity> points;
  nikola::DynamicArray<Vehicle> vehicles;
};

static EntityManager s_entt;
/// EntityManager
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Private functions

static void resolve_player_begin_collisions(Entity* player, Entity* other) {
  Level* lvl = player->level_ref;

  switch(other->type) {
    case ENTITY_VEHICLE:
      player_set_active(s_entt.player, false);

      game_event_dispatch(GameEvent{
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_LOST
      });
      break;
    case ENTITY_END_POINT:
      player_set_active(s_entt.player, false);
      game_event_dispatch(GameEvent{
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_WON
      });
      break;
    case ENTITY_VEHICLE_POINT: 
    case ENTITY_DEATH_POINT: 
      player_set_active(s_entt.player, false);
      game_event_dispatch(GameEvent{
        .type       = GAME_EVENT_STATE_CHANGED, 
        .state_type = STATE_LOST
      });
      break;
    case ENTITY_CHAPTER_POINT: 
      game_event_dispatch(GameEvent{.type = GAME_EVENT_CHAPTER_ENTERED}, other);
      break;
    case ENTITY_COIN:
      other->is_active = false; 
      nikola::physics_body_set_awake(other->body, false);

      game_event_dispatch(GameEvent{.type = GAME_EVENT_COIN_COLLECTED});
      game_event_dispatch(GameEvent{
        .type       = GAME_EVENT_SOUND_PLAYED, 
        .sound_type = SOUND_KEY_COLLECT,
      });
      break;
    default: 
      break;
  }
}

static void resolve_player_end_collisions(Entity* player, Entity* other) {
  Level* lvl = player->level_ref;

  switch(other->type) {
    case ENTITY_CHAPTER_POINT: 
      game_event_dispatch(GameEvent{.type = GAME_EVENT_CHAPTER_EXITED}, other);
      break;
    default: 
      break;
  }
}

static void resolve_vehicle_collisions(Entity* vehicle, Entity* other) {
  // Point
  if(other->type == ENTITY_VEHICLE_POINT) {
    nikola::physics_body_set_position(vehicle->body, vehicle->start_pos);
  }
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Callbacks

static void on_entity_begin_collision(const nikola::CollisionPoint& point) {
  // Getting the entities
  Entity* entt_a = (Entity*)nikola::physics_body_get_user_data(point.body_a);
  Entity* entt_b = (Entity*)nikola::physics_body_get_user_data(point.body_b);

  // @NOTE: Yeah. Terrible. I know.

  // This might cause problems, but we do not 
  // care for entities that are inactive. 
  if(!entt_a->is_active || !entt_b->is_active) {
    return;
  }

  // Player collisions
  
  if(entt_a->type == ENTITY_PLAYER) {
    resolve_player_begin_collisions(entt_a, entt_b);
  }
  else if(entt_b->type == ENTITY_PLAYER) {
    resolve_player_begin_collisions(entt_b, entt_a);
  }

  // Vehicle collisions
  
  if(entt_a->type == ENTITY_VEHICLE) {
    resolve_vehicle_collisions(entt_a, entt_b);
  }
  else if(entt_b->type == ENTITY_VEHICLE) {
    resolve_vehicle_collisions(entt_b, entt_a);
  }
}

static void on_entity_end_collision(const nikola::CollisionPoint& point) {
  // Getting the entities
  Entity* entt_a = (Entity*)nikola::physics_body_get_user_data(point.body_a);
  Entity* entt_b = (Entity*)nikola::physics_body_get_user_data(point.body_b);

  // @NOTE: Yeah. Terrible. I know.

  // This might cause problems, but we do not 
  // care for entities that are inactive. 
  if(!entt_a->is_active || !entt_b->is_active) {
    return;
  }

  // Player collisions
  
  if(entt_a->type == ENTITY_PLAYER) {
    resolve_player_end_collisions(entt_a, entt_b);
  }
  else if(entt_b->type == ENTITY_PLAYER) {
    resolve_player_end_collisions(entt_b, entt_a);
  }
}

/// Callbacks
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Entity manager functions

void entity_manager_create(Level* level_ref) {
  // Level init
  s_entt.level_ref = level_ref;
 
  // Physics world callback init
  nikola::physics_world_set_collision_callback(on_entity_begin_collision, on_entity_end_collision); 
}

void entity_manager_destroy() {
  // Player destroy
  nikola::physics_body_destroy(s_entt.player.entity.body);

  // Coin destroy 
  if(s_entt.coin.is_active) {
    nikola::physics_body_destroy(s_entt.coin.body);
  } 

  // End points destroy
  for(auto& point : s_entt.points) {
    nikola::physics_body_destroy(point.body);
  }
  s_entt.points.clear();

  // Vehicles destroy
  for(auto& v : s_entt.vehicles) {
    nikola::physics_body_destroy(v.entity.body);
  }
  s_entt.vehicles.clear();
}

void entity_manager_load() {
  // For better visualization 
  NKLevelFile* nklvl = &s_entt.level_ref->nkbin;

  // Player init
  player_create(&s_entt.player, s_entt.level_ref, nklvl->start_position);

  // Coin init
 
  s_entt.coin.is_active = false;
  if(nklvl->has_coin) {
    entity_create(&s_entt.coin, 
                  s_entt.level_ref, 
                  nklvl->coin_position,
                  nikola::Vec3(1.4f, 0.5f, 4.0f),
                  ENTITY_COIN, 
                  nikola::PHYSICS_BODY_DYNAMIC, 
                  true);

    nikola::collider_set_local_position(s_entt.coin.collider, nikola::Vec3(0.0f, 0.0f, 1.6f));
    
    nikola::physics_body_set_rotation(s_entt.coin.body, nikola::Vec3(1.0f, 0.0f, 0.0f), 4.7f);
    nikola::physics_body_set_angular_velocity(s_entt.coin.body, nikola::Vec3(0.0f, 4.5f, 0.0f));
  }

  // Points init

  s_entt.points.resize(nklvl->points_count);
  for(nikola::sizei i = 0; i < s_entt.points.size(); i++) {
    Entity* point = &s_entt.points[i];

    entity_create(point, 
                  s_entt.level_ref, 
                  nklvl->points[i].position, 
                  nklvl->points[i].scale, 
                  (EntityType)nklvl->points[i].type,
                  nikola::PHYSICS_BODY_STATIC, 
                  true);
  }

  // Vehicles init
  
  s_entt.vehicles.resize(nklvl->vehicles_count);
  for(nikola::sizei i = 0; i < s_entt.vehicles.size(); i++) {
    Vehicle* vehicle = &s_entt.vehicles[i];

    vehicle_create(vehicle,  
                   s_entt.level_ref, 
                   (VehicleType)nklvl->vehicles[i].vehicle_type, 
                   nklvl->vehicles[i].position, 
                   nklvl->vehicles[i].direction, 
                   nklvl->vehicles[i].acceleration);
  }
}

void entity_manager_save() {
  // For better visualization 
  NKLevelFile* nklvl = &s_entt.level_ref->nkbin;

  // Save the player
  nklvl->start_position = nikola::physics_body_get_position(s_entt.player.entity.body); 

  // Save the coin 

  nklvl->has_coin = s_entt.coin.is_active;
  if(nklvl->has_coin) {
    nklvl->coin_position = nikola::physics_body_get_position(s_entt.coin.body); 
  }

  // Save the end points

  nklvl->points_count = s_entt.points.size();
  for(nikola::sizei i = 0; i < s_entt.points.size(); i++) {
    Entity* point = &s_entt.points[i];

    nklvl->points[i].position = nikola::physics_body_get_position(point->body);
    nklvl->points[i].scale    = nikola::collider_get_extents(point->collider);
    nklvl->points[i].type     = (nikola::u16)point->type;
  }

  // Save the vehicles
  
  nklvl->vehicles_count = s_entt.vehicles.size();
  for(nikola::sizei i = 0; i < s_entt.vehicles.size(); i++) {
    Vehicle* vehicle = &s_entt.vehicles[i];

    nklvl->vehicles[i].position     = nikola::physics_body_get_position(vehicle->entity.body); 
    nklvl->vehicles[i].direction    = vehicle->direction; 
    nklvl->vehicles[i].acceleration = vehicle->acceleration;
    nklvl->vehicles[i].vehicle_type = (nikola::u8)vehicle->type; 
  }
}

void entity_manager_reset() {
  // Reset the player
  nikola::physics_body_set_position(s_entt.player.entity.body, s_entt.level_ref->nkbin.start_position);
  s_entt.player.entity.is_active = true;

  // Reset the vehicles
  for(auto& v : s_entt.vehicles) {
    nikola::physics_body_set_position(v.entity.body, v.entity.start_pos);
    vehicle_set_active(v, true);
  }

  // Reset the coin
  if(s_entt.coin.is_active) {
    nikola::physics_body_set_angular_velocity(s_entt.coin.body, nikola::Vec3(0.0f, 4.5f, 0.0f));
  }
}

void entity_manager_update() {
  // Player update
  player_update(s_entt.player);

  // Coin update
  if(s_entt.coin.is_active) {
    nikola::physics_body_set_angular_velocity(s_entt.coin.body, nikola::Vec3(0.0f, 1.5f, 0.0f));
  } 

  // Tiles test
  // tile_manager_check_collisions(s_entt.player);
}

void entity_manager_render() {
  nikola::Transform transform = {}; 

  // Render vehicles
  
  for(auto& v : s_entt.vehicles) {
    transform = nikola::physics_body_get_transform(v.entity.body);

    switch(v.type) {
      case VEHICLE_CAR:
        nikola::transform_scale(transform, nikola::Vec3(4.0f));
        nikola::renderer_queue_model(resource_database_get(RESOURCE_CAR), transform);
        break;
      case VEHICLE_TRUCK:
        nikola::transform_scale(transform, nikola::Vec3(6.0f));
        nikola::renderer_queue_model(resource_database_get(RESOURCE_TRUCK), transform);
        break;
    }

    if(s_entt.level_ref->debug_mode) {
      nikola::renderer_debug_collider(v.entity.collider, nikola::Vec3(1.0f, 0.0f, 0.0f));
    }
  }

  // Render the coin
  
  if(s_entt.coin.is_active) {
    transform = nikola::physics_body_get_transform(s_entt.coin.body);
    
    nikola::transform_scale(transform, nikola::Vec3(0.025f));
    nikola::renderer_queue_model(resource_database_get(RESOURCE_COIN), transform);
  }

  // Render the player 
 
  transform = nikola::physics_body_get_transform(s_entt.player.entity.body);
  nikola::transform_scale(transform, nikola::collider_get_extents(s_entt.player.entity.collider));
  nikola::renderer_queue_mesh(resource_database_get(RESOURCE_CUBE), transform);

  // Debug rendering
  
  if(s_entt.level_ref->debug_mode) {
    // Player
    nikola::renderer_debug_collider(s_entt.player.entity.collider);

    // Points
    for(auto& point : s_entt.points) {
      nikola::renderer_debug_collider(point.collider);
    }

    // Coin 
    if(s_entt.coin.is_active) {
      nikola::renderer_debug_collider(s_entt.coin.collider); 
    } 
  }
}

void entity_manager_render_gui() {
  // Player
  if(ImGui::CollapsingHeader("Player")) {
    nikola::gui_edit_physics_body("Player body", s_entt.player.entity.body);
    nikola::gui_edit_collider("Player collider", s_entt.player.entity.collider);
  }

  // Coin
  if(ImGui::CollapsingHeader("Coin")) {
    nikola::gui_edit_physics_body("Coin body", s_entt.coin.body);
    nikola::gui_edit_collider("Coin collider", s_entt.coin.collider);

    // Collider offset
    nikola::Vec3 offset = nikola::collider_get_local_transform(s_entt.coin.collider).position;
    if(ImGui::DragFloat3("Collider offset", &offset[0], 0.1f)) {
      nikola::collider_set_local_position(s_entt.coin.collider, offset);
    }

    // Rotation
    nikola::Vec4 rotation = nikola::physics_body_get_rotation(s_entt.coin.body);
    if(ImGui::DragFloat4("Rotation Axis", &rotation[0], 0.1f)) {
      nikola::physics_body_set_rotation(s_entt.coin.body, nikola::Vec3(rotation), rotation.w);
    }

    // Active state
    ImGui::Checkbox("Active", &s_entt.coin.is_active);
  }
  
  // Points
  if(ImGui::CollapsingHeader("Points")) {
    ImGui::Text("End points count: %zu", s_entt.points.size());

    for(nikola::sizei i = 0; i < s_entt.points.size(); i++) {
      nikola::String name = ("Point " + std::to_string(i)); 
      Entity* entity      = &s_entt.points[i];
      
      ImGui::SeparatorText(name.c_str());
      ImGui::PushID(name.c_str());

      // Position 
      nikola::Vec3 position = nikola::physics_body_get_position(entity->body);
      if(ImGui::DragFloat3("Position", &position[0], 0.1f)) {
        nikola::physics_body_set_position(entity->body, position);
        entity->start_pos = position;
      }

      // Size
      nikola::Vec3 size = nikola::collider_get_extents(entity->collider);
      if(ImGui::DragFloat3("Extents", &size[0], 0.1f)) {
        nikola::collider_set_extents(entity->collider, size);
      } 

      // Type
      if(ImGui::BeginCombo("Type", "End point")) {
        if(ImGui::Selectable("End point")) {
          entity->type = ENTITY_END_POINT;
        } 
        else if(ImGui::Selectable("Vehicle point")) {
          entity->type = ENTITY_VEHICLE_POINT;
        }
        else if(ImGui::Selectable("Death point")) {
          entity->type = ENTITY_DEATH_POINT;
        }
        else if(ImGui::Selectable("Chapter point")) {
          entity->type = ENTITY_CHAPTER_POINT;
        }

        ImGui::EndCombo();
      }
      
      // Remove the point
      if(ImGui::Button("Remove")) {
        s_entt.points.erase(s_entt.points.begin() + i);
      }
      
      ImGui::PopID();
    }
    
    ImGui::SeparatorText("Add options");

    // Position
    static nikola::Vec3 position = nikola::Vec3(10.0f, 0.0f, 10.0f);
    ImGui::DragFloat3("Position", &position[0], 0.1f);
    
    // Scale
    static nikola::Vec3 scale = nikola::Vec3(1.0f);
    ImGui::DragFloat3("Scale", &scale[0], 0.1f);

    // Type 
    static int current_point = 0; 
    EntityType point_types[] = {ENTITY_END_POINT, ENTITY_VEHICLE_POINT, ENTITY_DEATH_POINT, ENTITY_CHAPTER_POINT};
    ImGui::Combo("Type", &current_point, "End point\0Vehicle point\0Death point\0Chapter point\0\0");

    // Add an point
    if(ImGui::Button("Add point")) {
      s_entt.points.resize(s_entt.points.size() + 1);
      entity_create(&s_entt.points[s_entt.points.size() - 1],
                    s_entt.level_ref,
                    position,
                    scale,
                    point_types[current_point]);
    }
  }
  
  // Vehicles
  if(ImGui::CollapsingHeader("Vehicles")) {
    ImGui::Text("Vehicles count: %zu", s_entt.vehicles.size());
      
    // Clear all
    if(ImGui::Button("Clear all")) {
      s_entt.vehicles.clear();
    }

    for(nikola::sizei i = 0; i < s_entt.vehicles.size(); i++) {
      nikola::String name  = ("Vehicle " + std::to_string(i)); 
      Entity* vehicle_entt = &s_entt.vehicles[i].entity;
      
      ImGui::SeparatorText(name.c_str());
      ImGui::PushID(name.c_str());

      // Position 
      nikola::Vec3 position = nikola::physics_body_get_position(vehicle_entt->body);
      if(ImGui::DragFloat3("Position", &position[0], 2.0f)) {
        nikola::physics_body_set_position(vehicle_entt->body, position);
        vehicle_entt->start_pos = position;
      }

      // Size
      nikola::Vec3 size = nikola::collider_get_extents(vehicle_entt->collider);
      if(ImGui::DragFloat3("Extents", &size[0], 0.1f)) {
        nikola::collider_set_extents(vehicle_entt->collider, size);
      }
      
      // Collider offset
      nikola::Vec3 offset = nikola::collider_get_local_transform(vehicle_entt->collider).position;
      if(ImGui::DragFloat3("Collider offset", &offset[0], 0.1f)) {
        nikola::collider_set_local_position(vehicle_entt->collider, offset);
      }
      
      // Acceleration
      if(ImGui::DragFloat("Acceleration", &s_entt.vehicles[i].acceleration, 0.1f)) {
        // @TEMP: Little hack because I'm lazy
        vehicle_set_active(s_entt.vehicles[i], false); 
        vehicle_set_active(s_entt.vehicles[i], true); 
      }

      // Direction 
      ImGui::DragFloat3("Direction", &s_entt.vehicles[i].direction[0], 1.0f, -1.0f, 1.0f);

      // Active state
      if(ImGui::Checkbox("Active", &vehicle_entt->is_active)) {
        vehicle_set_active(s_entt.vehicles[i], vehicle_entt->is_active);
      }

      // Remove the vehicle
      if(ImGui::Button("Remove")) {
        s_entt.vehicles.erase(s_entt.vehicles.begin() + i);
      }

      ImGui::PopID();
    }
      
    ImGui::SeparatorText("Add options");

    // Type
    static int type = VEHICLE_CAR;
    ImGui::Combo("Type", &type, "Car\0Truck\0\0");

    // Position
    static nikola::Vec3 position = nikola::Vec3(-16.0f, -1.5f, -32.0f);
    ImGui::DragFloat3("Position", &position[0], 2.0f);
    
    // Direction
    static nikola::Vec3 dir = nikola::Vec3(0.0f, 0.0f, 1.0f);
    ImGui::DragFloat3("Direction", &dir[0], 1.0f, -1.0f, 1.0f);
    
    // Acceleration
    static float accel = 50.0f;
    ImGui::DragFloat("Acceleration", &accel, 0.1f);

    // Add a vehicle
    if(ImGui::Button("Add vehicle")) {
      s_entt.vehicles.resize(s_entt.vehicles.size() + 1);

      vehicle_create(&s_entt.vehicles[s_entt.vehicles.size() - 1], 
                     s_entt.level_ref, 
                     (VehicleType)type, 
                     position, 
                     dir, 
                     accel);
    }
  }
}

/// Entity manager functions
/// ----------------------------------------------------------------------
