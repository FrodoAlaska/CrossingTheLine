#pragma once 

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// UIAnchor
enum UIAnchor {
  UI_ANCHOR_TOP_LEFT, 
  UI_ANCHOR_TOP_CENTER, 
  UI_ANCHOR_TOP_RIGHT, 
  
  UI_ANCHOR_CENTER_LEFT, 
  UI_ANCHOR_CENTER, 
  UI_ANCHOR_CENTER_RIGHT, 
  
  UI_ANCHOR_BOTTOM_LEFT, 
  UI_ANCHOR_BOTTOM_CENTER, 
  UI_ANCHOR_BOTTOM_RIGHT, 
};
/// UIAnchor
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// UITextAnimationType 
enum UITextAnimation {
  UI_TEXT_ANIMATION_FADE_IN,
  UI_TEXT_ANIMATION_FADE_OUT,
  UI_TEXT_ANIMATION_BLINK,
};
/// UITextAnimationType 
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// UIText
struct UIText {
  nikola::Vec2 position, offset;

  nikola::Window* window_ref;

  nikola::String string;
  nikola::Font* font;
  float font_size;
  
  UIAnchor anchor;
  nikola::Vec4 color;

  nikola::Timer animation_timer;
};
/// UIText
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
struct UITextDesc {
  nikola::String string;

  nikola::ResourceID font_id = {};
  float font_size            = 32.0f;

  UIAnchor anchor     = UI_ANCHOR_TOP_LEFT; 
  nikola::Vec2 offset = nikola::Vec2(0.0f);

  nikola::Vec4 color = nikola::Vec4(1.0f);
};
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// UIText functions

void ui_text_create(UIText* text, const nikola::Window* window_ref, const UITextDesc& desc);

const nikola::Vec2 ui_text_measure_size(const UIText& text);

void ui_text_set_anchor(UIText& text, const UIAnchor anchor);

void ui_text_set_string(UIText& text, const nikola::String& string);

void ui_text_render(const UIText& text);

void ui_text_render_animation(UIText& text, const UITextAnimation anim_type, const float duration);

/// UIText functions
/// ----------------------------------------------------------------------
