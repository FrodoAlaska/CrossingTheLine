#include "entity.h"
#include "levels/level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Globals (@TEMP)

static float random_speeds[] = {1000.0f, 1200.0f, 1400.0f, 1600.0f};

/// Globals (@TEMP)
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Vehicle functions

void vehicle_create(Vehicle* v, 
                    Level* lvl, 
                    const VehicleType type, 
                    const nikola::Vec3& start_pos, 
                    const nikola::Vec3& dir,
                    const float acceleration) {
  // Each vehicle type has a different scale and collider offset
  nikola::Vec3 collider_scale, collider_offset; 
  switch(type) {
    case VEHICLE_CAR:
      collider_scale  = nikola::Vec3(6.0f, 4.9f, 9.7f);
      collider_offset = nikola::Vec3(0.0f, 2.5f, 0.0f);
      break;
    case VEHICLE_TRUCK:
      // @TODO
      collider_scale  = nikola::Vec3(1.0f);
      collider_offset = nikola::Vec3(0.0f);
      break;
  }

  // Entity init
  entity_create(&v->entity, 
                lvl, 
                start_pos, 
                collider_scale,
                ENTITY_VEHICLE, 
                nikola::PHYSICS_BODY_DYNAMIC);

  // Based on the direction, the vehicle should be
  // facing towards the correct direction. 
  if(dir.z == -1) {
    nikola::physics_body_set_rotation(v->entity.body, nikola::Vec3(0.0f, 1.0f, 0.0f), 180.0f * nikola::DEG2RAD);
  } 

  // Collider position init
  nikola::collider_set_local_position(v->entity.collider, collider_offset);

  // Vehicle variables init 
  v->type         = type; 
  v->acceleration = acceleration;
  v->direction    = dir;

  nikola::physics_body_set_linear_velocity(v->entity.body, nikola::Vec3(acceleration) * v->direction);
}

void vehicle_set_active(Vehicle& v, const bool active) {
  v.entity.is_active = active;
  nikola::physics_body_set_awake(v.entity.body, active);

  if(active) {
    nikola::physics_body_set_rotation(v.entity.body, nikola::Vec3(1.0f), 0.0f);
    nikola::physics_body_apply_force(v.entity.body, nikola::Vec3(0.0f));
    nikola::physics_body_set_angular_velocity(v.entity.body, nikola::Vec3(0.0f));
    
    nikola::physics_body_set_linear_velocity(v.entity.body, nikola::Vec3(v.acceleration) * v.direction);
  }
}

/// Vehicle functions
/// ----------------------------------------------------------------------
