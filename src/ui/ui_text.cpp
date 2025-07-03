#include "ui.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Private functions

static void apply_animation_fade(UIText& text, const float direction) {
  if(!text.animation_timer.has_runout) {
    return;
  }
  
  text.color.a += direction;
  text.color.a = nikola::clamp_float(text.color.a, 0.0f, 1.0f);
}

static void apply_animation_blink(UIText& text, const float duration) {
  // @TODO: Probably not the best idea to have a static variable here.
  static float dir = 0.1f;

  if(text.color.a >= 1 || text.color.a <= 0) {
    dir = -dir;
  }
  
  apply_animation_fade(text, dir);
}

/// Private functions
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// UIText functions

void ui_text_create(UIText* text, const nikola::Window* window_ref, const UITextDesc& desc) {
  NIKOLA_ASSERT(text, "Invalid UIText given to ui_text_create");
  NIKOLA_ASSERT(RESOURCE_IS_VALID(desc.font_id), "Invalid font given to ui_text_create");

  // Variables init

  text->position = nikola::Vec2(0.0f);
  text->offset   = desc.offset;

  text->window_ref = (nikola::Window*)window_ref;

  text->string    = desc.string;
  text->font      = nikola::resources_get_font(desc.font_id);
  text->font_size = desc.font_size;

  text->anchor = desc.anchor;
  text->color  = desc.color;
  ui_text_set_anchor(*text, text->anchor);

  nikola::timer_create(&text->animation_timer, 1.0f, false);

  text->is_active = true;
}

const nikola::Vec2 ui_text_measure_size(const nikola::String& str, const UIText& text) {
  nikola::Vec2 result(0.0f, text.font_size);

  float font_scale   = (text.font_size / 256.0f); // @TODO: This is an engine problem, but the `256` should be a constant. This is _REALLY_ bad.
  float prev_advance = 0.0f;

  for(auto& ch : str) {
    nikola::Glyph* glyph = &text.font->glyphs[ch];
     
    // Give some love to the Y-axis as well
    if(ch == '\n') {
      result.y += text.font_size;
      result.x  = 0.0f; 

      continue; 
    }

    // Take into account the spaces as well as normal characters
    if(ch == ' ' || ch == '\t') {
      result.x += prev_advance;
      continue;
    }
     
    result.x    += glyph->size.x + glyph->offset.x;
    prev_advance = glyph->advance_x;
  }

  return nikola::Vec2(result.x * font_scale, result.y);
}

const nikola::Vec2 ui_text_measure_size(const UIText& text) {
  return ui_text_measure_size(text.string, text);
}

void ui_text_set_anchor(UIText& text, const UIAnchor anchor) {
  text.anchor = anchor;

  int width, height; 
  nikola::window_get_size(text.window_ref, &width, &height);

  nikola::Vec2 text_size   = ui_text_measure_size(text);
  nikola::Vec2 text_center = text_size / 2.0f;
  
  nikola::Vec2 window_size   = nikola::Vec2(width, height);
  nikola::Vec2 window_center = window_size / 2.0f;
  
  nikola::Vec2 padding = nikola::Vec2(10.0f, text.font_size);

  switch(text.anchor) {
    case UI_ANCHOR_TOP_LEFT:  
      text.position = padding + text.offset;
      break;
    case UI_ANCHOR_TOP_CENTER:
      text.position.x = (window_center.x - text_center.x) + text.offset.x; 
      text.position.y = padding.y + text.offset.y; 
      break;
    case UI_ANCHOR_TOP_RIGHT:
      text.position.x = (window_size.x - text_size.x - padding.x) + text.offset.x; 
      text.position.y = padding.y + text.offset.y;  
      break;
    case UI_ANCHOR_CENTER_LEFT:  
      text.position.x = padding.x + text.offset.x;
      text.position.y = (window_center.y - text_center.y) + text.offset.y; 
      break;
    case UI_ANCHOR_CENTER:
      text.position = (window_center - text_center) + text.offset;
      break;
    case UI_ANCHOR_CENTER_RIGHT:
      text.position.x = (window_size.x - text_size.x - padding.x) + text.offset.x; 
      text.position.y = (window_center.y - text_center.y) + text.offset.y; 
      break;
    case UI_ANCHOR_BOTTOM_LEFT:  
      text.position.x = padding.x + text.offset.x;
      text.position.y = (window_size.y - text_size.y - padding.y) + text.offset.y; 
      break;
    case UI_ANCHOR_BOTTOM_CENTER:
      text.position.x = (window_center.x - text_center.x) + text.offset.x;
      text.position.y = (window_size.y - text_size.y - padding.y) + text.offset.y; 
      break;
    case UI_ANCHOR_BOTTOM_RIGHT:
      text.position = (window_size - text_size - padding) + text.offset; 
      break;
  }
}

void ui_text_set_string(UIText& text, const nikola::String& string) {
  text.string = string; 
  ui_text_set_anchor(text, text.anchor);
}

void ui_text_render(const UIText& text) {
  if(!text.is_active) {
    return;
  }

  batch_render_text(text.font, text.string, text.position, text.font_size, text.color);
}

void ui_text_render_animation(UIText& text, const UITextAnimation type, const float duration) {
  if(!text.is_active) {
    return;
  }

  text.animation_timer.is_active = true;
  text.animation_timer.limit     = duration;
  nikola::timer_update(text.animation_timer);

  switch(type) {
    case UI_TEXT_ANIMATION_FADE_IN:
      apply_animation_fade(text, 0.1f);
      break;
    case UI_TEXT_ANIMATION_FADE_OUT:
      apply_animation_fade(text, -0.1f);
      break;
    case UI_TEXT_ANIMATION_BLINK:
      apply_animation_blink(text, duration);
      break;
  }

  ui_text_render(text);
}

/// UIText functions
/// ----------------------------------------------------------------------
