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
/// Callbacks

// Forward declarations
struct UILayout;
struct UIText;

using OnUILayoutClickFunc = void(*)(UILayout& layout, UIText& text, void* user_data);

/// Callbacks
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

  bool is_active = false;
};
/// UIText
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// UITextDesc
struct UITextDesc {
  nikola::String string;

  nikola::ResourceID font_id = {};
  float font_size            = 32.0f;

  UIAnchor anchor     = UI_ANCHOR_TOP_LEFT; 
  nikola::Vec2 offset = nikola::Vec2(0.0f);

  nikola::Vec4 color = nikola::Vec4(1.0f);
};
/// UITextDesc
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// UILayout
struct UILayout {
  nikola::ResourceID font_id;
  nikola::Window* window_ref;

  nikola::DynamicArray<UIText> texts;

  UIAnchor current_anchor     = UI_ANCHOR_TOP_LEFT;
  nikola::Vec2 extra_offset   = nikola::Vec2(0.0f);
  nikola::Vec2 current_offset = nikola::Vec2(0.0f);
  nikola::u32 current_option  = 0;

  OnUILayoutClickFunc click_func = nullptr;
  void* user_data                = nullptr;

  bool is_active = false;
};
/// UILayout
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

/// ----------------------------------------------------------------------
/// UILayout functions

void ui_layout_create(UILayout* layout, 
                      nikola::Window* window_ref, 
                      const nikola::ResourceID& font_id, 
                      const OnUILayoutClickFunc& click_func = nullptr, 
                      const void* user_data = nullptr);

void ui_layout_begin(UILayout& layout, const UIAnchor anchor, const nikola::Vec2& offset);

void ui_layout_end(UILayout& layout);

void ui_layout_push_text(UILayout& layout, const nikola::String& str, const float size, const nikola::Vec4& color);

void ui_layout_update(UILayout& layout);

void ui_layout_render(UILayout& layout);

void ui_layout_render_animation(UILayout& layout, const UITextAnimation anim_type, const float duration);

/// UILayout functions
/// ----------------------------------------------------------------------
