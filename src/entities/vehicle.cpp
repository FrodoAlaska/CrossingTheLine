#include "entity.h"
#include "levels/level.h"
#include "sound_manager.h"
#include "game_event.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Vehicle functions

void vehicle_create(Vehicle* v, 
                    Level* lvl, 
                    const VehicleType type, 
                    const nikola::Vec3& start_pos, 
                    const nikola::Vec3& dir,
                    const float acceleration) {
  nikola::Vec3 collider_scale, collider_offset; 

  // Each vehicle type has a different scale and collider offset
  switch(type) {
    case VEHICLE_CAR:
      collider_scale  = nikola::Vec3(5.0f, 4.9f, 9.0f);
      collider_offset = nikola::Vec3(0.0f, 2.5f, 0.0f);
      break;
    case VEHICLE_TRUCK:
      collider_scale  = nikola::Vec3(7.5f, 8.0f, 17.0f);
      collider_offset = nikola::Vec3(0.0f, 4.2f, 0.0f);
      break;
  }

  // Entity init
  entity_create(&v->entity, 
                lvl, 
                start_pos, 
                collider_scale,
                ENTITY_VEHICLE, 
                nikola::PHYSICS_BODY_DYNAMIC);

  // Vehicle variables init 
  v->type         = type; 
  v->acceleration = acceleration;
  v->direction    = dir;
  v->rotation     = nikola::Vec4(0.0f);

  // Based on the direction, the vehicle should be
  // facing towards the correct direction. 
  if(dir.z <= -1) {
    v->rotation = nikola::Vec4(0.0f, 1.0f, 0.0f, 180.0f * nikola::DEG2RAD);
    nikola::physics_body_set_rotation(v->entity.body, nikola::Vec3(v->rotation), v->rotation.w);
  } 

  // Collider position init
  nikola::collider_set_local_position(v->entity.collider, collider_offset);

  // Set the collision layer
  nikola::physics_body_set_layers(v->entity.body, PHYSICS_LAYER_0);
}

void vehicle_set_active(Vehicle& v, const bool active) {
  v.entity.is_active = active;
  nikola::physics_body_set_awake(v.entity.body, active);

  if(active) {
    nikola::physics_body_set_rotation(v.entity.body, nikola::Vec3(v.rotation), v.rotation.w);
    nikola::physics_body_set_angular_velocity(v.entity.body, nikola::Vec3(0.0f));
    nikola::physics_body_set_linear_velocity(v.entity.body, nikola::Vec3(v.acceleration) * v.direction);
  }
}

/// Vehicle functions
/// ----------------------------------------------------------------------
