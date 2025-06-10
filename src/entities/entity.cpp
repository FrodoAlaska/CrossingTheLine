#include "entity.h"
#include "level.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Generic entity functions

void entity_create(Entity* entity, 
                   Level* lvl,
                   const nikola::Vec3& pos, 
                   const nikola::Vec3& scale, 
                   const EntityType entt_type, 
                   const nikola::PhysicsBodyType body_type) {
  // Entity variables init
  entity->type      = entt_type;
  entity->level_ref = lvl;
  entity->is_active = true;
  entity->start_pos = pos;

  // Body init
  nikola::PhysicsBodyDesc body_desc = {
    .position  = pos, 
    .type      = body_type,
    .user_data = entity,
  };
  entity->body = nikola::physics_body_create(body_desc);

  // Collider init
  nikola::ColliderDesc coll_desc = {
    .position  = nikola::Vec3(0.0f), 
    .extents   = scale,
    .friction  = 0.0f,
    .is_sensor = (entity->type == ENTITY_END_POINT),
  };
  entity->collider = nikola::physics_body_add_collider(entity->body, coll_desc);
}

const bool entity_aabb_test(Entity& entity, Entity& other) {
  nikola::Vec3 sum_size = nikola::collider_get_extents(entity.collider) + nikola::collider_get_extents(other.collider);
  nikola::Vec3 diff     = nikola::physics_body_get_position(entity.body) - nikola::physics_body_get_position(other.body); 

  // The sum of the sizes is greater than the difference between
  // the two boxes on ALL axises
  return (nikola::abs(diff.x) < sum_size.x) && (nikola::abs(diff.y) < sum_size.y) && (nikola::abs(diff.z) < sum_size.z);
}

/// Generic entity functions
/// ----------------------------------------------------------------------
